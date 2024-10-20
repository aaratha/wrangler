#pragma once

#include "utils.hpp"
#include "raylib-cpp.hpp"

class Terrain {
public:
    vec3 pos;
    Model model;

    Terrain(Shader shadowShader);
    void draw();
};
