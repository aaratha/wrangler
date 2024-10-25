#pragma once

#include "raylib-cpp.hpp"
#include "utils.hpp"

class Blade {
public:
  vec3 pos;
  Model model;

  Blade(Shader shadowShader, vec3 pos);
  void draw();
};

class Terrain {
public:
  vec3 pos;
  Model planeModel;
  Blade blade;
  int bladeCount;
  Matrix *transforms;
  std::vector<Blade> grass;
  Shader instanceShader;
  Terrain(Shader shadowShader, int bladeCount);
  void draw();
};
