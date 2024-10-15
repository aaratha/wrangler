#pragma once

#include "utils.h"
#include "raylib-cpp.hpp"

class Tether {
public:
  vec3 pos;
  vec3 targ;
  Shader shader;
  Model model;

  Tether(Shader shader);

  void update(const Camera3D& camera);

  void draw();
};

class Rope {
public:
  vec3 start;
  vec3 end;
  float thickness;
  int num_points;
  float constraint;
  std::vector<vec3> points;
  std::vector<vec3> velocities;  // MAX_POINTS should be the number of points in the rope
  float friction = 0.98f;  // Friction factor (close to 1.0 means low friction, close to 0 means high friction)


  int sides = 10;

    Rope(
      vec3 playerPos,
      vec3 tetherPos,
      float thickness,
      int num_points,
      float constraint
    );

    void init_points();

    void update(vec3 playerPos, vec3 tetherPos);

  void draw();

};

class Player {
public:
    vec3 pos;
    vec3 targ;
    float movementSpeed;
    Tether tether;
    Rope rope;
    vec3 com;
    Shader shader;
    Model model;
    float weight = 0.1;
    //Rope rope = Rope(pos, tether);

    // Constructor
    Player(vec3 startPos, float speed, Shader shader);


    // Method to handle input and move the player
    void update();

    void draw();
};
