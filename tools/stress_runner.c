#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "core/config.h"
#include "core/game.h"
#include "core/world.h"
#include "systems/rng.h"

static int run_stress_game(uint32_t seed, int max_steps) {
    GameConfig cfg;
    config_init_defaults(&cfg);
    cfg.deterministic_mode = true;
    cfg.rng_seed = seed;
    cfg.max_dungeon_level = 5;
    cfg.dungeon.min_rooms = 4;
    cfg.dungeon.max_rooms = 8;
    cfg.enable_fog = false;

    Game game;
    int rc = game_init(&game, &cfg);
    if (rc != 0) return -1;

    const char *moves[] = {"w","s","a","d","q","e","z","x",".",">","<","g","i"};
    int num_moves = sizeof(moves) / sizeof(moves[0]);

    for (int step = 0; step < max_steps; step++) {
        if (game.state != GS_PLAYING) break;
        if (!game.world.player || !game.world.player->alive) break;

        int move_idx = (seed + step * 7) % num_moves;
        char key = moves[move_idx][0];
        game_handle_input(&game, key);
    }

    int final_depth = game.world.current_depth;
    bool survived = game.state == GS_PLAYING;

    game_destroy(&game);

    if (survived) return final_depth;
    return -final_depth;
}

int main(int argc, char *argv[]) {
    int num_games = 100;
    int max_steps = 500;
    uint32_t base_seed = (uint32_t)time(NULL);

    if (argc > 1) num_games = atoi(argv[1]);
    if (argc > 2) max_steps = atoi(argv[2]);
    if (argc > 3) base_seed = (uint32_t)strtoul(argv[3], NULL, 10);

    printf("=== Stress Runner ===\n");
    printf("Games: %d, Max Steps: %d, Seed: %u\n\n", num_games, max_steps, base_seed);

    int survived = 0;
    int died = 0;
    int max_depth = 0;
    int total_depth = 0;
    int errors = 0;

    for (int i = 0; i < num_games; i++) {
        uint32_t seed = base_seed + i;
        int result = run_stress_game(seed, max_steps);

        if (result > 0) {
            survived++;
            total_depth += result;
            if (result > max_depth) max_depth = result;
            printf("Game %4d: survived, depth %d\n", i, result);
        } else if (result < 0) {
            died++;
            printf("Game %4d: died at depth %d\n", i, -result);
        } else {
            errors++;
            printf("Game %4d: ERROR\n", i);
        }
    }

    printf("\n=== Results ===\n");
    printf("Survived: %d\n", survived);
    printf("Died:     %d\n", died);
    printf("Errors:   %d\n", errors);
    printf("Avg Depth: %.1f\n", survived > 0 ? (float)total_depth / survived : 0.0f);
    printf("Max Depth: %d\n", max_depth);
    printf("Survival Rate: %.1f%%\n", (float)survived / num_games * 100.0f);

    return errors > 0 ? 1 : 0;
}
