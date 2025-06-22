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
    /* Compute checksum using portable field-by-field approach */
    uint16_t sum = 0;
    sum ^= (uint16_t)(hdr->magic & 0xFFFF);
    sum ^= (uint16_t)(hdr->magic >> 16);
    sum ^= hdr->version;
    sum ^= (uint16_t)hdr->flags;
    sum ^= (uint16_t)(hdr->depth & 0xFFFF);
    sum ^= (uint16_t)(hdr->depth >> 16);
    sum ^= (uint16_t)(hdr->max_depth & 0xFFFF);
    sum ^= (uint16_t)(hdr->turn_count & 0xFFFF);
    sum ^= (uint16_t)(hdr->stats_turns & 0xFFFF);
    sum ^= (uint16_t)(hdr->stats_kills & 0xFFFF);
    sum ^= (uint16_t)(hdr->stats_items & 0xFFFF);
    sum ^= (uint16_t)(hdr->stats_levels & 0xFFFF);
    sum ^= (uint16_t)(hdr->seed & 0xFFFF);
    sum ^= (uint16_t)(hdr->seed >> 16);
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

    /* Write header field-by-field (portable, no alignment issues) */
    serializer_write_uint32(f, hdr.magic);
    serializer_write_uint16(f, hdr.version);
    serializer_write_uint8(f, hdr.flags);
    serializer_write_int32(f, hdr.depth);
    serializer_write_int32(f, hdr.max_depth);
    serializer_write_int32(f, hdr.turn_count);
    serializer_write_int32(f, hdr.stats_turns);
    serializer_write_int32(f, hdr.stats_kills);
    serializer_write_int32(f, hdr.stats_items);
    serializer_write_int32(f, hdr.stats_levels);
    serializer_write_int32(f, hdr.stats_damage_dealt);
    serializer_write_int32(f, hdr.stats_damage_taken);
    serializer_write_int32(f, hdr.stats_healed);
    serializer_write_int32(f, hdr.stats_gold);
    serializer_write_int32(f, hdr.stats_crits);
    serializer_write_int32(f, hdr.monster_count);
    serializer_write_int32(f, hdr.item_count);
    serializer_write_uint32(f, hdr.seed);
    serializer_write_uint16(f, hdr.checksum);

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
    memset(&hdr, 0, sizeof(hdr));
    /* Read header field-by-field (portable, no alignment issues) */
    if (serializer_read_uint32(f, &hdr.magic) != 0) { fclose(f); return -1; }
    serializer_read_uint16(f, &hdr.version);
    serializer_read_uint8(f, &hdr.flags);
    serializer_read_int32(f, &hdr.depth);
    serializer_read_int32(f, &hdr.max_depth);
    serializer_read_int32(f, &hdr.turn_count);
    serializer_read_int32(f, &hdr.stats_turns);
    serializer_read_int32(f, &hdr.stats_kills);
    serializer_read_int32(f, &hdr.stats_items);
    serializer_read_int32(f, &hdr.stats_levels);
    serializer_read_int32(f, &hdr.stats_damage_dealt);
    serializer_read_int32(f, &hdr.stats_damage_taken);
    serializer_read_int32(f, &hdr.stats_healed);
    serializer_read_int32(f, &hdr.stats_gold);
    serializer_read_int32(f, &hdr.stats_crits);
    serializer_read_int32(f, &hdr.monster_count);
    serializer_read_int32(f, &hdr.item_count);
    serializer_read_uint32(f, &hdr.seed);
    serializer_read_uint16(f, &hdr.checksum);

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
        /* Bounds check: prevent reading beyond map tiles array */
        if (mw > 0 && mw <= MAX_MAP_WIDTH && mh > 0 && mh <= MAX_MAP_HEIGHT) {
            serializer_read_raw(f, w->map->tiles, sizeof(char) * (size_t)mw * (size_t)mh);
        }
    }

    fclose(f);
    return 0;
}
