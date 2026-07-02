#include "core/state.h"
#include "io/serializer.h"
#include <stdio.h>
#include <string.h>

void state_reset_stats(GameStats *stats) {
    memset(stats, 0, sizeof(GameStats));
}

void state_print_stats(const GameStats *stats) {
    printf("=== Game Statistics ===\n");
    printf("Turns:          %d\n", stats->turn_count);
    printf("Monsters Killed: %d\n", stats->monster_kills);
    printf("Items Collected: %d\n", stats->items_collected);
    printf("Levels Explored: %d\n", stats->levels_explored);
    printf("Damage Dealt:   %d\n", stats->damage_dealt);
    printf("Damage Taken:   %d\n", stats->damage_taken);
    printf("Health Healed:  %d\n", stats->health_healed);
    printf("Gold Collected: %d\n", stats->gold_collected);
    printf("Critical Hits:  %d\n", stats->critical_hits);
}

int state_save_to_file(const World *w, const GameStats *stats, const char *path) {
    FILE *f = fopen(path, "wb");
    if (!f) return -1;

    serializer_write_uint32(f, SAVE_MAGIC);
    serializer_write_uint32(f, SAVE_VERSION);
    serializer_write_uint32(f, (uint32_t)w->current_depth);
    serializer_write_uint32(f, (uint32_t)w->max_depth);
    serializer_write_uint32(f, (uint32_t)w->turn_count);
    serializer_write_uint32(f, w->seed);
    serializer_write_uint32(f, w->game_over ? 1 : 0);
    serializer_write_uint32(f, w->game_won ? 1 : 0);
    serializer_write_uint32(f, (uint32_t)w->monster_count);
    serializer_write_uint32(f, (uint32_t)w->item_count);
    serializer_write_uint32(f, (uint32_t)stats->turn_count);
    serializer_write_uint32(f, (uint32_t)stats->monster_kills);
    serializer_write_uint32(f, (uint32_t)stats->items_collected);
    serializer_write_uint32(f, (uint32_t)stats->levels_explored);
    serializer_write_uint32(f, (uint32_t)stats->damage_dealt);
    serializer_write_uint32(f, (uint32_t)stats->damage_taken);
    serializer_write_uint32(f, (uint32_t)stats->health_healed);
    serializer_write_uint32(f, (uint32_t)stats->gold_collected);
    serializer_write_uint32(f, (uint32_t)stats->critical_hits);

    if (w->map) {
        serializer_write_uint32(f, (uint32_t)w->map->width);
        serializer_write_uint32(f, (uint32_t)w->map->height);
        serializer_write_raw(f, w->map->tiles, sizeof(char) * (size_t)w->map->width * (size_t)w->map->height);
    }

    fclose(f);
    return 0;
}

int state_load_from_file(World *w, GameStats *stats, const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return -1;

    uint32_t magic;
    serializer_read_uint32(f, &magic);
    if (magic != SAVE_MAGIC) { fclose(f); return -2; }

    uint32_t version;
    serializer_read_uint32(f, &version);
    if (version != SAVE_VERSION) { fclose(f); return -3; }

    uint32_t tmp;
    serializer_read_uint32(f, &tmp); w->current_depth = (int)tmp;
    serializer_read_uint32(f, &tmp); w->max_depth = (int)tmp;
    serializer_read_uint32(f, &tmp); w->turn_count = (int)tmp;
    serializer_read_uint32(f, &w->seed);
    serializer_read_uint32(f, &tmp); w->game_over = tmp != 0;
    serializer_read_uint32(f, &tmp); w->game_won = tmp != 0;

    int mcount, icount;
    serializer_read_uint32(f, &tmp); mcount = (int)tmp;
    serializer_read_uint32(f, &tmp); icount = (int)tmp;

    serializer_read_uint32(f, &tmp); stats->turn_count = (int)tmp;
    serializer_read_uint32(f, &tmp); stats->monster_kills = (int)tmp;
    serializer_read_uint32(f, &tmp); stats->items_collected = (int)tmp;
    serializer_read_uint32(f, &tmp); stats->levels_explored = (int)tmp;
    serializer_read_uint32(f, &tmp); stats->damage_dealt = (int)tmp;
    serializer_read_uint32(f, &tmp); stats->damage_taken = (int)tmp;
    serializer_read_uint32(f, &tmp); stats->health_healed = (int)tmp;
    serializer_read_uint32(f, &tmp); stats->gold_collected = (int)tmp;
    serializer_read_uint32(f, &tmp); stats->critical_hits = (int)tmp;

    if (w->map) {
        int mw, mh;
        serializer_read_uint32(f, &tmp); mw = (int)tmp;
        serializer_read_uint32(f, &tmp); mh = (int)tmp;
        serializer_read_raw(f, w->map->tiles, sizeof(char) * (size_t)mw * (size_t)mh);
    }

    fclose(f);
    return 0;
}
