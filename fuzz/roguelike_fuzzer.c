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
#include "engine/map.h"
#include "engine/entity.h"
#include "engine/combat.h"
#include "engine/dungeon.h"
#include "engine/ai.h"
#include "systems/rng.h"

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

    int steps = (size - 4) / 3;
    if (steps > 20) steps = 20;

    for (int i = 0; i < steps; i++) {
        if (game.state != GS_PLAYING) break;
        if (!game.world.player || !game.world.player->alive) break;

        size_t pos = 4 + i * 3;
        if (pos + 3 > size) break;

        unsigned char cmd;
        int tx, ty;
        fuzz_parse_input(data + pos, size - pos, (char *)&cmd, &tx, &ty);

        switch (cmd % 12) {
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
