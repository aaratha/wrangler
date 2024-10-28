#include "player.hpp"

Tether::Tether(Shader shader) : shader(shader) {
  pos = vec3{0.0, 1.0, 10.0};
  targ = vec3{0.0, 0.0, 10.0};

  model = LoadModelFromMesh(GenMeshSphere(radius, 20, 20));
  model.materials[0].shader = shader;
}

void Tether::update(const Camera3D& camera,
                    GameState& GameState,
                    vec3 playerPos) {
  // Get mouse position
  if (GameState.itemActive == 0) {
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && maxDistance < 15.0)
      maxDistance = lerp_to(maxDistance, 15.0, 0.1);
    if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT) && maxDistance > 4.0)
      maxDistance = lerp_to(maxDistance, 4.0, 0.1);
  }

  Vector2 mousePos = GetMousePosition();

  // Get the ray from the mouse position
  Ray ray = GetMouseRay(mousePos, camera);

  // Calculate intersection with XZ plane (Y = 0)
  // Using the formula: t = -plane.y / ray.direction.y
  float t = -ray.position.y / ray.direction.y;

  // Get the intersection point
  vec3 intersection = {ray.position.x + ray.direction.x * t,
                       1.0f,  // We're projecting onto XZ plane, so y = 0
                       ray.position.z + ray.direction.z * t};

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
  DrawModel(model, Vector3Zero(), 1.0f, GRAY);
}

Rope::Rope(vec3 playerPos,
           vec3 tetherPos,
           float thickness,
           int num_points,
           float constraint)
    : start(tetherPos),
      end(playerPos),
      thickness(thickness),
      num_points(num_points),
      constraint(constraint) {
  points = std::vector<vec3>(num_points);  // Initialize vector with num_points
  velocities = std::vector<vec3>(num_points);
  init_points();
}

void Rope::init_points() {
  for (int i = 0; i < num_points; ++i) {
    float t = static_cast<float>(i) / (num_points - 1);  // Normalized factor
    points[i] = Vector3Lerp(start, end, t);  // Calculate the position at t
  }
}

void Rope::add_point(vec3 playerPos) {
  if (num_points < max_points) {
    num_points++;
    points.push_back(playerPos);
    velocities.push_back(vec3{0, 0, 0});
  }
}

void Rope::remove_point() {
  if (num_points > min_points) {
    num_points--;
    points.pop_back();
    velocities.pop_back();
  }
}

void Rope::update(vec3 playerPos, vec3 tetherPos, float dt) {
  start = tetherPos;
  end = playerPos;
  points[0] = start;
  points[num_points - 1] = end;

  deltaTimer += dt;
  if (IsKeyDown(KEY_LEFT_SHIFT)) {
    color = RED;

  } else {
    color = GRAY;
  }
  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && num_points < max_points &&
      deltaTimer >= deltaInterval) {
    add_point(playerPos);
    deltaTimer = 0.0;
  } else if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT) && num_points > min_points &&
             deltaTimer >= deltaInterval) {
    remove_point();
    deltaTimer = 0.0;
  }

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
  for (int i = 0; i < num_points - 1; i++) {
    vec3 segment_dir = points[i + 1] - points[i];
    vec3 midpoint = points[i] + segment_dir * 0.6f;
    DrawCylinderEx(points[i], midpoint, thickness, thickness, sides, color);
  }
}

Player::Player(vec3 startPos, float speed, Shader shader)
    : pos(startPos),
      targ(startPos),
      movementSpeed(speed),
      tether(shader),
      rope(pos, targ, 0.1, 8, 0.01f),
      com(0.0, 0.0, 5.0),
      shader(shader) {
  weight = 0.3f;
  model = LoadModel("resources/models/character.glb");
  model.materials[0].shader = shader;
}

void Player::update() {
  vec3 direction = vec3(0.0f, 0.0f, 0.0f);  // Movement direction
  if (IsKeyDown(KEY_W)) {
    direction += vec3(0.0f, 0.0f, -1.0f);  // Move forward
  }
  if (IsKeyDown(KEY_S)) {
    direction += vec3(0.0f, 0.0f, 1.0f);  // Move backward
  }
  if (IsKeyDown(KEY_A)) {
    direction += vec3(-1.0f, 0.0f, 0.0f);  // Move left
  }
  if (IsKeyDown(KEY_D)) {
    direction += vec3(1.0f, 0.0f, 0.0f);  // Move right
  }

  // Normalize direction if it's non-zero to prevent diagonal speed boost
  if (Vector3Length(direction) > 0.0f) {
    direction = Vector3Normalize(direction);
    vec3 forward = vec3(-1.0f, 0.0f, 0.0f);  // Default forward direction
    float newAngleTarg = atan2f(direction.x, direction.z);
    // Find the shortest path to the new angle
    float angleDiff = shortestAngleDifference(angleTarg, newAngleTarg);
    angleTarg = normalizeAngle(angleTarg + angleDiff);

    // Calculate tilt based on turning rate
    float turnRate = angleDiff;  // How fast we're turning
    float maxTiltAngle = -0.3f;  // Maximum tilt in radians (about 30 degrees)
    tiltTarg =
        turnRate * maxTiltAngle;  // Negative because we want to tilt into turns
    tiltTarg =
        fmaxf(fminf(tiltTarg, maxTiltAngle), -maxTiltAngle);  // Clamp tilt
  } else {
    // If not moving, gradually return tilt to zero
    tiltTarg = 0.0f;
  }

  // Smoothly interpolate rotation angle
  float angleDiff = shortestAngleDifference(angle, angleTarg);
  angle = normalizeAngle(angle + angleDiff * 0.1f);

  // Smoothly interpolate tilt
  tilt = lerp_to(tilt, tiltTarg, 0.1);

  // Build rotation matrices
  Matrix rotationY = MatrixRotateY(angle - (3.14f / 2));
  Matrix rotationZ = MatrixRotateZ(-tilt);  // Apply tilt around Z axis
  Matrix combinedRotation = MatrixMultiply(rotationZ, rotationY);

  // Apply translation

  Matrix scaleMatrix =
      MatrixScale(1.5f, 1.5f, 1.5f);  // Adjust these values as needed
  Matrix rotationAndScale = MatrixMultiply(combinedRotation, scaleMatrix);

  Matrix translationMatrix = MatrixTranslate(pos.x, pos.y + 0.5f, pos.z);
  model.transform = MatrixMultiply(rotationAndScale, translationMatrix);

  // Apply movement speed
  targ += direction * movementSpeed;

  // Interpolate position smoothly
  pos = lerp3D(pos, targ, 0.4f);

  // Update center of mass (com)
  com = Vector3Add(Vector3Scale(pos, 1.0f - weight),
                   Vector3Scale(tether.pos, weight));
}

void Player::draw() {
  // Draw the cube with WHITE as base color (shader will modify it)
  DrawModel(model, Vector3Zero(), 1.0f, GRAY);
  // DrawModelEx(model, Vector3Zero(), vec3(0.0, 1.0, 0.0), 0.0,
  //            vec3(1.0, 1.0, 1.0), GRAY);
}
