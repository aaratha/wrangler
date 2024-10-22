#include "physics.hpp"

// Helper function to compute the grid cell based on position
GridKey get_grid_key(const vec3 &pos, float grid_size) {
  return {
      static_cast<int>(pos.x / grid_size),
      static_cast<int>(pos.z / grid_size)  // 2D grid, ignoring z-axis
  };
}

// Helper function to add objects to grid cells
void add_to_grid(Grid &grid, Animal *animal, float grid_size) {
  GridKey key = get_grid_key(animal->pos, grid_size);
  grid[key].push_back(animal);
}

void check_grid_collisions(const Grid &grid, const GridKey &key,
                           const float animalRadius, const float playerRadius,
                           GameState &GameState) {
  static const int neighbor_offsets[3] = {
      -1, 0, 1};  // To check neighboring cells in both x and z axes

  for (int dx : neighbor_offsets) {
    for (int dz : neighbor_offsets) {
      GridKey neighbor_key = {key.first + dx, key.second + dz};

      auto it = grid.find(neighbor_key);
      if (it != grid.end()) {
        const std::vector<Animal *> &nearbyAnimals = it->second;

        // Player vs Animals in nearby grid cells
        for (Animal *animal : nearbyAnimals) {
          if (CheckCollisionSpheres(GameState.player->pos, playerRadius,
                                    animal->pos, animalRadius)) {
            // Handle player-animal collision
            vec3 collisionNormal = Vector3Normalize(
                Vector3Subtract(animal->pos, GameState.player->pos));
            float overlap = playerRadius + animalRadius -
                            Vector3Distance(GameState.player->pos, animal->pos);
            GameState.player->pos =
                Vector3Subtract(GameState.player->pos,
                                Vector3Scale(collisionNormal, overlap * 0.5f));
            animal->pos = Vector3Add(
                animal->pos, Vector3Scale(collisionNormal, overlap * 0.5f));
          }
        }

        // Animal vs Animal within the same nearby grid cells
        for (size_t i = 0; i < nearbyAnimals.size(); ++i) {
          for (size_t j = i + 1; j < nearbyAnimals.size(); ++j) {
            if (CheckCollisionSpheres(nearbyAnimals[i]->pos, animalRadius,
                                      nearbyAnimals[j]->pos, animalRadius)) {
              // Handle animal-animal collision
              vec3 collisionNormal = Vector3Normalize(Vector3Subtract(
                  nearbyAnimals[j]->pos, nearbyAnimals[i]->pos));
              float overlap =
                  2 * animalRadius -
                  Vector3Distance(nearbyAnimals[i]->pos, nearbyAnimals[j]->pos);
              nearbyAnimals[i]->pos = Vector3Subtract(
                  nearbyAnimals[i]->pos,
                  Vector3Scale(collisionNormal, overlap * 0.5f));
              nearbyAnimals[j]->pos =
                  Vector3Add(nearbyAnimals[j]->pos,
                             Vector3Scale(collisionNormal, overlap * 0.5f));
            }
          }
        }
      }
    }
  }
}

// Check collisions within a grid cell and its neighbors
void handle_collisions(GameState &GameState, int &substeps,
                       std::vector<std::unique_ptr<Pen>> &pens) {
  const float playerRadius = 1.5f;
  const float tetherRadius = 0.5f;       // From the Tether constructor
  const float ropeSegmentRadius = 0.5f;  // From the Rope constructor

  // Player cube vs Animals
  for (int i = 0; i < substeps; i++) {
    Grid grid;

    for (auto &animal : GameState.animals) {
      add_to_grid(grid, animal.get(), GRID_SIZE);
    }

    // Step 2: Perform collision detection using grid
    for (auto &animal : GameState.animals) {
      GridKey key = get_grid_key(animal->pos, GRID_SIZE);
      check_grid_collisions(grid, key, animal->species.radius, playerRadius,
                            GameState);
    }

    // rope and animals
    if (!IsKeyDown(KEY_LEFT_SHIFT)) {
      for (auto &animal : GameState.animals) {
        for (int i = 0; i < GameState.player->rope.num_points - 1; i++) {
          if (CheckCollisionPointLine(
                  animal->pos, GameState.player->rope.points[i],
                  GameState.player->rope.points[i + 1], ropeSegmentRadius)) {
            // Handle rope-animal collision
            vec3 closestPoint = GetClosestPointOnLineFromPoint(
                animal->pos, GameState.player->rope.points[i],
                GameState.player->rope.points[i + 1]);
            vec3 collisionNormal =
                Vector3Normalize(Vector3Subtract(animal->pos, closestPoint));
            float overlap = ropeSegmentRadius + animal->species.radius -
                            Vector3Distance(closestPoint, animal->pos);
            animal->targ = Vector3Add(
                animal->targ, Vector3Scale(collisionNormal, overlap * 0.8));

            // Displace rope points
            vec3 displacementVector =
                Vector3Scale(collisionNormal, overlap * 0.2f);
            GameState.player->rope.points[i] = Vector3Subtract(
                GameState.player->rope.points[i], displacementVector);
            GameState.player->rope.points[i + 1] = Vector3Subtract(
                GameState.player->rope.points[i + 1], displacementVector);
          }
        }
      }
    }

    // pens and animals
    for (auto &animal : GameState.animals) {
      for (auto &pen : GameState.pens) {
        for (size_t i = 0; i < pen->rope_points.size(); ++i) {
          for (size_t j = 0; j < pen->rope_points[i].size() - 1; ++j) {
            vec3 start = pen->rope_points[i][j];
            vec3 end = pen->rope_points[i][j + 1];

            // if (CheckCollisionPointLine(animal->pos, start, end,
            // ropeSegmentRadius)) {
            if (CheckCollisionSpheres(animal->pos, animal->species.radius,
                                      start, 0.1)) {
              vec3 closestPoint =
                  GetClosestPointOnLineFromPoint(animal->pos, start, end);
              vec3 collisionNormal =
                  Vector3Normalize(Vector3Subtract(animal->pos, closestPoint));
              float overlap = ropeSegmentRadius + animal->species.radius -
                              Vector3Distance(closestPoint, animal->pos);

              // Update animal target position
              animal->targ = Vector3Add(
                  animal->targ, Vector3Scale(collisionNormal, overlap * 0.8f));

              // Displace rope points (except fixed points)
              vec3 displacementVector =
                  Vector3Scale(collisionNormal, overlap * 0.2f);
              if (j > 0) {
                pen->rope_points[i][j] =
                    Vector3Subtract(pen->rope_points[i][j], displacementVector);
              }
              if (j < pen->rope_points[i].size() - 2) {
                pen->rope_points[i][j + 1] = Vector3Subtract(
                    pen->rope_points[i][j + 1], displacementVector);
              }
            }
          }
        }
        for (size_t i = 0; i < pen->fixed_points.size(); ++i) {
          if (CheckCollisionSpheres(animal->pos, animal->species.radius,
                                    pen->fixed_points[i], 1.0)) {
            vec3 collisionNormal = Vector3Normalize(
                Vector3Subtract(animal->pos, pen->fixed_points[i]));
            float overlap = animal->species.radius + 1.0 -
                            Vector3Distance(animal->pos, pen->fixed_points[i]);
            animal->pos =
                Vector3Add(animal->pos, Vector3Scale(collisionNormal, overlap));
          }
        }
      }
    }

    // Player tether vs Animals
    for (auto &animal : GameState.animals) {
      if (CheckCollisionSpheres(GameState.player->tether.pos, tetherRadius,
                                animal->pos, animal->species.radius)) {
        // Handle tether-animal collision
        vec3 collisionNormal = Vector3Normalize(
            Vector3Subtract(animal->pos, GameState.player->tether.pos));
        float overlap =
            tetherRadius + animal->species.radius -
            Vector3Distance(GameState.player->tether.pos, animal->pos);
        animal->pos =
            Vector3Add(animal->pos, Vector3Scale(collisionNormal, overlap));
      }
    }
  }
}
