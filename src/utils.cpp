#include "utils.hpp"

#include "animal.hpp"
#include "buildings.hpp"
#include "player.hpp"
#include "render_utils.hpp"
#include "terrain.hpp"

GameState::GameState(const rl::Shader &shadowShader, const int screenWidth, const int screenHeight)
    : screenWidth(screenWidth), screenHeight(screenHeight), toggleFence(false), itemActive(0),
      coins(0), camera(RenderUtils::SetupCamera()), lightCam(RenderUtils::SetupLightCamera()),
      terrain(std::make_unique<Terrain>(shadowShader, 5000)),
      player(std::make_unique<Player>(vec3{0.0, 1.0, 0.0}, 0.2, shadowShader)),
      animals(CreateAnimals(shadowShader, 1)), fence(std::make_unique<Fence>()), pens() {
  // The unique_ptrs will automatically handle memory management
}

void GameState::addAnimal(const rl::Shader &shadowShader) {
  animals.push_back(std::make_unique<Animal>(
      vec3{GetRandomFloat(-25, 25), 1.0f, GetRandomFloat(-25, 25)}, 5.0f, shadowShader));
}

float lerp_to(float position, float target, float rate) {
  return position + (target - position) * rate; // Lerp between position and target
}

vec3 lerp3D(vec3 position, vec3 target, float rate) {
  return position + (target - position) * rate; // Lerp between vec3 position and target
}

float GetRandomFloat(float min, float max) {
  return min + (max - min) * ((float)GetRandomValue(0, RAND_MAX) / (float)RAND_MAX);
}

// Helper function to get the closest point on a line segment to a point
vec3 GetClosestPointOnLineFromPoint(vec3 point, vec3 lineStart, vec3 lineEnd) {
  vec3 line = Vector3Subtract(lineEnd, lineStart);
  float lineLength = Vector3Length(line);
  vec3 lineNormalized = Vector3Scale(line, 1.0f / lineLength);

  float t = Vector3DotProduct(Vector3Subtract(point, lineStart), lineNormalized);
  t = Clamp(t, 0, lineLength);

  return Vector3Add(lineStart, Vector3Scale(lineNormalized, t));
}

// Helper function to check collision between a point and a line segment
bool CheckCollisionPointLine(vec3 point, vec3 lineStart, vec3 lineEnd, float threshold) {
  vec3 closest = GetClosestPointOnLineFromPoint(point, lineStart, lineEnd);
  return Vector3Distance(point, closest) <= threshold;
}

vec3 vec2to3(vec2 vec2, float y) { return vec3{vec2.x, y, vec2.y}; }

vec2 project_mouse(float y, Camera3D camera) {
  Vector2 mousePos = GetMousePosition();
  Ray ray = GetMouseRay(mousePos, camera);

  // Check if the ray intersects the ground (y = 0 plane)
  if (ray.direction.y != 0) {                            // Prevent division by zero
    float t = (1.0f - ray.position.y) / ray.direction.y; // Calculate parameter t for y = 1.0
    if (t >= 0) { // Ensure the intersection is in front of the camera
      vec2 intersection = {ray.position.x + ray.direction.x * t,
                           ray.position.z + ray.direction.z * t};
      return intersection;
    }
  }
}

bool CheckCollisionPolyline(vec3 point, float radius, const std::vector<vec3> &polyline,
                            float thickness) {
  for (size_t i = 0; i < polyline.size() - 1; ++i) {
    if (CheckCollisionPointLine(point, polyline[i], polyline[i + 1], radius + thickness)) {
      return true;
    }
  }
  return false;
}

void update_lightDir(vec3 &lightDir, float dt) {
  const float cameraSpeed = 0.05f;
  if (IsKeyDown(KEY_LEFT)) {
    if (lightDir.x < 0.6f)
      lightDir.x += cameraSpeed * 60.0f * dt;
  }
  if (IsKeyDown(KEY_RIGHT)) {
    if (lightDir.x > -0.6f)
      lightDir.x -= cameraSpeed * 60.0f * dt;
  }
  if (IsKeyDown(KEY_UP)) {
    if (lightDir.z < 0.6f)
      lightDir.z += cameraSpeed * 60.0f * dt;
  }
  if (IsKeyDown(KEY_DOWN)) {
    if (lightDir.z > -0.6f)
      lightDir.z -= cameraSpeed * 60.0f * dt;
  }
}

void update_itemActive(int &itemActive) {
  if (IsKeyDown(KEY_ONE)) {
    itemActive = 0;
  }
  if (IsKeyDown(KEY_TWO)) {
    itemActive = 1;
  }
  if (IsKeyDown(KEY_THREE)) {
    itemActive = 2;
  }
}

// Helper function to triangulate the polygon (in the xz plane)
std::vector<std::array<vec2, 3>> triangulatePolygon(const std::vector<vec3> &bounds) {
  std::vector<std::array<vec2, 3>> triangles;
  for (size_t i = 1; i < bounds.size() - 1; ++i) {
    // Correctly construct std::array for the triangle
    std::array<vec2, 3> triangle = {
        vec2{bounds[0].x, bounds[0].z},        // First vertex
        vec2{bounds[i].x, bounds[i].z},        // Second vertex
        vec2{bounds[i + 1].x, bounds[i + 1].z} // Third vertex
    };

    // Push the triangle into the triangles vector
    triangles.push_back(triangle);
  }
  return triangles;
}

// Helper function to randomly select a triangle, weighted by area
std::array<vec2, 3> selectRandomTriangle(const std::vector<std::array<vec2, 3>> &triangles) {
  std::vector<float> areas;
  float totalArea = 0.0f;

  // Calculate the area of each triangle
  for (const auto &tri : triangles) {
    float area = calculateTriangleArea(tri[0], tri[1], tri[2]);
    areas.push_back(area);
    totalArea += area;
  }

  // Select a triangle based on the area
  float randomValue = static_cast<float>(rand()) / RAND_MAX * totalArea;
  float cumulativeArea = 0.0f;

  for (size_t i = 0; i < triangles.size(); ++i) {
    cumulativeArea += areas[i];
    if (randomValue <= cumulativeArea) {
      return triangles[i];
    }
  }

  // Fallback (shouldn't happen unless precision issues)
  return triangles.back();
}

// Helper function to calculate the area of a triangle (using Heron's formula in
// 2D)
float calculateTriangleArea(const vec2 &p1, const vec2 &p2, const vec2 &p3) {
  float a = sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2));
  float b = sqrt(pow(p3.x - p2.x, 2) + pow(p3.y - p2.y, 2));
  float c = sqrt(pow(p1.x - p3.x, 2) + pow(p1.y - p3.y, 2));
  float s = (a + b + c) / 2.0f;
  return sqrt(s * (s - a) * (s - b) * (s - c));
}

// Helper function to generate a random point within a triangle using
// barycentric coordinates
vec2 generateRandomPointInTriangle(const std::array<vec2, 3> &tri) {
  float r1 = static_cast<float>(rand()) / RAND_MAX;
  float r2 = static_cast<float>(rand()) / RAND_MAX;

  // Adjust r1 and r2 to ensure the point is within the triangle
  if (r1 + r2 > 1.0f) {
    r1 = 1.0f - r1;
    r2 = 1.0f - r2;
  }

  // Use barycentric coordinates to get the random point
  float x = tri[0].x + r1 * (tri[1].x - tri[0].x) + r2 * (tri[2].x - tri[0].x);
  float y = tri[0].y + r1 * (tri[1].y - tri[0].y) + r2 * (tri[2].y - tri[0].y);

  return vec2{x, y};
}

void addAnimal(const rl::Shader &shadowShader) {}

float normalizeAngle(float angle) {
  // Normalize angle to [-PI, PI]
  while (angle > PI)
    angle -= 2 * PI;
  while (angle < -PI)
    angle += 2 * PI;
  return angle;
}

float shortestAngleDifference(float from, float to) {
  // Find the shortest angular distance between two angles
  float diff = normalizeAngle(to - from);
  return diff;
}

vec3 make_vec3(float e) { return vec3{e, e, e}; }
