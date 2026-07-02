#ifndef STATE_H
#define STATE_H

#include "core/config.h"
#include "core/world.h"

typedef struct {
    int turn_count;
    int monster_kills;
    int items_collected;
    int levels_explored;
    int damage_dealt;
    int damage_taken;
    int health_healed;
    int gold_collected;
    int critical_hits;
    int fov_recalculations;
} GameStats;

int state_save_to_file(const World *w, const GameStats *stats, const char *path);
int state_load_from_file(World *w, GameStats *stats, const char *path);
void state_reset_stats(GameStats *stats);
void state_print_stats(const GameStats *stats);

#endif
