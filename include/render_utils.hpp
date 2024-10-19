#pragma once
#include "utils.hpp"
#include "raylib-cpp.hpp"
#include "rlgl.h"
#include "player.hpp"
#include "animal.hpp"
#include "buildings.hpp"
#include <vector>
#include <stdio.h>
#include <cstring>  // For strncpy


#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

#define SHADOWMAP_RESOLUTION 2048

namespace RenderUtils {

    void InitializeWindow(int& screenWidth, int& screenHeight);

    Camera3D SetupCamera();

    RenderTexture2D LoadShadowmapRenderTexture(int width, int height);
    void UnloadShadowmapRenderTexture(RenderTexture2D target);

    void draw_scene(Model cube, Player player, std::vector<Animal> animals);

    void update_camera(Camera3D& camera, Player player);

    void UnloadResources(
        Shader shadowShader,
        Player player,
        std::vector<Animal> animals,
        RenderTexture2D shadowMap,
        Model cube,
        Shader dofShader,
        RenderTexture2D dofTexture
    );

    RenderTexture2D SetupDofTexture(int screenWidth, int screenHeight);

    rl::Shader SetupDofShader(int screenWidth, int screenHeight);

    rl::Shader SetupShadowShader(vec3& lightDir);

    void RenderShadowMap(
        Shader shadowShader,
        RenderTexture2D& shadowMap,
        Camera3D& lightCam,
        Model& cube,
        Player& player,
        std::vector<Animal>& animals,
        GameState& GameState
    );

    void RenderSceneToTexture(
        RenderTexture2D& dofTexture,
        Camera3D& camera,
        rl::Shader& shadowShader,
        RenderTexture2D& shadowMap,
        Model& cube,
        Player& player,
        std::vector<Animal>& animals,
        GameState& GameState
    );

    void HandleWindowResize(int& screenWidth, int& screenHeight, RenderTexture2D& dofTexture, Shader& dofShader);

    void DrawGUI(GameState& GameState, int& screenWidth, int& screenHeight);
}
