#include "player.h"

Tether::Tether(Shader shader) : shader(shader) {
    pos = vec3{0.0, 1.0, 10.0};
    targ = vec3{0.0, 0.0, 10.0};

    model = LoadModelFromMesh(GenMeshSphere(0.5, 20, 20));
    model.materials[0].shader = shader;
}

void Tether::update(const Camera3D& camera, GameState GameState, vec3 playerPos) {
    // Get mouse position
    float maxDistance = 3.0;
    if (GameState.itemActive == 0) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) maxDistance = 10.0;
    }
    Vector2 mousePos = GetMousePosition();

    // Get the ray from the mouse position
    Ray ray = GetMouseRay(mousePos, camera);

    // Calculate intersection with XZ plane (Y = 0)
    // Using the formula: t = -plane.y / ray.direction.y
    float t = -ray.position.y / ray.direction.y;

    // Get the intersection point
    vec3 intersection = {
        ray.position.x + ray.direction.x * t,
        1.0f,  // We're projecting onto XZ plane, so y = 0
        ray.position.z + ray.direction.z * t
    };

     // Calculate direction from player to intersection
    Vector3 direction = Vector3Subtract(intersection, playerPos);

    // Limit the distance
    float distance = Vector3Length(direction);
    if (distance > maxDistance) {
        direction = Vector3Scale(Vector3Normalize(direction), maxDistance);
    }

    // Calculate new position
    Vector3 newPos = Vector3Add(playerPos, direction);

    // Lerp to the new position
    pos = lerp3D(pos, newPos, 0.3f);
    model.transform = MatrixTranslate(pos.x, pos.y, pos.z);
}

void Tether::draw() {
    DrawModel(model, Vector3Zero(), 1.0f, BLUE);
}


Rope::Rope(
    vec3 playerPos,
    vec3 tetherPos,
    float thickness,
    int num_points,
    float constraint
) : start(playerPos),
    end(tetherPos),
    thickness(thickness),
    num_points(num_points),
    constraint(constraint) {

    points = std::vector<vec3>(num_points); // Initialize vector with num_points
    velocities = std::vector<vec3>(num_points);
    init_points();
}

void Rope::init_points() {
    for (int i = 0; i < num_points; ++i) {
        float t = static_cast<float>(i) / (num_points - 1); // Normalized factor
        points[i] = Vector3Lerp(start, end, t); // Calculate the position at t
    }
}

void Rope::update(vec3 playerPos, vec3 tetherPos) {
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

void Rope::draw() {
    for (int i=0; i<num_points -1; i++) {
        DrawCylinderEx(
            points[i],
            points[i+1],
            thickness,
            thickness,
            sides,
            RED
        ); // Draw a cylinder with base at startPos and top at endPos
    }
}

Player::Player(vec3 startPos, float speed, Shader shader)
    : pos(startPos), targ(startPos), movementSpeed(speed),
    tether(shader), rope(pos, targ, 0.1, 15, 0.05f), com(0.0, 0.0, 5.0), shader(shader) {

    weight = 0.3f;
    model = LoadModelFromMesh(GenMeshCube(2.0, 2.0, 2.0));
    model.materials[0].shader = shader;
}


void Player::update() {
    vec3 direction = vec3(0.0f, 0.0f, 0.0f);  // Movement direction

    if (IsKeyDown(KEY_W)) {
        direction += vec3(0.0f, 0.0f, -1.0f);  // Move forward
    }
    if (IsKeyDown(KEY_S)) {
        direction += vec3(0.0f, 0.0f, 1.0f);   // Move backward
    }
    if (IsKeyDown(KEY_A)) {
        direction += vec3(-1.0f, 0.0f, 0.0f);  // Move left
    }
    if (IsKeyDown(KEY_D)) {
        direction += vec3(1.0f, 0.0f, 0.0f);   // Move right
    }

    // Normalize direction if it's non-zero to prevent diagonal speed boost
    if (Vector3Length(direction) > 0.0f) {
        direction = Vector3Normalize(direction);
    }

    // Apply movement speed
    targ += direction * movementSpeed;

    // Interpolate position smoothly
    pos = lerp3D(pos, targ, 0.4);

    // Update player model transformation
    model.transform = MatrixTranslate(pos.x, pos.y, pos.z);

    // Update center of mass (com)
    com = Vector3Add(Vector3Scale(pos, 1.0f - weight), Vector3Scale(tether.pos, weight));

}

void Player::draw() {
    // Draw the cube with WHITE as base color (shader will modify it)
    DrawModel(model, Vector3Zero(), 1.0f, RED);
}
