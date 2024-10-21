#pragma once

#include "animal.hpp"
#include "raylib-cpp.hpp"
#include "utils.hpp"
#include "collectables.hpp"
#include <iostream> // For printing
#include <vector>

// Axis-Aligned Bounding Box (AABB) struct
struct AABB {
  vec3 min;
  vec3 max;
};

class Pen {
private:
  float coinTimer = 0.0f; // Timer to accumulate time for coin addition
  const float coinInterval = 5.0f; // 1 second interval for adding coins
public:
  std::vector<vec3> fixed_points;
  std::vector<Animal *> contained_animals;
  std::vector<Coin> contained_coins;
  Species species = Species(SpeciesType::NULL_SPECIES);
  float rope_segment_length = 1.0f; // Desired length between rope points
  float constraint = 0.4f;          // Maximum distance between rope points
  float friction = 0.99f;           // Friction coefficient
  float thickness = 0.1f;           // Thickness of the rope
  int sides = 8;                    //[<35;139;19M]
  std::vector<std::vector<vec3>> rope_points;
  std::vector<std::vector<vec3>> rope_velocities;
  void initializeRopePoints();
  Pen(std::vector<vec3> points);
  void spawnCoins();
  void update(float dt);
  void draw();
};

class Fence {
public:
  std::vector<vec2> points;
  float joinDist;
  Fence();
  void place(vec2 point, std::vector<std::unique_ptr<Pen>> &pens);
  void undo();
  void draw(GameState &GameState);
};

void handle_building(GameState &GameState, Camera3D camera);

AABB compute_aabb(const Pen &pen);
bool is_point_in_polygon(const vec3 &point, const Pen &pen);
void detect_animals_in_pens(
    std::vector<std::unique_ptr<Pen>> &pens,
    const std::vector<std::unique_ptr<Animal>> &animals);
