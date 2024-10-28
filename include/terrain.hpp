#pragma once

#include "raylib-cpp.hpp"
#include "render_utils.hpp"
#include "utils.hpp"

class Blade {
 public:
  Blade(Shader shadowShader, vec3 pos);
  Model model;
  vec3 pos;
};

class Terrain {
 public:
  Terrain(Shader shadowShader, int bladeCount);
  void draw();
  void update(GameState& GameState, float dt);  // New update method
  Blade blade;
  int bladeCount;
  Model planeModel;
  Matrix* transforms;
  float windTime;  // New wind time accumulator
};
