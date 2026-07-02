#include "systems/scheduler.h"
#include <string.h>

void scheduler_init(Scheduler *s) {
    s->count = 0;
    s->current_index = -1;
    s->tick = 0;
    s->round_robin = true;
    memset(s->entries, 0, sizeof(s->entries));
}

int scheduler_add(Scheduler *s, uint32_t entity_id, int speed) {
    if (s->count >= MAX_SCHEDULED_ENTITIES) return -1;
    s->entries[s->count].entity_id = entity_id;
    s->entries[s->count].energy = 0;
    s->entries[s->count].speed = speed > 0 ? speed : 100;
    s->entries[s->count].tick_offset = s->count;
    s->count++;
    return 0;
}

int scheduler_remove(Scheduler *s, uint32_t entity_id) {
    for (int i = 0; i < s->count; i++) {
        if (s->entries[i].entity_id == entity_id) {
            for (int j = i; j < s->count - 1; j++) {
                s->entries[j] = s->entries[j + 1];
            }
            s->count--;
            if (s->current_index >= i) s->current_index--;
            return 0;
        }
    }
    return -1;
}

int scheduler_next(Scheduler *s) {
    if (s->count <= 0) return -1;

    if (s->current_index < 0 || s->current_index >= s->count) {
        s->current_index = 0;
    }

    if (s->round_robin) {
        s->current_index = (s->current_index + 1) % s->count;
        s->tick++;
        return s->entries[s->current_index].entity_id;
    }

    int best_idx = -1;
    int best_energy = -1;

    for (int i = 0; i < s->count; i++) {
        s->entries[i].energy += s->entries[i].speed;

        if (s->entries[i].energy > best_energy) {
            best_energy = s->entries[i].energy;
            best_idx = i;
        }
    }

    if (best_idx >= 0) {
        s->entries[best_idx].energy -= 1000;
        s->current_index = best_idx;
        s->tick++;
        return s->entries[best_idx].entity_id;
    }

    return -1;
}

void scheduler_reset(Scheduler *s) {
    for (int i = 0; i < s->count; i++) {
        s->entries[i].energy = 0;
    }
    s->current_index = -1;
    s->tick = 0;
}

void scheduler_set_speed(Scheduler *s, uint32_t entity_id, int speed) {
    for (int i = 0; i < s->count; i++) {
        if (s->entries[i].entity_id == entity_id) {
            s->entries[i].speed = speed > 0 ? speed : 100;
            return;
        }
    }
}

uint32_t scheduler_get_current(const Scheduler *s) {
    if (s->current_index < 0 || s->current_index >= s->count) return 0;
    return s->entries[s->current_index].entity_id;
}

bool scheduler_has_entity(const Scheduler *s, uint32_t entity_id) {
    for (int i = 0; i < s->count; i++) {
        if (s->entries[i].entity_id == entity_id) return true;
    }
    return false;
}

void scheduler_clear(Scheduler *s) {
    s->count = 0;
    s->current_index = -1;
    s->tick = 0;
}
