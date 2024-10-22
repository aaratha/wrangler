#pragma once

#include <cmath>
#include <cstdlib>  // For rand()

#include "utils.hpp"

class Coin {
 public:
  Coin(std::vector<vec3> bounds);
  vec3 pos;
  float radius = 0.2;
  void draw();
};
