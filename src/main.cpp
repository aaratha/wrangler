#include "raylib-cpp.hpp"
#include "raymath.h"
#include "rlgl.h"
#include "utils.h"
#include "player.h"
#include "animal.h"
#include "physics.h"
#include "render_utils.h"
#include <random>
#include <limits>
#include <vector>
#include <array>


#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

#define SHADOWMAP_RESOLUTION 2048




int main(void) {
  int width = GetScreenWidth();
  int height = GetScreenHeight();


    const float rate = 0.4;
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1280;
    const int screenHeight = 720;

    SetConfigFlags(FLAG_MSAA_4X_HINT);  // Enable Multi Sampling Anti Aliasing 4x (if available)
    InitWindow(screenWidth, screenHeight, "raylib [core] example - 3d camera mode");

    // Define the camera to look into our 3d world
    Camera3D camera = { 0 };
    camera.position = CAMERA_OFFSET;  // Camera position
    camera.target = vec3{0.0, 10.0, 10.0};      // Camera looking at point
    camera.up = vec3{ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 60.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera mode type
    // cameraMode = CAMERA_THIRD_PERSON;

// Load textures
    // After initializing the camera
    RenderTexture2D dofTexture = LoadRenderTexture(screenWidth, screenHeight);
    if (dofTexture.id == 0) {
        printf("Failed to create dofTexture\n");
        return -1;
    }

    rl::Shader dofShader (0,TextFormat("resources/shaders/dof.fs", GLSL_VERSION));
    if (dofShader.id == 0) {
        printf("Failed to compile DoF shader\n");
        return -1;
    }
    float blurRadius = 3.0f;  // Control the blur strength
    SetShaderValue(dofShader, GetShaderLocation(dofShader, "resolution"), (float[2]){(float)screenWidth, (float)screenHeight}, SHADER_UNIFORM_VEC2);
    SetShaderValue(dofShader, GetShaderLocation(dofShader, "radius"), &blurRadius, SHADER_UNIFORM_FLOAT);
    int resolutionLoc = GetShaderLocation(dofShader, "resolution");
    int radiusLoc = GetShaderLocation(dofShader, "radius");
    if (resolutionLoc == -1 || radiusLoc == -1) {
        printf("Failed to get shader uniform locations\n");
        return -1;
    }


    rl::Shader shadowShader (TextFormat("resources/shaders/lighting.vs", GLSL_VERSION),
                       TextFormat("resources/shaders/lighting.fs", GLSL_VERSION));
    // Get some required shader locations
    shadowShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shadowShader, "viewPos");
    Vector3 lightDir = Vector3Normalize((Vector3){ 0.35f, -1.0f, -0.35f });
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


    Player player = Player(
        vec3{0.0,1.0,0.0},
        0.2,
        shadowShader);

    std::vector<Animal> animals;
    for (int i = 0; i < 10; i++) {  // Create 10 animals, for example
        animals.push_back(Animal(vec3{GetRandomFloat(-25, 25), 1.0f, GetRandomFloat(-25, 25)}, 5.0f, shadowShader));
    }

    Model cube = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
    cube.materials[0].shader = shadowShader;

    RenderTexture2D shadowMap = RenderUtils::LoadShadowmapRenderTexture(SHADOWMAP_RESOLUTION, SHADOWMAP_RESOLUTION);
    // For the shadowmapping algorithm, we will be rendering everything from the light's point of view
    Camera3D lightCam = (Camera3D){ 0 };
    lightCam.position = Vector3Normalize(vec3{-10.0, 70.0, 10.0});//Vector3Scale(lightDir, -15.0f);
    lightCam.target = Vector3Zero();
    // Use an orthographic projection for directional lights
    lightCam.projection = CAMERA_ORTHOGRAPHIC;
    lightCam.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    lightCam.fovy = 50.0f;

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
        int fc = 0;

    SetExitKey(KEY_NULL);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------

         float dt = GetFrameTime();

      handle_collisions(player, animals);
      player.tether.update(camera);
      player.update();
      player.rope.update(player.pos, player.tether.pos);
        for (auto& animal : animals) {
            animal.update();
        }
      RenderUtils::update_camera(camera, player);

        Vector3 cameraPos = camera.position;
        SetShaderValue(shadowShader, shadowShader.locs[SHADER_LOC_VECTOR_VIEW], &cameraPos, SHADER_UNIFORM_VEC3);

                const float cameraSpeed = 0.05f;
        if (IsKeyDown(KEY_LEFT))
        {
            if (lightDir.x < 0.6f)
                lightDir.x += cameraSpeed * 60.0f * dt;
        }
        if (IsKeyDown(KEY_RIGHT))
        {
            if (lightDir.x > -0.6f)
                lightDir.x -= cameraSpeed * 60.0f * dt;
        }
        if (IsKeyDown(KEY_UP))
        {
            if (lightDir.z < 0.6f)
                lightDir.z += cameraSpeed * 60.0f * dt;
        }
        if (IsKeyDown(KEY_DOWN))
        {
            if (lightDir.z > -0.6f)
                lightDir.z -= cameraSpeed * 60.0f * dt;
        }
        lightDir = Vector3Normalize(lightDir);
        lightCam.position = Vector3Scale(lightDir, -15.0f);
        SetShaderValue(shadowShader, lightDirLoc, &lightDir, SHADER_UNIFORM_VEC3);


        // Draw
        //----------------------------------------------------------------------------------
        Matrix lightView;
        Matrix lightProj;
        BeginTextureMode(shadowMap);
        ClearBackground(WHITE);
        BeginMode3D(lightCam);
            lightView = rlGetMatrixModelview();
            lightProj = rlGetMatrixProjection();
            RenderUtils::draw_scene(cube, player, animals);
        EndMode3D();
        EndTextureMode();

        BeginTextureMode(dofTexture);
            Matrix lightViewProj = MatrixMultiply(lightView, lightProj);


            ClearBackground(RAYWHITE);

            SetShaderValueMatrix(shadowShader, lightVPLoc, lightViewProj);

            rlEnableShader(shadowShader.id);
            int slot = 10; // Can be anything 0 to 15, but 0 will probably be taken up
            rlActiveTextureSlot(10);
            rlEnableTexture(shadowMap.depth.id);
            rlSetUniform(shadowMapLoc, &slot, SHADER_UNIFORM_INT, 1);

            rlDisableShader();
            BeginMode3D(camera);

            // Update light values (ensure this is called in the main game loop)
                RenderUtils::draw_scene(cube, player, animals);

            EndMode3D();

        EndTextureMode();

        BeginDrawing();
            ClearBackground(RAYWHITE);

    // Use the blur shader
            BeginShaderMode(dofShader);
                // Draw the render texture to the screen with the shader
                DrawTexture(dofTexture.texture, 0, 0, WHITE);

            EndShaderMode();
            DrawText("Welcome to the third dimension!", 10, 40, 20, DARKGRAY);

            DrawFPS(10, 10);
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    RenderUtils::UnloadResources(
        shadowShader,
        player,
        animals,
        shadowMap,
        cube,
        dofShader,
        dofTexture
    );

    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
