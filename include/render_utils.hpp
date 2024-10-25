#pragma once
#include <stdio.h>

#include <cstring> // For strncpy
#include <vector>

#include "animal.hpp"
#include "buildings.hpp"
#include "player.hpp"
#include "raylib-cpp.hpp"
#include "rlgl.h"
#include "terrain.hpp"
#include "utils.hpp"

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION 330
#else // PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION 100
#endif

#define SHADOWMAP_RESOLUTION 2048

namespace RenderUtils {

void InitializeWindow(int &screenWidth, int &screenHeight);

Camera3D SetupCamera();

Camera3D SetupLightCamera();

RenderTexture2D LoadShadowmapRenderTexture(int width, int height);
void UnloadShadowmapRenderTexture(RenderTexture2D target);

void update_camera(GameState &GameState);

bool is_in_camera_view(const Vector3 &position, float radius, const Camera &camera, int screenWidth,
                       int screenHeight);

void draw_scene(GameState &GameState);

void UnloadResources(Shader shadowShader, RenderTexture2D shadowMap, GameState &GameState,
                     Shader dofShader, RenderTexture2D dofTexture);

RenderTexture2D SetupDofTexture(int screenWidth, int screenHeight);

rl::Shader SetupDofShader(int screenWidth, int screenHeight);

rl::Shader SetupShadowShader(vec3 &lightDir);

void RenderShadowMap(Shader shadowShader, RenderTexture2D &shadowMap, Camera3D &lightCam,
                     GameState &GameState);

void RenderSceneToTexture(RenderTexture2D &dofTexture, Camera3D &camera, rl::Shader &shadowShader,
                          RenderTexture2D &shadowMap, GameState &GameState);

void HandleWindowResize(GameState &GameState, int &screenWidth, int &screenHeight,
                        RenderTexture2D &dofTexture, Shader &dofShader);

void DrawGUI(GameState &GameState, int &screenWidth, int &screenHeight);
} // namespace RenderUtils
