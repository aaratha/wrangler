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


std::vector<Animal> CreateAnimals(const rl::Shader& shadowShader, int count = 10) {
    std::vector<Animal> animals;
    for (int i = 0; i < count; i++) {
        animals.push_back(Animal(vec3{GetRandomFloat(-25, 25), 1.0f, GetRandomFloat(-25, 25)}, 5.0f, shadowShader));
    }
    return animals;
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


void GameLoop(vec3 lightDir, Camera3D& camera, Player& player, std::vector<Animal>& animals, Model& cube,
              RenderTexture2D& shadowMap, Camera3D& lightCam, rl::Shader& shadowShader,
              rl::Shader& dofShader, RenderTexture2D& dofTexture) {
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // Update game state
        handle_collisions(player, animals);
        player.tether.update(camera);
        player.update();
        player.rope.update(player.pos, player.tether.pos);
        for (auto& animal : animals) {
            animal.update();
        }
        RenderUtils::update_camera(camera, player);

        // Update shaders
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
        int lightDirLoc = GetShaderLocation(shadowShader, "lightDir");
        SetShaderValue(shadowShader, lightDirLoc, &lightDir, SHADER_UNIFORM_VEC3);

        RenderShadowMap(shadowShader, shadowMap, lightCam, cube, player, animals);

        // Render scene
        RenderSceneToTexture(dofTexture, camera, shadowShader, shadowMap, cube, player, animals);


        // Render final image
        BeginDrawing();
            ClearBackground(RAYWHITE);
            BeginShaderMode(dofShader);
                DrawTexture(dofTexture.texture, 0, 0, WHITE);
            EndShaderMode();
            DrawText("Welcome to the third dimension!", 10, 40, 20, DARKGRAY);
            DrawFPS(10, 10);
        EndDrawing();
    }
}


int main(void) {
    int screenWidth = 1280;
    int screenHeight = 720;

    try {
        InitializeWindow(screenWidth, screenHeight);

        vec3 lightDir = Vector3Normalize((Vector3){ 0.35f, -1.0f, -0.35f });
        Camera3D camera = SetupCamera();
        RenderTexture2D dofTexture = SetupDofTexture(screenWidth, screenHeight);
        rl::Shader dofShader = SetupDofShader(screenWidth, screenHeight);
        rl::Shader shadowShader = SetupShadowShader(lightDir);

        Player player(vec3{0.0,1.0,0.0}, 0.2, shadowShader);
        std::vector<Animal> animals = CreateAnimals(shadowShader);

        Model cube = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
        cube.materials[0].shader = shadowShader;

        RenderTexture2D shadowMap = RenderUtils::LoadShadowmapRenderTexture(SHADOWMAP_RESOLUTION, SHADOWMAP_RESOLUTION);
        //Camera3D lightCam = RenderUtils::SetupLightCamera();
        Camera3D lightCam = (Camera3D){ 0 };
        lightCam.position = Vector3Normalize(vec3{-10.0, 70.0, 10.0});//Vector3Scale(lightDir, -15.0f);
        lightCam.target = Vector3Zero();
        // Use an orthographic projection for directional lights
        lightCam.projection = CAMERA_ORTHOGRAPHIC;
        lightCam.up = (Vector3){ 0.0f, 1.0f, 0.0f };
        lightCam.fovy = 50.0f;


        SetTargetFPS(60);
                int fc = 0;

        SetExitKey(KEY_NULL);

        GameLoop(lightDir, camera, player, animals, cube, shadowMap, lightCam, shadowShader, dofShader, dofTexture);

        RenderUtils::UnloadResources(shadowShader, player, animals, shadowMap, cube, dofShader, dofTexture);
    }
    catch (const std::exception& e) {
        TraceLog(LOG_ERROR, "An error occurred: %s", e.what());
    }

    CloseWindow();
    return 0;
}
