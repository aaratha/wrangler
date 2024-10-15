#include "utils.h"

float lerp_to(float position, float target, float rate) {
    return position + (target - position) * rate;  // Lerp between position and target
}

vec3 lerp3D(vec3 position, vec3 target, float rate) {
    return position + (target - position) * rate;  // Lerp between vec3 position and target
}

float GetRandomFloat(float min, float max) {
    return min + (max - min) * ((float)GetRandomValue(0, RAND_MAX) / (float)RAND_MAX);
}
