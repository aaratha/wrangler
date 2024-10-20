#pragma once

#include "animal.hpp"
#include "buildings.hpp"
#include "player.hpp"
#include "utils.hpp"
#include <stdatomic.h>
#include <tuple>
#include <unordered_map>
#include <utility> // For std::pair
#include <vector>

struct pair_hash {
  std::size_t operator()(const std::pair<int, int> &p) const {
    return std::hash<int>()(p.first) ^ std::hash<int>()(p.second);
  }
};

using GridKey = std::pair<int, int>;
using Grid = std::unordered_map<GridKey, std::vector<Animal *>, pair_hash>;

constexpr int GRID_SIZE = 5;

void handle_collisions(GameState &GameState, int &substeps,
                       std::vector<std::unique_ptr<Pen>> &pens);

GridKey get_grid_key(const vec3 &pos, float grid_size);
void add_to_grid(Grid &grid, Animal *animal, float grid_size);
void check_grid_collisions(const Grid &grid, const GridKey &key,
                           const float animalRadius, const float playerRadius,
                           GameState &GameState);
