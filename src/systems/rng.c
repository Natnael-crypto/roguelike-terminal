#include "systems/rng.h"
#include <string.h>

void rng_init(RNGState *rng, uint64_t seed) {
    rng_seed(rng, seed);
}

void rng_seed(RNGState *rng, uint64_t seed) {
    rng->seed = seed;
    rng->call_count = 0;

    rng->state[0] = 0x0F1B2C3D4E5F6A7BLLU ^ seed;
    rng->state[1] = 0x9A8B7C6D5E4F3A2BLLU ^ (seed << 17 | seed >> 47);
    rng->state[2] = 0x1A2B3C4D5E6F7890LLU ^ (seed * 6364136223846793005LLU);
    rng->state[3] = 0xDEADBEEFCAFEBABELLU ^ (seed >> 13);

    for (int i = 0; i < 4; i++) {
        if (rng->state[i] == 0) rng->state[i] = ~seed;
    }
}

uint64_t rng_next(RNGState *rng) {
    rng->call_count++;
    uint64_t result = rng->state[0] + rng->state[3];
    uint64_t t = rng->state[1] << 17;
    rng->state[2] ^= rng->state[0];
    rng->state[3] ^= rng->state[1];
    rng->state[1] ^= rng->state[2];
    rng->state[0] ^= rng->state[3];
    rng->state[2] ^= t;
    rng->state[3] = (rng->state[3] << 45) | (rng->state[3] >> 19);
    return result;
}

uint32_t rng_next_u32(RNGState *rng) {
    return (uint32_t)(rng_next(rng) & 0xFFFFFFFF);
}

int rng_range(RNGState *rng, int min, int max) {
    if (min >= max) return min;
    uint64_t range = (uint64_t)(max - min);
    uint64_t val = rng_next(rng);
    return min + (int)(val % range);
}

float rng_float(RNGState *rng) {
    uint64_t val = rng_next(rng);
    return (float)(val & 0xFFFFFF) / 16777216.0f;
}

float rng_range_float(RNGState *rng, float min, float max) {
    return min + rng_float(rng) * (max - min);
}

int rng_dice(RNGState *rng, int sides) {
    return rng_range(rng, 1, sides + 1);
}

int rng_roll(RNGState *rng, int count, int sides, int bonus) {
    int total = bonus;
    for (int i = 0; i < count; i++) {
        total += rng_dice(rng, sides);
    }
    return total;
}

bool rng_chance(RNGState *rng, int percent) {
    return rng_range(rng, 0, 100) < percent;
}

int rng_weighted(RNGState *rng, const int *weights, int count) {
    int total = 0;
    for (int i = 0; i < count; i++) total += weights[i];
    if (total <= 0) return 0;
    int roll = rng_range(rng, 0, total);
    int accum = 0;
    for (int i = 0; i < count; i++) {
        accum += weights[i];
        if (roll < accum) return i;
    }
    return count - 1;
}

static void swap(void *a, void *b, size_t size) {
    char tmp[64];
    size_t s = size > 64 ? 64 : size;
    memcpy(tmp, a, s);
    memcpy(a, b, s);
    memcpy(b, tmp, s);
}

void rng_shuffle(RNGState *rng, void *array, int count, size_t size) {
    char *arr = (char *)array;
    for (int i = count - 1; i > 0; i--) {
        int j = rng_range(rng, 0, i + 1);
        swap(arr + i * size, arr + j * size, size);
    }
}

uint64_t rng_get_state(const RNGState *rng) {
    return rng->seed;
}
