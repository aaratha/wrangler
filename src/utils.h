#pragma once

#include "raylib-cpp.hpp"

namespace rl = raylib;  // Create an alias for the raylib namespace
using vec3 = rl::Vector3;  // Define vec3 as an alias for raylib's Vector3

// Function declarations
float lerp_to(float position, float target, float rate);
vec3 lerp3D(vec3 position, vec3 target, float rate);
float GetRandomFloat(float min, float max);
