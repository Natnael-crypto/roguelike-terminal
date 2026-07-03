#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "core/config.h"
#include "core/game.h"
#include "core/world.h"
#include "core/state.h"
#include "engine/map.h"
#include "engine/entity.h"
#include "engine/combat.h"
#include "engine/dungeon.h"
#include "engine/ai.h"
#include "engine/inventory.h"
#include "engine/item.h"
#include "systems/rng.h"
#include "systems/event.h"

#ifdef __cplusplus
}
#endif

static int fuzz_parse_input(const uint8_t *data, size_t size, char *cmd, int *x, int *y) {
    if (size < 3) return -1;
    *cmd = (char)data[0];
    *x = data[1] % 80;
    *y = data[2] % 25;
    return 0;
}

#ifdef __cplusplus
extern "C" {
#endif

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < 4) return 0;

    GameConfig cfg;
    config_init_defaults(&cfg);
    cfg.deterministic_mode = true;
    cfg.rng_seed = 42;
    cfg.max_dungeon_level = 3;
    cfg.dungeon.min_rooms = 3;
    cfg.dungeon.max_rooms = 6;
    cfg.dungeon.monster_density = 2;
    cfg.dungeon.item_density = 1;

    Game game;
    int rc = game_init(&game, &cfg);
    if (rc != 0) return 0;

    /* --- Bug-specific trigger sequences (deterministic) --- */

    /* Bug 7: Map negative index — teleport player to left edge, then try to go further left */
    if (size >= 5 && (data[4] % 8) == 0) {
        if (game.world.player) {
            game.world.player->x = 0;
            game.world.player->y = 10;
        }
        game_handle_input(&game, 'a');  /* triggers tiles[-1][ny] read */
    }

    /* Bug 6: Event double-free — push heap events, teleport to stairs, descend */
    if (size >= 5 && (data[4] % 8) == 1) {
        /* Push several heap-allocated events via game_tick */
        for (int i = 0; i < 5; i++)
            game_tick(&game);
        /* Teleport player directly to stairs */
        if (game.world.map && game.world.player) {
            game.world.player->x = game.world.map->stairs_down_x;
            game.world.player->y = game.world.map->stairs_down_y;
        }
        /* Descend — triggers event_queue_clear on already-freed slots */
        game_handle_input(&game, '>');
    }

    /* Bug 3: Inventory desync — create items directly, equip, then remove */
    if (size >= 5 && (data[4] % 8) == 2) {
        Entity *p = game.world.player;
        if (p) {
            /* Create two items directly in inventory */
            Item *sword = item_create("iron sword", '/', ITEM_WEAPON);
            Item *potion = item_create("health potion", '!', ITEM_POTION);
            if (sword) { sword->equipped = false; inventory_add(p, sword); }
            if (potion) { potion->equipped = false; inventory_add(p, potion); }
            /* Equip the sword at slot 0 */
            if (p->inventory_count > 0)
                inventory_equip(p, 0);
            /* Use the potion at slot 1 — calls inventory_remove(1) which shifts array */
            /* After shift, equipment[] still points to old slot index, now wrong item */
            if (p->inventory_count > 1)
                inventory_use(p, 1);
        }
    }

    /* Bug 5: Serializer alignment — save then load raw packed struct */
    if (size >= 5 && (data[4] % 8) == 3) {
        GameStats stats;
        state_reset_stats(&stats);
        state_save_to_file(&game.world, &stats, "/tmp/fuzz_test_save.dat");
        state_load_from_file(&game.world, &stats, "/tmp/fuzz_test_save.dat");
    }

    /* --- General random input fuzzing --- */
    int steps = (size - 4) / 3;
    if (steps > 50) steps = 50;

    for (int i = 0; i < steps; i++) {
        if (game.state != GS_PLAYING) break;
        if (!game.world.player || !game.world.player->alive) break;

        size_t pos = 4 + i * 3;
        if (pos + 3 > size) break;

        unsigned char cmd;
        int tx, ty;
        fuzz_parse_input(data + pos, size - pos, (char *)&cmd, &tx, &ty);

        switch (cmd % 16) {
            case 0: game_handle_input(&game, 'w'); break;
            case 1: game_handle_input(&game, 's'); break;
            case 2: game_handle_input(&game, 'a'); break;
            case 3: game_handle_input(&game, 'd'); break;
            case 4: game_handle_input(&game, '.'); break;
            case 5: game_handle_input(&game, 'g'); break;
            case 6: game_handle_input(&game, '>'); break;
            case 7: game_handle_input(&game, '<'); break;
            case 8: game_tick(&game); break;
            case 9: game_tick(&game); break;
            case 10: game_tick(&game); break;
            case 11: game_handle_input(&game, 'i'); break;
            case 12: if (game.world.player && game.world.player->inventory_count > 0) inventory_equip(game.world.player, 0); break;
            case 13: if (game.world.player) inventory_unequip(game.world.player, SLOT_WEAPON); break;
            case 14: if (game.world.player && game.world.player->inventory_count > 0) inventory_use(game.world.player, 0); break;
            case 15: game_tick(&game); break;
        }

        if (game.world.map && game.world.player) {
            map_compute_fov(game.world.map,
                game.world.player->x,
                game.world.player->y, 8);
        }
    }

    game_destroy(&game);
    return 0;
}

#ifdef __cplusplus
}
#endif
