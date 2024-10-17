#pragma once

#include "raylib-cpp.hpp"
#include "utils.h"
#include "animal.h"
#include <vector>



class Pen {
  public:
  std::vector<vec2> points;
  std::vector<Animal> animals;

  Pen(std::vector<vec2> points) : points(points) {};
  void draw();
};

class Fence {
  public:
  std::vector<vec2> points;

  Fence();
  void place(vec2 point, std::vector<std::unique_ptr<Pen>>& pens);
  void undo();
  void draw();
};


void handle_building(GameState& GameState, Camera3D camera);
