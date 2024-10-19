#pragma once

#include "player.hpp"
#include "animal.hpp"
#include "utils.hpp"
#include "buildings.hpp"
#include <vector>

void handle_collisions(
    Player& player,
    std::vector<Animal>& animals,
    int& substeps,
    std::vector<std::unique_ptr<Pen>>& pens
);
