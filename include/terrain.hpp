#pragma once

#include "raylib-cpp.hpp"
#include "utils.hpp"

class Terrain {
 public:
  vec3 pos;
  Model model;

  Terrain(Shader shadowShader);
  void draw();
};
