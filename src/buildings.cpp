#include "buildings.hpp"

// Function to compute AABB for a pen
AABB compute_aabb(const Pen &pen) {
  vec3 min = pen.fixed_points[0];
  vec3 max = pen.fixed_points[0];

  for (const auto &point : pen.fixed_points) {
    min.x = std::min(min.x, point.x);
    min.z = std::min(min.z, point.z);
    max.x = std::max(max.x, point.x);
    max.z = std::max(max.z, point.z);
  }

  // Include the dynamic rope points in the AABB calculation
  for (const auto &rope_segment : pen.rope_points) {
    for (const auto &point : rope_segment) {
      min.x = std::min(min.x, point.x);
      min.z = std::min(min.z, point.z);
      max.x = std::max(max.x, point.x);
      max.z = std::max(max.z, point.z);
    }
  }

  return {min, max};
}

// Point-in-polygon test using the ray-casting algorithm
bool is_point_in_polygon(const vec3 &point, const Pen &pen) {
  int intersections = 0;
  for (size_t i = 0; i < pen.fixed_points.size(); i++) {
    vec3 start = pen.fixed_points[i];
    vec3 end =
        pen.fixed_points[(i + 1) %
                         pen.fixed_points.size()];  // Wrap around to form
                                                    // closed polygon

    // Check if the ray from 'point' intersects with the edge from start to end
    if ((point.z > start.z) != (point.z > end.z)) {
      float slope = (end.x - start.x) / (end.z - start.z);
      float intersectionX = start.x + (point.z - start.z) * slope;
      if (point.x < intersectionX) {
        intersections++;
      }
    }
  }
  return (intersections % 2 ==
          1);  // Odd intersections mean the point is inside
}

void detect_animals_in_pens(
    std::vector<std::unique_ptr<Pen>> &pens,
    const std::vector<std::unique_ptr<Animal>> &animals) {
  // Step 1: Assign each animal to the relevant pen
  for (auto &pen : pens) {
    pen->contained_animals.clear();  // Reset contained animals
    // Compute the AABB for quick rejection
    AABB pen_aabb = compute_aabb(*pen);
    // Step 2: For each animal, check if it's inside the pen
    for (const auto &animal : animals) {
      vec3 animal_pos = animal->pos;
      // Quick rejection via AABB test
      if (animal_pos.x < pen_aabb.min.x || animal_pos.x > pen_aabb.max.x ||
          animal_pos.z < pen_aabb.min.z || animal_pos.z > pen_aabb.max.z) {
        continue;  // Animal is outside the pen's AABB
      }
      if (is_point_in_polygon(animal_pos, *pen)) {
        pen->contained_animals.push_back(
            animal.get());  // Add to pen's contained animals
      }
    }

    // Step 3: Determine the species of the pen
    if (!pen->contained_animals.empty()) {
      SpeciesType first_species = pen->contained_animals[0]->species.type;
      bool all_same_species = true;

      for (const auto *animal : pen->contained_animals) {
        if (animal->species.type != first_species) {
          all_same_species = false;
          break;
        }
      }

      if (all_same_species) {
        pen->species = Species(first_species);
      } else {
        pen->species = Species(SpeciesType::NULL_SPECIES);
      }
    } else {
      pen->species = Species(SpeciesType::NULL_SPECIES);
    }
  }
}

void Pen::initializeRopePoints() {
  rope_points.clear();
  rope_velocities.clear();

  for (size_t i = 0; i < fixed_points.size(); ++i) {
    size_t next_i = (i + 1) % fixed_points.size();
    vec3 start = fixed_points[i];
    vec3 end = fixed_points[next_i];

    float total_distance = Vector3Distance(start, end);
    int num_segments =
        std::max(1, static_cast<int>(total_distance / rope_segment_length));

    std::vector<vec3> segment_points;
    std::vector<vec3> segment_velocities;

    for (int j = 0; j <= num_segments; ++j) {
      float t = static_cast<float>(j) / num_segments;
      segment_points.push_back(lerp3D(start, end, t));
      segment_velocities.push_back(Vector3Zero());
    }

    rope_points.push_back(segment_points);
    rope_velocities.push_back(segment_velocities);
  }
}

Pen::Pen(std::vector<vec3> points) : fixed_points(points) {
  initializeRopePoints();
}

void Pen::spawnCoin() {
  // Number of coins to spawn based on number of animals in the pen
  // Generate a random coin position within the pen's bounds (xz plane)
  Coin new_coin = Coin(fixed_points);  // Use fixed_points as the polygon bounds
  contained_coins.push_back(new_coin);  // Add the new coin to the vector
}

void Pen::update(GameState &GameState, float dt) {
  for (size_t i = 0; i < fixed_points.size(); ++i) {
    size_t next_i = (i + 1) % fixed_points.size();
    vec3 start = fixed_points[i];
    vec3 end = fixed_points[next_i];

    auto &segment_points = rope_points[i];
    auto &segment_velocities = rope_velocities[i];

    segment_points[0] = start;
    segment_points[segment_points.size() - 1] = end;

    for (size_t j = 1; j < segment_points.size() - 1; ++j) {
      vec3 vec2prev = Vector3Subtract(segment_points[j], segment_points[j - 1]);
      vec3 vec2next = Vector3Subtract(segment_points[j + 1], segment_points[j]);
      float dist2prev = Vector3Length(vec2prev);
      float dist2next = Vector3Length(vec2next);

      if (dist2prev > constraint) {
        vec2prev = Vector3Scale(Vector3Normalize(vec2prev), constraint);
      }
      if (dist2next > constraint) {
        vec2next = Vector3Scale(Vector3Normalize(vec2next), constraint);
      }

      vec3 newPos = Vector3Scale(
          Vector3Add(Vector3Add(segment_points[j - 1], vec2prev),
                     Vector3Subtract(segment_points[j + 1], vec2next)),
          0.5f);
      vec3 velocity = Vector3Subtract(newPos, segment_points[j]);
      velocity = Vector3Scale(velocity, friction);

      segment_points[j] = Vector3Add(segment_points[j], velocity);
      segment_velocities[j] = velocity;
    }
    // Coin collection logic
    coinTimer += dt;
    if (coinTimer >= coinInterval / contained_animals.size() &&
        species.type != SpeciesType::NULL_SPECIES) {
      spawnCoin();
      coinTimer = 0.0f;  // Reset the timer
    }

    // Check for collisions between player and coins
    for (auto it = contained_coins.begin(); it != contained_coins.end();) {
      Coin &coin = *it;
      if (checkCoinCollisions(GameState, coin)) {
        // Remove the coin from the contained_coins vector and increment the
        // GameState.coins count
        it = contained_coins.erase(it);
        GameState.coins++;
      } else {
        ++it;
      }
    }
  }
}

bool Pen::checkCoinCollisions(GameState &GameState, Coin &coin) {
  const float playerRadius = 1.5f;
  const float tetherRadius = 0.5f;

  // Check for collisions
  if (CheckCollisionSpheres(GameState.player->pos, playerRadius, coin.pos,
                            coin.radius)) {
    // std::cout << "Collision detected: Player and Coin" << std::endl;
    return true;
  }

  if (CheckCollisionSpheres(GameState.player->tether.pos, tetherRadius,
                            coin.pos, coin.radius)) {
    // std::cout << "Collision detected: Tether and Coin" << std::endl;
    return true;
  }

  if (CheckCollisionPolyline(coin.pos, coin.radius,
                             GameState.player->rope.points,
                             GameState.player->rope.thickness)) {
    // std::cout << "Collision detected: Rope and Coin" << std::endl;
    return true;
  }

  // std::cout << "No collision detected" << std::endl;
  return false;
}

Fence::Fence() {
  points.clear();  // Initialize points vector
  joinDist = 1.0;
}

void Fence::place(vec2 point, std::vector<std::unique_ptr<Pen>> &pens) {
  if (points.size() > 2 && Vector2Distance(point, points[0]) < joinDist) {
    points.push_back(points[0]);
    std::vector<vec3> fixed_points;
    for (auto &point : points) {
      fixed_points.push_back(vec2to3(point, 1.0));
    }
    pens.push_back(std::unique_ptr<Pen>(new Pen(fixed_points)));
    points.clear();
  } else {
    points.push_back(point);
  }
}

void Fence::undo() {
  if (points.size() > 0) {
    points.pop_back();
  }
}

void Fence::draw(GameState &GameState) {
  if (points.size() > 0) {
    if (Vector2Distance(points[0], GameState.mouse_proj) > joinDist) {
      DrawCylinderEx(vec2to3(points.back(), 1.0),
                     vec2to3(GameState.mouse_proj, 1.0), 0.1f, 0.1f, 10, WHITE);
    } else {
      DrawCylinderEx(vec2to3(points.back(), 1.0), vec2to3(points[0], 1.0), 0.1f,
                     0.1f, 10, WHITE);
    }
  }
  if (points.size() < 2)
    return;  // Avoid drawing if there are not enough points
  for (size_t i = 0; i < points.size() - 1; ++i) {
    DrawCylinderEx(vec2to3(points[i], 1.0), vec2to3(points[i + 1], 1.0), 0.1f,
                   0.1f, 10, WHITE);
    DrawCylinderEx(vec2to3(points[i], 0.0), vec2to3(points[i], 1.0), 0.1f, 0.1f,
                   8, GRAY);
    if (i == points.size() - 2) {
      DrawCylinderEx(vec2to3(points[i + 1], 0.0), vec2to3(points[i + 1], 1.0),
                     0.1f, 0.1f, 8, GRAY);
    }
  }
  DrawCircle3D(vec2to3(points[0], 1.0), joinDist, (Vector3){1.0, 0.0, 0.0}, 90,
               WHITE);
}

void Pen::draw() {
  for (const auto &segment : rope_points) {
    for (size_t i = 0; i < segment.size() - 1; i++) {
      Vector3 start = segment[i];
      Vector3 end = segment[i + 1];
      DrawCylinderEx(start, end, thickness, thickness, sides, species.color);
    }
  }
  for (const auto &post : fixed_points) {
    DrawCylinderEx(vec3{post.x, 0.0, post.z}, vec3{post.x, 1.0, post.z}, 0.1,
                   0.1, 8, GRAY);
  }
  for (auto &coin : contained_coins) {
    coin.draw();
  }
}

void handle_building(GameState &gameState, Camera3D camera) {
  Vector2 mousePos = GetMousePosition();
  Ray ray = GetMouseRay(mousePos, camera);

  // Check if the ray intersects the ground (y = 0 plane)
  if (ray.direction.y != 0) {  // Prevent division by zero
    float t = (1.0f - ray.position.y) /
              ray.direction.y;  // Calculate parameter t for y = 1.0
    if (t >= 0) {  // Ensure the intersection is in front of the camera
      vec2 intersection = {ray.position.x + ray.direction.x * t,
                           ray.position.z + ray.direction.z * t};

      if (gameState.itemActive == 1) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
          // Print debug info
          gameState.fence->place(intersection, gameState.pens);
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
          gameState.fence->undo();
        }
      }
    }
  }
}
