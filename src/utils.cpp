#include "utils.hpp"
#include "terrain.hpp"
#include "render_utils.hpp"
#include "buildings.hpp"
#include "player.hpp"
#include "animal.hpp"



GameState::GameState(const rl::Shader& shadowShader) :
    toggleFence(false),
    itemActive(0),
    camera(RenderUtils::SetupCamera()),
    lightCam(RenderUtils::SetupLightCamera()),
    terrain(std::make_unique<Terrain>(shadowShader)),
    player(std::make_unique<Player>(vec3{0.0,1.0,0.0}, 0.2, shadowShader)),
    animals(CreateAnimals(shadowShader, 10)),
    fence(std::make_unique<Fence>()),
    pens() {
    // The unique_ptrs will automatically handle memory management
}


float lerp_to(float position, float target, float rate) {
    return position + (target - position) * rate;  // Lerp between position and target
}

vec3 lerp3D(vec3 position, vec3 target, float rate) {
    return position + (target - position) * rate;  // Lerp between vec3 position and target
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

vec3 vec2to3(vec2 vec2, float y) {
    return vec3{vec2.x, y, vec2.y};
}


vec2 project_mouse(float y, Camera3D camera) {
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
            return intersection;
        }
    }
}

void update_lightDir(vec3& lightDir, float dt) {
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

void update_itemActive(int& itemActive) {
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
