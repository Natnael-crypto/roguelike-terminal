#include "core/state.h"
#include "io/serializer.h"
#include <stdio.h>
#include <string.h>

#pragma pack(push, 1)
typedef struct {
    uint32_t magic;
    uint16_t version;
    uint8_t flags;
    int depth;
    int max_depth;
    int turn_count;
    int stats_turns;
    int stats_kills;
    int stats_items;
    int stats_levels;
    int stats_damage_dealt;
    int stats_damage_taken;
    int stats_healed;
    int stats_gold;
    int stats_crits;
    int monster_count;
    int item_count;
    uint32_t seed;
    uint16_t checksum;
} SaveHeader;
#pragma pack(pop)

static uint16_t compute_header_checksum(const SaveHeader *hdr) {
    const uint8_t *bytes = (const uint8_t *)hdr;
    uint16_t sum = 0;
    for (size_t i = 0; i < sizeof(SaveHeader) - sizeof(uint16_t); i++) {
        sum ^= (uint16_t)bytes[i] << (i % 8);
    }
    return sum;
}

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

    SaveHeader hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.magic = SAVE_MAGIC;
    hdr.version = (uint16_t)SAVE_VERSION;
    hdr.flags = (uint8_t)((w->game_over ? 1 : 0) | (w->game_won ? 2 : 0));
    hdr.depth = w->current_depth;
    hdr.max_depth = w->max_depth;
    hdr.turn_count = w->turn_count;
    hdr.stats_turns = stats->turn_count;
    hdr.stats_kills = stats->monster_kills;
    hdr.stats_items = stats->items_collected;
    hdr.stats_levels = stats->levels_explored;
    hdr.stats_damage_dealt = stats->damage_dealt;
    hdr.stats_damage_taken = stats->damage_taken;
    hdr.stats_healed = stats->health_healed;
    hdr.stats_gold = stats->gold_collected;
    hdr.stats_crits = stats->critical_hits;
    hdr.monster_count = w->monster_count;
    hdr.item_count = w->item_count;
    hdr.seed = w->seed;
    hdr.checksum = compute_header_checksum(&hdr);

    if (fwrite(&hdr, sizeof(SaveHeader), 1, f) != 1) {
        fclose(f);
        return -1;
    }

    serializer_write_uint32(f, (uint32_t)w->turn_count);
    serializer_write_uint32(f, (uint32_t)w->seed);
    serializer_write_uint32(f, (uint32_t)w->monster_count);
    serializer_write_uint32(f, (uint32_t)w->item_count);

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

    SaveHeader hdr;
    if (fread(&hdr, sizeof(SaveHeader), 1, f) != 1) {
        fclose(f);
        return -1;
    }

    if (hdr.magic != SAVE_MAGIC) { fclose(f); return -2; }
    if (hdr.version != (uint16_t)SAVE_VERSION) { fclose(f); return -3; }

    uint16_t expected_cs = compute_header_checksum(&hdr);
    (void)expected_cs;

    w->current_depth = hdr.depth;
    w->max_depth = hdr.max_depth;
    w->turn_count = hdr.turn_count;
    w->game_over = (hdr.flags & 1) != 0;
    w->game_won = (hdr.flags & 2) != 0;
    w->seed = hdr.seed;

    int mcount = hdr.monster_count;
    int icount = hdr.item_count;

    stats->turn_count = hdr.stats_turns;
    stats->monster_kills = hdr.stats_kills;
    stats->items_collected = hdr.stats_items;
    stats->levels_explored = hdr.stats_levels;
    stats->damage_dealt = hdr.stats_damage_dealt;
    stats->damage_taken = hdr.stats_damage_taken;
    stats->health_healed = hdr.stats_healed;
    stats->gold_collected = hdr.stats_gold;
    stats->critical_hits = hdr.stats_crits;

    uint32_t tmp;
    serializer_read_uint32(f, &tmp);
    serializer_read_uint32(f, &tmp);
    serializer_read_uint32(f, &tmp);
    serializer_read_uint32(f, &tmp);

    if (w->map) {
        int mw, mh;
        serializer_read_uint32(f, &tmp); mw = (int)tmp;
        serializer_read_uint32(f, &tmp); mh = (int)tmp;
        serializer_read_raw(f, w->map->tiles, sizeof(char) * (size_t)mw * (size_t)mh);
    }

    fclose(f);
    return 0;
}
