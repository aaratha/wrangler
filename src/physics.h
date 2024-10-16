#pragma once

#include "player.h"
#include "animal.h"
#include "utils.h"
#include <vector>

void handle_collisions(Player& player, std::vector<Animal>& animals, int& substeps);
