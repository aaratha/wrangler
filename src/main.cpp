#include "raylib-cpp.hpp"
#include "raymath.h"
#include "rlgl.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "utils.hpp"
#include "player.hpp"
#include "animal.hpp"
#include "physics.hpp"
#include "render_utils.hpp"
#include "buildings.hpp"
#include "terrain.hpp"

#include <random>
#include <limits>
#include <vector>
#include <iostream>
#include <array>

const float PHYSICS_TIME = 1.0/60.0;



void GameLoop(vec3 lightDir, RenderTexture2D& shadowMap, rl::Shader& shadowShader,
              rl::Shader& dofShader, RenderTexture2D& dofTexture, int screenWidth, int screenHeight,
              GameState& GameState) {
    float accumulator = 0.0;
    int substeps = 8;
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        handle_building(GameState, GameState.camera);
        accumulator += dt;
        while (accumulator >= PHYSICS_TIME) {
            // Update game state
            GameState.mouse_proj = project_mouse(1.0, GameState.camera);
            handle_collisions(GameState, substeps, GameState.pens);
            GameState.player->tether.update(GameState.camera, GameState, GameState.player->pos);
            GameState.player->update();
            GameState.player->rope.update(GameState.player->pos, GameState.player->tether.pos);
            for (auto& animal : GameState.animals) {
                animal->update();
            }
            RenderUtils::update_camera(GameState.camera, GameState.player);
            // Update shaders
            Vector3 cameraPos = GameState.camera.position;
            SetShaderValue(shadowShader, shadowShader.locs[SHADER_LOC_VECTOR_VIEW], &cameraPos, SHADER_UNIFORM_VEC3);

            const float cameraSpeed = 0.05f;
            if (IsKeyDown(KEY_LEFT)) {
                if (lightDir.x < 0.6f)
                    lightDir.x += cameraSpeed * 60.0f * dt;
            }
            if (IsKeyDown(KEY_RIGHT)) {
                if (lightDir.x > -0.6f)
                    lightDir.x -= cameraSpeed * 60.0f * dt;
            }
            if (IsKeyDown(KEY_UP)) {
                if (lightDir.z < 0.6f)
                    lightDir.z += cameraSpeed * 60.0f * dt;
            }
            if (IsKeyDown(KEY_DOWN)) {
                if (lightDir.z > -0.6f)
                    lightDir.z -= cameraSpeed * 60.0f * dt;
            }
            if (IsKeyDown(KEY_ONE)) {
                GameState.itemActive = 0;
            }
            if (IsKeyDown(KEY_TWO)) {
                GameState.itemActive = 1;
            }
            if (IsKeyDown(KEY_THREE)) {
                GameState.itemActive = 2;
            }
            accumulator -= PHYSICS_TIME;
        }
        lightDir = Vector3Normalize(lightDir);
        GameState.lightCam.position = Vector3Scale(lightDir, -15.0f);
        int lightDirLoc = GetShaderLocation(shadowShader, "lightDir");
        SetShaderValue(shadowShader, lightDirLoc, &lightDir, SHADER_UNIFORM_VEC3);

        RenderUtils::RenderShadowMap(shadowShader, shadowMap, GameState.lightCam, GameState);

        // Render scene
        RenderUtils::RenderSceneToTexture(dofTexture, GameState.camera, shadowShader, shadowMap, GameState);

        RenderUtils::HandleWindowResize(screenWidth, screenHeight, dofTexture, dofShader);

        // Render final image
        BeginDrawing();
            ClearBackground(RAYWHITE);
            BeginShaderMode(dofShader);
                DrawTexture(dofTexture.texture, 0, 0, WHITE);
            EndShaderMode();
            RenderUtils::DrawGUI(GameState, screenWidth, screenHeight);
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
        RenderTexture2D dofTexture = RenderUtils::SetupDofTexture(screenWidth, screenHeight);
        rl::Shader dofShader = RenderUtils::SetupDofShader(screenWidth, screenHeight);
        rl::Shader shadowShader = RenderUtils::SetupShadowShader(lightDir);
        GameState GameState(shadowShader);


        RenderTexture2D shadowMap = RenderUtils::LoadShadowmapRenderTexture(SHADOWMAP_RESOLUTION, SHADOWMAP_RESOLUTION);
        //Camera3D lightCam = RenderUtils::SetupLightCamera();


        SetTargetFPS(165);
                int fc = 0;

        SetExitKey(KEY_NULL);

        GameLoop(
            lightDir,
            shadowMap,
            shadowShader,
            dofShader,
            dofTexture,
            screenWidth,
            screenHeight,
            GameState
        );

        RenderUtils::UnloadResources(shadowShader, shadowMap, GameState, dofShader, dofTexture);
    }
    catch (const std::exception& e) {
        TraceLog(LOG_ERROR, "An error occurred: %s", e.what());
    }

    CloseWindow();
    return 0;
}
