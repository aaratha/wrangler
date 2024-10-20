#pragma once

#include "utils.hpp"
#include <cmath>
#include <cstdlib> // For rand()

class Coin {
public:
  Coin(std::vector<vec3> bounds);
  vec3 pos;
  float radius = 0.2;
  void draw();
};
