#pragma once

#include "player.h"
#include "animal.h"
#include "utils.h"
#include "buildings.h"
#include <vector>

void handle_collisions(
    Player& player,
    std::vector<Animal>& animals,
    int& substeps,
    std::vector<std::unique_ptr<Pen>>& pens
);
