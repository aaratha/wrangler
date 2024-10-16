#include "render_utils.h"

namespace RenderUtils {

    void InitializeWindow(int& screenWidth, int& screenHeight) {
        SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
        InitWindow(screenWidth, screenHeight, "raylib [core] example - 3d camera mode");
    }

    Camera3D SetupCamera() {
        Camera3D camera = { 0 };
        camera.position = CAMERA_OFFSET;
        camera.target = vec3{0.0, 10.0, 10.0};
        camera.up = vec3{ 0.0f, 1.0f, 0.0f };
        camera.fovy = 60.0f;
        camera.projection = CAMERA_PERSPECTIVE;
        return camera;
    }

    RenderTexture2D LoadShadowmapRenderTexture(int width, int height)
    {
        RenderTexture2D target = { 0 };

        target.id = rlLoadFramebuffer(0,0); // Load an empty framebuffer
        target.texture.width = width;
        target.texture.height = height;

        if (target.id > 0)
        {
            rlEnableFramebuffer(target.id);

            // Create depth texture
            // We don't need a color texture for the shadowmap
            target.depth.id = rlLoadTextureDepth(width, height, false);
            target.depth.width = width;
            target.depth.height = height;
            target.depth.format = 19;       //DEPTH_COMPONENT_24BIT?
            target.depth.mipmaps = 1;

            // Attach depth texture to FBO
            rlFramebufferAttach(target.id, target.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);

            // Check if fbo is complete with attachments (valid)
            if (rlFramebufferComplete(target.id)) TRACELOG(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully", target.id);

            rlDisableFramebuffer();
        }
        else TRACELOG(LOG_WARNING, "FBO: Framebuffer object can not be created");

        return target;
    }

    // Unload shadowmap render texture from GPU memory (VRAM)
    void UnloadShadowmapRenderTexture(RenderTexture2D target)
    {
        if (target.id > 0)
        {
            // NOTE: Depth texture/renderbuffer is automatically
            // queried and deleted before deleting framebuffer
            rlUnloadFramebuffer(target.id);
        }
    }

    void draw_scene(Model cube, Player player, std::vector<Animal> animals) {
        DrawModelEx(cube,
            (Vector3) { 0.0f, -0.5f, 0.0f },
            Vector3Zero(),
            0.0f,
            (Vector3) { 40.0f, 1.0f, 40.0f },
            (Color) {50, 168, 82, 255}
        );
        player.draw();
        player.tether.draw();
        player.rope.draw();

        for (auto& animal : animals) {
            animal.draw();
        }
    }

    void update_camera(Camera3D& camera, Player player) {
        camera.target.x = lerp_to(camera.target.x, player.com.x, 0.2f);
        camera.target.z = lerp_to(camera.target.z, player.com.z, 0.2f);
        camera.target.y = 0.0f;
        //camera.position = lerp3D(camera.position, player.com + vec3{0.0, 15.0, 8.0}, 0.9);
        camera.position = player.pos + CAMERA_OFFSET;
        camera.position.x = player.com.x + CAMERA_OFFSET.x;

        #if defined(_WIN32) || defined(_WIN64)
            camera.fovy -= 3*GetMouseWheelMove();
        #else
            camera.fovy -= GetMouseWheelMove();
        #endif
        camera.fovy = Clamp(camera.fovy, 20.0f, 100.0f);
        camera.fovy = Clamp(camera.fovy, 20.0f, 100.0f);
    }

    void UnloadResources(
        Shader shadowShader,
        Player player,
        std::vector<Animal> animals,
        RenderTexture2D shadowMap,
        Model cube,
        Shader dofShader,
        RenderTexture2D dofTexture
    ) {
        UnloadShader(shadowShader);
        UnloadModel(player.model);
        UnloadModel(player.tether.model);
        for (auto& animal : animals) {
            UnloadModel(animal.model);
        }
        UnloadShadowmapRenderTexture(shadowMap);
        UnloadModel(cube);
        UnloadShader(dofShader);
        UnloadRenderTexture(dofTexture);
    }

    RenderTexture2D SetupDofTexture(int screenWidth, int screenHeight) {
        RenderTexture2D dofTexture = LoadRenderTexture(screenWidth, screenHeight);
        if (dofTexture.id == 0) {
            throw std::runtime_error("Failed to create dofTexture");
        }
        return dofTexture;
    }


    rl::Shader SetupDofShader(int screenWidth, int screenHeight) {
        rl::Shader dofShader(0, TextFormat("resources/shaders/dof.fs", GLSL_VERSION));
        if (dofShader.id == 0) {
            throw std::runtime_error("Failed to compile DoF shader");
        }

        float blurRadius = 3.0f;
        float resolution[2] = {(float)screenWidth, (float)screenHeight};
        SetShaderValue(dofShader, GetShaderLocation(dofShader, "resolution"), resolution, SHADER_UNIFORM_VEC2);
        SetShaderValue(dofShader, GetShaderLocation(dofShader, "radius"), &blurRadius, SHADER_UNIFORM_FLOAT);

        return dofShader;
    }

    rl::Shader SetupShadowShader(vec3& lightDir) {
        rl::Shader shadowShader(TextFormat("resources/shaders/lighting.vs", GLSL_VERSION),
                                TextFormat("resources/shaders/lighting.fs", GLSL_VERSION));
        shadowShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shadowShader, "viewPos");
        Color lightColor = WHITE;
        Vector4 lightColorNormalized = ColorNormalize(lightColor);
        int lightDirLoc = GetShaderLocation(shadowShader, "lightDir");
        int lightColLoc = GetShaderLocation(shadowShader, "lightColor");
        SetShaderValue(shadowShader, lightDirLoc, &lightDir, SHADER_UNIFORM_VEC3);
        SetShaderValue(shadowShader, lightColLoc, &lightColorNormalized, SHADER_UNIFORM_VEC4);
        int ambientLoc = GetShaderLocation(shadowShader, "ambient");
        float ambient[4] = {0.1f, 0.1f, 0.1f, 1.0f};
        SetShaderValue(shadowShader, ambientLoc, ambient, SHADER_UNIFORM_VEC4);
        int lightVPLoc = GetShaderLocation(shadowShader, "lightVP");
        int shadowMapLoc = GetShaderLocation(shadowShader, "shadowMap");
        int shadowMapResolution = SHADOWMAP_RESOLUTION;
        SetShaderValue(shadowShader, GetShaderLocation(shadowShader, "shadowMapResolution"), &shadowMapResolution, SHADER_UNIFORM_INT);

        return shadowShader;
    }


    void RenderShadowMap(Shader shadowShader, RenderTexture2D& shadowMap, Camera3D& lightCam, Model& cube, Player& player,
                        std::vector<Animal>& animals) {
            BeginTextureMode(shadowMap);
            ClearBackground(WHITE);
            BeginMode3D(lightCam);
                Matrix lightViewProj = MatrixMultiply(rlGetMatrixModelview(), rlGetMatrixProjection());
            SetShaderValueMatrix(shadowShader, GetShaderLocation(shadowShader, "lightVP"), lightViewProj);
                RenderUtils::draw_scene(cube, player, animals);
            EndMode3D();
            EndTextureMode();
    }

    void RenderSceneToTexture(RenderTexture2D& dofTexture, Camera3D& camera, rl::Shader& shadowShader,
                            RenderTexture2D& shadowMap, Model& cube, Player& player, std::vector<Animal>& animals) {
        BeginTextureMode(dofTexture);
        ClearBackground(RAYWHITE);

        rlEnableShader(shadowShader.id);
        int slot = 10;
        rlActiveTextureSlot(10);
        rlEnableTexture(shadowMap.depth.id);
        rlSetUniform(GetShaderLocation(shadowShader, "shadowMap"), &slot, SHADER_UNIFORM_INT, 1);

        rlDisableShader();
        BeginMode3D(camera);
        RenderUtils::draw_scene(cube, player, animals);
        EndMode3D();

        EndTextureMode();
    }

    void HandleWindowResize(int& screenWidth, int& screenHeight, RenderTexture2D& dofTexture, Shader& dofShader) {
        if (IsWindowResized()) {
            screenWidth = GetScreenWidth();
            screenHeight = GetScreenHeight();

            // Re-create the DOF texture with the new screen size
            UnloadRenderTexture(dofTexture);
            dofTexture = RenderUtils::SetupDofTexture(screenWidth, screenHeight);

            // Re-apply the resolution to the shader
            float resolution[2] = { (float)screenWidth, (float)screenHeight };
            SetShaderValue(dofShader, GetShaderLocation(dofShader, "resolution"), resolution, SHADER_UNIFORM_VEC2);

            // Update the viewport to match the new window size
            rlViewport(0, 0, screenWidth, screenHeight);
        }
    }

    void DrawGUI(GameState& GameState, int& screenWidth, int& screenHeight) {
        int width = 40;
        int height = 40;
        int margin = 20;
        GuiToggleGroup(
            (Rectangle) {
                screenWidth-width-margin,
                screenHeight-(4.05*height)-margin,
                width,
                height
            },
            "#1#\n#3#\n#8#\n#23#",
            &GameState.itemActive
        );
    }
}
