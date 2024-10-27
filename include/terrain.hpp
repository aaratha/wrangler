#pragma once

#include "raylib-cpp.hpp"
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
  void update(float deltaTime);  // New update method
  Blade blade;
  int bladeCount;
  Model planeModel;
  Matrix* transforms;
  float windTime;  // New wind time accumulator
};
