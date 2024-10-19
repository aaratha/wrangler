#pragma once

#include "raylib-cpp.hpp"
#include "utils.hpp"
#include "animal.hpp"
#include <vector>


class Pen {
private:
public:
  std::vector<vec3> fixed_points;
  std::vector<Animal> animals;
  float rope_segment_length = 1.0f; // Desired length between rope points
  float constraint = 0.4f; // Maximum distance between rope points
  float friction = 0.99f; // Friction coefficient
  float thickness = 0.1f; // Thickness of the rope
  int sides = 8; //[<35;139;19M]
  std::vector<std::vector<vec3>> rope_points;
  std::vector<std::vector<vec3>> rope_velocities;

  void initializeRopePoints();
  Pen(std::vector<vec3> points);
  void update();
  void draw();
};

class Fence {
  public:
  std::vector<vec2> points;
  float joinDist;
  Fence();
  void place(vec2 point, std::vector<std::unique_ptr<Pen>>& pens);
  void undo();
  void draw(GameState& GameState);
};


void handle_building(GameState& GameState, Camera3D camera);
