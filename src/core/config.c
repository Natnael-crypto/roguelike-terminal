#include "core/config.h"
#include <stdio.h>
#include <string.h>

void config_init_defaults(GameConfig *cfg) {
    memset(cfg, 0, sizeof(GameConfig));
    cfg->player_count = 1;
    cfg->start_health = 30;
    cfg->start_mana = 10;
    cfg->max_dungeon_level = 10;
    cfg->fov_radius = 8;
    cfg->tick_speed_ms = 50;
    cfg->debug_mode = false;
    cfg->god_mode = false;
    cfg->show_fov = true;
    cfg->enable_fog = true;
    cfg->deterministic_mode = false;
    cfg->rng_seed = 0;
    strcpy(cfg->save_file, "savegame.rgl");
    strcpy(cfg->replay_file, "replay.rgl");

    cfg->dungeon.room_count = 0;
    cfg->dungeon.min_rooms = 6;
    cfg->dungeon.max_rooms = 12;
    cfg->dungeon.min_room_size = 4;
    cfg->dungeon.max_room_size = 10;
    cfg->dungeon.monster_density = 3;
    cfg->dungeon.item_density = 2;
    cfg->dungeon.depth = 1;
    cfg->dungeon.seed = 0;
    cfg->dungeon.has_stairs = true;
    cfg->dungeon.dark_sight = false;
    cfg->dungeon.enable_traps = false;
}

void config_print(const GameConfig *cfg) {
    printf("Game Configuration:\n");
    printf("  Players: %d\n", cfg->player_count);
    printf("  Start HP: %d\n", cfg->start_health);
    printf("  Start MP: %d\n", cfg->start_mana);
    printf("  Max Depth: %d\n", cfg->max_dungeon_level);
    printf("  FOV Radius: %d\n", cfg->fov_radius);
    printf("  Tick Speed: %dms\n", cfg->tick_speed_ms);
    printf("  Debug: %s\n", cfg->debug_mode ? "yes" : "no");
    printf("  God Mode: %s\n", cfg->god_mode ? "yes" : "no");
    printf("  Deterministic: %s\n", cfg->deterministic_mode ? "yes" : "no");
    printf("  Seed: %u\n", cfg->rng_seed);
    printf("  Save: %s\n", cfg->save_file);
    printf("  Replay: %s\n", cfg->replay_file);
    printf("Dungeon Config:\n");
    printf("  Rooms: %d-%d\n", cfg->dungeon.min_rooms, cfg->dungeon.max_rooms);
    printf("  Room Size: %d-%d\n", cfg->dungeon.min_room_size, cfg->dungeon.max_room_size);
    printf("  Monster Density: %d\n", cfg->dungeon.monster_density);
    printf("  Item Density: %d\n", cfg->dungeon.item_density);
}
