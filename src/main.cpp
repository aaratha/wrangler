#include "raylib-cpp.hpp"
#include "raymath.h"
#include "rlgl.h"
#define RAYGUI_IMPLEMENTATION
#include "animal.hpp"
#include "buildings.hpp"
#include "physics.hpp"
#include "player.hpp"
#include "raygui.h"
#include "render_utils.hpp"
#include "terrain.hpp"
#include "utils.hpp"

#include <array>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

const float PHYSICS_TIME = 1.0 / 60.0;

void GameLoop(vec3 lightDir, RenderTexture2D &shadowMap,
              rl::Shader &shadowShader, rl::Shader &dofShader,
              RenderTexture2D &dofTexture, int screenWidth, int screenHeight,
              GameState &GameState) {
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
      GameState.player->tether.update(GameState.camera, GameState,
                                      GameState.player->pos);
      GameState.player->update();
      GameState.player->rope.update(GameState.player->pos,
                                    GameState.player->tether.pos);
      for (auto &animal : GameState.animals) {
        animal->update();
      }
      for (auto &pen : GameState.pens) {
        pen->update(GameState, dt);
      }
      detect_animals_in_pens(GameState.pens, GameState.animals);
      RenderUtils::update_camera(GameState.camera, GameState.player);
      // Update shaders
      Vector3 cameraPos = GameState.camera.position;
      SetShaderValue(shadowShader, shadowShader.locs[SHADER_LOC_VECTOR_VIEW],
                     &cameraPos, SHADER_UNIFORM_VEC3);

      update_lightDir(lightDir, dt);
      update_itemActive(GameState.itemActive);

      accumulator -= PHYSICS_TIME;
    }
    lightDir = Vector3Normalize(lightDir);
    GameState.lightCam.position = Vector3Scale(lightDir, -15.0f);
    int lightDirLoc = GetShaderLocation(shadowShader, "lightDir");
    SetShaderValue(shadowShader, lightDirLoc, &lightDir, SHADER_UNIFORM_VEC3);

    RenderUtils::RenderShadowMap(shadowShader, shadowMap, GameState.lightCam,
                                 GameState);

    // Render scene
    RenderUtils::RenderSceneToTexture(dofTexture, GameState.camera,
                                      shadowShader, shadowMap, GameState);

    RenderUtils::HandleWindowResize(screenWidth, screenHeight, dofTexture,
                                    dofShader);

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

    Font customFont = LoadFont("resources/fonts/Roboto-Regular.ttf"); // Load
    GuiSetFont(customFont); // Set the custom font

    GuiSetStyle(DEFAULT, TEXT_SIZE, 26); // Adjust size as needed

    vec3 lightDir = Vector3Normalize((Vector3){0.35f, -1.0f, -0.35f});
    RenderTexture2D dofTexture =
        RenderUtils::SetupDofTexture(screenWidth, screenHeight);
    rl::Shader dofShader =
        RenderUtils::SetupDofShader(screenWidth, screenHeight);
    rl::Shader shadowShader = RenderUtils::SetupShadowShader(lightDir);
    GameState GameState(shadowShader);

    RenderTexture2D shadowMap = RenderUtils::LoadShadowmapRenderTexture(
        SHADOWMAP_RESOLUTION, SHADOWMAP_RESOLUTION);
    // Camera3D lightCam = RenderUtils::SetupLightCamera();

    SetTargetFPS(165);
    int fc = 0;

    SetExitKey(KEY_NULL);

    GameLoop(lightDir, shadowMap, shadowShader, dofShader, dofTexture,
             screenWidth, screenHeight, GameState);

    RenderUtils::UnloadResources(shadowShader, shadowMap, GameState, dofShader,
                                 dofTexture);
    UnloadFont(customFont);
  } catch (const std::exception &e) {
    TraceLog(LOG_ERROR, "An error occurred: %s", e.what());
  }

  CloseWindow();
  return 0;
}
