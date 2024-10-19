#pragma once

#include "raylib-cpp.hpp"
#include <memory>

namespace rl = raylib;  // Create an alias for the raylib namespace
using vec3 = rl::Vector3;  // Define vec3 as an alias for raylib's Vector3
using vec2 = rl::Vector2;

const float speed = 0.2;
const vec3 CAMERA_OFFSET = {0.0, 15.0, 8.0};

class Pen;
class Fence;
class Animal;
class Player;

class GameState {
    public:
    bool toggleFence;
    int itemActive;
    Camera3D camera;
    Camera3D lightCam;
    std::unique_ptr<Player> player;
    std::vector<std::unique_ptr<Animal>> animals;
    std::unique_ptr<Fence> fence;  // Use unique_ptr for automatic memory management
    std::vector<std::unique_ptr<Pen>> pens;  // Use unique_ptr here as well
    vec2 mouse_proj;

    GameState(const rl::Shader& shadowShader);
};

// Function declarations
float lerp_to(float position, float target, float rate);
vec3 lerp3D(vec3 position, vec3 target, float rate);
float GetRandomFloat(float min, float max);
// Helper function to get the closest point on a line segment to a point
vec3 GetClosestPointOnLineFromPoint(vec3 point, vec3 lineStart, vec3 lineEnd);

// Helper function to check collision between a point and a line segment
bool CheckCollisionPointLine(vec3 point, vec3 lineStart, vec3 lineEnd, float threshold);

vec3 vec2to3(vec2 vec2, float y);

vec2 project_mouse(float y, Camera3D camera);
