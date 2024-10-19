#pragma once

#include "player.hpp"
#include "animal.hpp"
#include "utils.hpp"
#include "buildings.hpp"
#include <vector>

void handle_collisions(
    GameState& GameState,
    int& substeps,
    std::vector<std::unique_ptr<Pen>>& pens
);
