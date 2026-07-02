#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "core/config.h"
#include "core/game.h"
#include "core/world.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <replay_file> [max_steps]\n", argv[0]);
        return 1;
    }

    const char *replay_path = argv[1];
    int max_steps = argc > 2 ? atoi(argv[2]) : 1000;

    FILE *f = fopen(replay_path, "r");
    if (!f) {
        fprintf(stderr, "Cannot open replay file: %s\n", replay_path);
        return 1;
    }

    char actions[65536][16];
    int action_count = 0;

    while (fgets(actions[action_count], 16, f) && action_count < 65536) {
        size_t len = strlen(actions[action_count]);
        if (len > 0 && actions[action_count][len-1] == '\n')
            actions[action_count][len-1] = '\0';
        action_count++;
    }
    fclose(f);

    if (action_count == 0) {
        fprintf(stderr, "Empty replay file\n");
        return 1;
    }

    printf("=== Replay Analyzer ===\n");
    printf("File: %s\n", replay_path);
    printf("Actions: %d\n", action_count);
    printf("Max steps: %d\n\n", max_steps);

    GameConfig cfg;
    config_init_defaults(&cfg);
    cfg.deterministic_mode = true;
    cfg.rng_seed = 42;
    cfg.max_dungeon_level = 10;
    cfg.enable_fog = false;

    Game game;
    int rc = game_init(&game, &cfg);
    if (rc != 0) {
        fprintf(stderr, "Failed to init game\n");
        return 1;
    }

    int played = 0;
    for (int i = 0; i < action_count && i < max_steps; i++) {
        if (game.state != GS_PLAYING) {
            printf("Game ended at step %d (state=%d)\n", i, game.state);
            break;
        }

        const char *action = actions[i];
        char key = action[0];

        if (key >= 'a' && key <= 'z' || key == '.' || key == '>' || key == '<' || key == 'g') {
            game_handle_input(&game, key);
            played++;
        }
    }

    printf("\n=== Results ===\n");
    printf("Actions played: %d\n", played);
    printf("Final state: %d\n", game.state);
    printf("Depth reached: %d\n", game.world.current_depth);
    printf("Turns: %d\n", game.world.turn_count);
    printf("Player alive: %s\n",
        game.world.player && game.world.player->alive ? "yes" : "no");

    if (game.world.player) {
        printf("Player HP: %d/%d\n", game.world.player->health, game.world.player->max_health);
        printf("Player level: %d\n", game.world.player->level);
        printf("Player gold: %d\n", game.world.player->gold);
    }

    game_destroy(&game);
    return 0;
}
