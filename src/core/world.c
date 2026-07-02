#include "core/world.h"
#include <string.h>
#include <stdio.h>

int world_init(World *w, GameConfig *cfg) {
    memset(w, 0, sizeof(World));
    w->current_depth = 1;
    w->max_depth = cfg->max_dungeon_level;
    w->seed = cfg->rng_seed != 0 ? cfg->rng_seed : (uint32_t)cfg->dungeon.seed;
    w->game_over = false;
    w->game_won = false;
    w->turn_count = 0;
    w->replay_count = 0;
    w->replay_pos = 0;
    w->replay_mode = false;
    w->recording = true;
    return 0;
}

void world_destroy(World *w) {
    for (int i = 0; i < w->monster_count; i++) {
        if (w->monsters[i]) {
            entity_destroy(w->monsters[i]);
            free(w->monsters[i]);
            w->monsters[i] = NULL;
        }
    }
    w->monster_count = 0;

    for (int i = 0; i < w->item_count; i++) {
        if (w->items[i]) {
            item_destroy(w->items[i]);
            free(w->items[i]);
            w->items[i] = NULL;
        }
    }
    w->item_count = 0;

    if (w->map) {
        free(w->map);
        w->map = NULL;
    }
}

int world_add_monster(World *w, Entity *m) {
    if (w->monster_count >= MAX_MONSTERS) return -1;
    w->monsters[w->monster_count++] = m;
    return 0;
}

int world_add_item(World *w, Item *it) {
    if (w->item_count >= MAX_ITEMS) return -1;
    w->items[w->item_count++] = it;
    return 0;
}

Entity *world_get_entity_at(World *w, int x, int y) {
    if (w->player && w->player->x == x && w->player->y == y && w->player->alive)
        return w->player;
    for (int i = 0; i < w->monster_count; i++) {
        if (w->monsters[i] && w->monsters[i]->alive &&
            w->monsters[i]->x == x && w->monsters[i]->y == y)
            return w->monsters[i];
    }
    return NULL;
}

Item *world_get_item_at(World *w, int x, int y) {
    for (int i = 0; i < w->item_count; i++) {
        if (w->items[i] && w->items[i]->x == x && w->items[i]->y == y)
            return w->items[i];
    }
    return NULL;
}

bool world_is_blocked(World *w, int x, int y) {
    if (!w->map) return true;
    if (!map_in_bounds(w->map, x, y)) return true;
    if (!map_is_walkable(w->map, x, y)) return true;

    if (w->player && w->player->alive && w->player->x == x && w->player->y == y)
        return w->player->blocks_movement;

    for (int i = 0; i < w->monster_count; i++) {
        if (w->monsters[i] && w->monsters[i]->alive &&
            w->monsters[i]->x == x && w->monsters[i]->y == y &&
            w->monsters[i]->blocks_movement)
            return true;
    }
    return false;
}

bool world_move_entity(World *w, Entity *e, int nx, int ny) {
    if (!map_in_bounds(w->map, nx, ny)) return false;
    if (!map_is_walkable(w->map, nx, ny)) return false;

    Entity *other = world_get_entity_at(w, nx, ny);
    if (other && other != e && other->alive && other->blocks_movement)
        return false;

    e->prev_x = e->x;
    e->prev_y = e->y;
    e->x = nx;
    e->y = ny;
    return true;
}

void world_record_action(World *w, const char *action) {
    if (!w->recording) return;
    if (w->replay_count >= MAX_REPLAY_COMMANDS) return;
    strncpy(w->replay_log[w->replay_count], action, 15);
    w->replay_log[w->replay_count][15] = '\0';
    w->replay_count++;
}

int world_next_level(World *w) {
    if (w->current_depth >= w->max_depth) {
        w->game_won = true;
        return -1;
    }
    w->current_depth++;
    return 0;
}

int world_prev_level(World *w) {
    if (w->current_depth <= 1) return -1;
    w->current_depth--;
    return 0;
}
