#include "utils.h"
#include "buildings.h"



GameState::GameState() : toggleFence(false), itemActive(0), fence(new Fence), pens() {
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
