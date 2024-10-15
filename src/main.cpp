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


std::vector<Animal> CreateAnimals(const rl::Shader& shadowShader, int count = 10) {
    std::vector<Animal> animals;
    for (int i = 0; i < count; i++) {
        animals.push_back(Animal(vec3{GetRandomFloat(-25, 25), 1.0f, GetRandomFloat(-25, 25)}, 5.0f, shadowShader));
    }
    return animals;
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

        RenderUtils::RenderShadowMap(shadowShader, shadowMap, lightCam, cube, player, animals);

        // Render scene
        RenderUtils::RenderSceneToTexture(dofTexture, camera, shadowShader, shadowMap, cube, player, animals);


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
        RenderUtils::InitializeWindow(screenWidth, screenHeight);

        vec3 lightDir = Vector3Normalize((Vector3){ 0.35f, -1.0f, -0.35f });
        Camera3D camera = RenderUtils::SetupCamera();
        RenderTexture2D dofTexture = RenderUtils::SetupDofTexture(screenWidth, screenHeight);
        rl::Shader dofShader = RenderUtils::SetupDofShader(screenWidth, screenHeight);
        rl::Shader shadowShader = RenderUtils::SetupShadowShader(lightDir);

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
