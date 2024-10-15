#pragma once

#include "raylib-cpp.hpp"

namespace rl = raylib;  // Create an alias for the raylib namespace
using vec3 = rl::Vector3;  // Define vec3 as an alias for raylib's Vector3

const float speed = 0.2;
const vec3 CAMERA_OFFSET = {0.0, 15.0, 8.0};

// Function declarations
float lerp_to(float position, float target, float rate);
vec3 lerp3D(vec3 position, vec3 target, float rate);
float GetRandomFloat(float min, float max);
// Helper function to get the closest point on a line segment to a point
vec3 GetClosestPointOnLineFromPoint(vec3 point, vec3 lineStart, vec3 lineEnd);

// Helper function to check collision between a point and a line segment
bool CheckCollisionPointLine(vec3 point, vec3 lineStart, vec3 lineEnd, float threshold);
