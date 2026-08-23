#include "avoidance.h"
#include <math.h>

const nd_vec3_t ND_SENSOR_DIRECTIONS[ND_NUM_SENSORS] = {
    [ND_SENSOR_FRONT] = { 1.0f,  0.0f,  0.0f},
    [ND_SENSOR_BACK]  = {-1.0f,  0.0f,  0.0f},
    [ND_SENSOR_UP]    = { 0.0f,  0.0f,  1.0f},
    [ND_SENSOR_LEFT]  = { 0.0f,  1.0f,  0.0f},
    [ND_SENSOR_RIGHT] = { 0.0f, -1.0f,  0.0f},
};

static nd_vec3_t vec3_add(nd_vec3_t a, nd_vec3_t b) {
    nd_vec3_t r = {a.x + b.x, a.y + b.y, a.z + b.z};
    return r;
}

static nd_vec3_t vec3_scale(nd_vec3_t v, float s) {
    nd_vec3_t r = {v.x * s, v.y * s, v.z * s};
    return r;
}

static float vec3_norm(nd_vec3_t v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

nd_vec3_t nd_repulsion_vector(const float distances_m[ND_NUM_SENSORS],
                               float safety_margin_m, float gain) {
    nd_vec3_t total = {0.0f, 0.0f, 0.0f};

    for (int i = 0; i < ND_NUM_SENSORS; i++) {
        float d = distances_m[i];
        if (d >= safety_margin_m || d <= 1e-6f) {
            continue;
        }
        float magnitude = gain * (1.0f / d - 1.0f / safety_margin_m);
        nd_vec3_t repulsion = vec3_scale(ND_SENSOR_DIRECTIONS[i], -magnitude);
        total = vec3_add(total, repulsion);
    }
    return total;
}

nd_vec3_t nd_command_velocity(nd_vec3_t desired_velocity,
                               const float distances_m[ND_NUM_SENSORS],
                               float safety_margin_m, float gain,
                               float max_speed) {
    nd_vec3_t repulsion = nd_repulsion_vector(distances_m, safety_margin_m, gain);
    nd_vec3_t combined = vec3_add(desired_velocity, repulsion);

    float speed = vec3_norm(combined);
    if (speed > max_speed && speed > 1e-9f) {
        combined = vec3_scale(combined, max_speed / speed);
    }
    return combined;
}
