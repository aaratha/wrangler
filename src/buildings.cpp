#include "buildings.hpp"

void Pen::initializeRopePoints() {
    rope_points.clear();
    rope_velocities.clear();

    for (size_t i = 0; i < fixed_points.size(); ++i) {
        size_t next_i = (i + 1) % fixed_points.size();
        vec3 start = fixed_points[i];
        vec3 end = fixed_points[next_i];

        float total_distance = Vector3Distance(start, end);
        int num_segments = std::max(1, static_cast<int>(total_distance / rope_segment_length));

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


void Pen::update() {
    for (size_t i = 0; i < fixed_points.size(); ++i) {
        size_t next_i = (i + 1) % fixed_points.size();
        vec3 start = fixed_points[i];
        vec3 end = fixed_points[next_i];

        auto& segment_points = rope_points[i];
        auto& segment_velocities = rope_velocities[i];

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

            vec3 newPos = Vector3Scale(Vector3Add(Vector3Add(segment_points[j - 1], vec2prev),
                                                    Vector3Subtract(segment_points[j + 1], vec2next)), 0.5f);
            vec3 velocity = Vector3Subtract(newPos, segment_points[j]);
            velocity = Vector3Scale(velocity, friction);

            segment_points[j] = Vector3Add(segment_points[j], velocity);
            segment_velocities[j] = velocity;
        }
    }
}

Fence::Fence() {
    points.clear();  // Initialize points vector
    joinDist = 1.0;
}

void Fence::place(vec2 point, std::vector<std::unique_ptr<Pen>>& pens) {
    if (points.size() > 2 && Vector2Distance(point, points[0]) < joinDist) {
      points.push_back(points[0]);
      std::vector<vec3> fixed_points;
      for (auto& point : points) {
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

void Fence::draw(GameState& GameState) {
    if (points.size() > 0) {
        if (Vector2Distance(points[0], GameState.mouse_proj) > joinDist) {
            DrawCylinderEx(
                vec2to3(points.back(), 1.0),
                vec2to3(GameState.mouse_proj, 1.0),
                0.1f,
                0.1f,
                10,
                WHITE
            );
        } else {
            DrawCylinderEx(
                vec2to3(points.back(), 1.0),
                vec2to3(points[0], 1.0),
                0.1f,
                0.1f,
                10,
                WHITE
            );
        }
    }
    if (points.size() < 2) return; // Avoid drawing if there are not enough points
    for (size_t i = 0; i < points.size() - 1; ++i) {
        DrawCylinderEx(
            vec2to3(points[i], 1.0),
            vec2to3(points[i+1], 1.0),
            0.1f,
            0.1f,
            10,
            WHITE
        );
        DrawCylinderEx(
            vec2to3(points[i], 0.0),
            vec2to3(points[i], 1.0),
            0.1f,
            0.1f,
            8,
            GRAY
        );
        if (i == points.size() - 2) {
            DrawCylinderEx(
                vec2to3(points[i+1], 0.0),
                vec2to3(points[i+1], 1.0),
                0.1f,
                0.1f,
                8,
                GRAY
            );
        }
    }
    DrawCircle3D(vec2to3(points[0], 1.0), joinDist, (Vector3){1.0, 0.0, 0.0}, 90, WHITE);
}


void Pen::draw() {
    for (const auto& segment : rope_points) {
        for (size_t i = 0; i < segment.size() - 1; i++) {
            Vector3 start = segment[i];
            Vector3 end = segment[i+1];
            DrawCylinderEx(
                start,
                end,
                thickness,
                thickness,
                sides,
                BLUE
            );
        }
    }
    for (const auto& post : fixed_points) {
        DrawCylinderEx(
            vec3{post.x, 0.0, post.z},
            vec3{post.x, 1.0, post.z},
            0.1,
            0.1,
            8,
            GRAY
        );
    }
}


void handle_building(GameState& gameState, Camera3D camera) {
    Vector2 mousePos = GetMousePosition();
    Ray ray = GetMouseRay(mousePos, camera);

    // Check if the ray intersects the ground (y = 0 plane)
    if (ray.direction.y != 0) { // Prevent division by zero
        float t = (1.0f - ray.position.y) / ray.direction.y; // Calculate parameter t for y = 1.0
        if (t >= 0) { // Ensure the intersection is in front of the camera
            vec2 intersection = {
                ray.position.x + ray.direction.x * t,
                ray.position.z + ray.direction.z * t
            };

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
