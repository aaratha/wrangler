#pragma once

#include "raylib-cpp.hpp"
#include "utils.hpp"

class Tether {
 public:
  vec3 pos;
  vec3 targ;
  Shader shader;
  float radius = 0.4;
  Model model;
  float maxDistance = 4.0;

  Tether(Shader shader);

  void update(const Camera3D &camera, GameState &GameState, vec3 playerPos);

  void draw();
};

class Rope {
 private:
  float deltaTimer = 0.0f;  // Timer to accumulate time for coin addition
  const float deltaInterval = 0.02f;  //
 public:
  vec3 start;
  vec3 end;
  float thickness;
  Color color = GRAY;
  int num_points;
  int min_points = 8;
  int max_points = 15;
  float constraint;
  std::vector<vec3> points;
  std::vector<vec3> velocities;
  float friction = 0.999f;  // Friction factor (close to 1.0 means low friction,
                            // close to 0 means high friction)

  int sides = 10;

  Rope(vec3 playerPos, vec3 tetherPos, float thickness, int num_points,
       float constraint);

  void init_points();
  void add_point(vec3 playerPos);
  void remove_point();
  void update(vec3 playerPos, vec3 tetherPos, float dt);

  void draw();
};

class Player {
 public:
  vec3 pos;
  vec3 targ;
  float radius = 0.8;
  float angle = 0.0;
  float angleTarg = 0.0;
  float tilt = 0.0;
  float tiltTarg = 0.0;
  float movementSpeed;
  Tether tether;
  Rope rope;
  vec3 com;
  Shader shader;
  Model model;
  float weight = 0.1;
  // Rope rope = Rope(pos, tether);

  // Constructor
  Player(vec3 startPos, float speed, Shader shader);

  // Method to handle input and move the player
  void update();

  void draw();
};
