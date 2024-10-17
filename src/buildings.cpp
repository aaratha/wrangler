#include "buildings.h"

Fence::Fence() {
    points.clear();  // Initialize points vector
    joinDist = 1.0;
}

void Fence::place(vec2 point, std::vector<std::unique_ptr<Pen>>& pens) {
    if (points.size() > 2 && Vector2Distance(point, points[0]) < joinDist) {
      points.push_back(points[0]);
      pens.push_back(std::unique_ptr<Pen>(new Pen(points)));
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
    }
    DrawCircle3D(vec2to3(points[0], 1.0), joinDist, (Vector3){1.0, 0.0, 0.0}, 90, WHITE);
}

void Pen::update() {
        start = playerPos;
    end = tetherPos;
    points[0] = start;
    points[num_points - 1] = end;

    for (int i = 1; i < num_points - 1; ++i) {
        // Calculate the position difference from neighboring points
        vec3 vec2prev = points[i] - points[i - 1];
        vec3 vec2next = points[i + 1] - points[i];
        float dist2prev = Vector3Length(vec2prev);
        float dist2next = Vector3Length(vec2next);

        // Constrain distances
        if (dist2prev > constraint) {
            vec2prev = Vector3Scale(Vector3Normalize(vec2prev), constraint);
        }
        if (dist2next > constraint) {
            vec2next = Vector3Scale(Vector3Normalize(vec2next), constraint);
        }

        // Calculate the velocity based on the current and next position
        vec3 newPos = (points[i - 1] + vec2prev + points[i + 1] - vec2next) / 2;
        vec3 velocity = newPos - points[i];

        // Apply friction to the velocity
        velocity = Vector3Scale(velocity, friction);

        // Update the point's position using the velocity
        points[i] = points[i] + velocity;

        // Optionally, store this velocity for the next update if needed
        velocities[i] = velocity;
    }

}

void Pen::draw() {
for (int i=0; i<points.size() -1; i++) {
    DrawCylinderEx(
    vec3{points[i].x, 1.0, points[i].y},
    vec3{points[i+1].x, 1.0, points[i+1].y},
    0.1,
    0.1,
    10,
    BLUE
    ); // Draw a cylinder with base at startPos and top at endPos
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
