#ifndef WORLD_H
#define WORLD_H

#include "core/config.h"
#include "engine/entity.h"
#include "engine/map.h"
#include "engine/item.h"
#include "systems/rng.h"

typedef struct {
    Entity *player;
    Entity *monsters[MAX_MONSTERS];
    int monster_count;
    Item *items[MAX_ITEMS];
    int item_count;
    Map *map;
    int current_depth;
    int max_depth;
    bool game_over;
    bool game_won;
    int turn_count;
    uint32_t seed;
    RNGState rng;
    char replay_log[MAX_REPLAY_COMMANDS][16];
    int replay_count;
    int replay_pos;
    bool replay_mode;
    bool recording;
} World;

int world_init(World *w, GameConfig *cfg);
void world_destroy(World *w);
int world_add_monster(World *w, Entity *m);
int world_add_item(World *w, Item *it);
Entity *world_get_entity_at(World *w, int x, int y);
Item *world_get_item_at(World *w, int x, int y);
bool world_is_blocked(World *w, int x, int y);
bool world_move_entity(World *w, Entity *e, int nx, int ny);
void world_record_action(World *w, const char *action);
int world_next_level(World *w);
int world_prev_level(World *w);

#endif
