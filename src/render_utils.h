#pragma once
#include "utils.h"
#include "raylib-cpp.hpp"
#include "rlgl.h"
#include "player.h"
#include "animal.h"
#include <vector>

RenderTexture2D LoadShadowmapRenderTexture(int width, int height);
void UnloadShadowmapRenderTexture(RenderTexture2D target);

RenderTexture2D LoadShadowmapRenderTexture(int width, int height);

// Unload shadowmap render texture from GPU memory (VRAM)
void UnloadShadowmapRenderTexture(RenderTexture2D target);

void draw_scene(Model cube, Player player, std::vector<Animal> animals);
