#include "roguelike.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void print_usage(const char *prog) {
    printf("Usage: %s [options]\n", prog);
    printf("Options:\n");
    printf("  -h, --help          Show this help\n");
    printf("  -s, --seed SEED     Set RNG seed\n");
    printf("  -d, --depth N       Set max dungeon depth (1-50)\n");
    printf("  -f, --fov N         Set FOV radius (1-20)\n");
    printf("  --debug             Enable debug mode\n");
    printf("  --god-mode          Enable god mode (invulnerable)\n");
    printf("  --no-fog            Disable fog of war\n");
    printf("  --deterministic     Enable deterministic mode\n");
    printf("  --save FILE         Save game file path\n");
    printf("  --load FILE         Load game from file\n");
    printf("  --replay FILE       Record/play replay file\n");
    printf("  --dump-config       Print config and exit\n");
}

static void parse_args(int argc, char *argv[], GameConfig *cfg) {
    config_init_defaults(cfg);

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            exit(0);
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--seed") == 0) {
            if (i + 1 < argc) cfg->rng_seed = (uint32_t)strtoul(argv[++i], NULL, 10);
        } else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--depth") == 0) {
            if (i + 1 < argc) {
                int d = atoi(argv[++i]);
                if (d > 0 && d <= 50) cfg->max_dungeon_level = d;
            }
        } else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--fov") == 0) {
            if (i + 1 < argc) {
                int f = atoi(argv[++i]);
                if (f > 0 && f <= 20) cfg->fov_radius = f;
            }
        } else if (strcmp(argv[i], "--debug") == 0) {
            cfg->debug_mode = true;
        } else if (strcmp(argv[i], "--god-mode") == 0) {
            cfg->god_mode = true;
        } else if (strcmp(argv[i], "--no-fog") == 0) {
            cfg->enable_fog = false;
        } else if (strcmp(argv[i], "--deterministic") == 0) {
            cfg->deterministic_mode = true;
        } else if (strcmp(argv[i], "--save") == 0) {
            if (i + 1 < argc) strncpy(cfg->save_file, argv[++i], MAX_NAME_LEN - 1);
        } else if (strcmp(argv[i], "--load") == 0) {
            if (i + 1 < argc) strncpy(cfg->save_file, argv[++i], MAX_NAME_LEN - 1);
        } else if (strcmp(argv[i], "--replay") == 0) {
            if (i + 1 < argc) strncpy(cfg->replay_file, argv[++i], MAX_NAME_LEN - 1);
        } else if (strcmp(argv[i], "--dump-config") == 0) {
            config_print(cfg);
            exit(0);
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            exit(1);
        }
    }
}

int main(int argc, char *argv[]) {
    GameConfig config;
    Game game;
    CliContext cli;
    int rc;

    parse_args(argc, argv, &config);

    if (config.rng_seed == 0) {
        config.rng_seed = (uint32_t)time(NULL);
    }

    rc = game_init(&game, &config);
    if (rc != 0) {
        fprintf(stderr, "Failed to initialize game: %d\n", rc);
        return 1;
    }

    rc = cli_init(&cli, &game);
    if (rc != 0) {
        fprintf(stderr, "Failed to initialize CLI: %d\n", rc);
        game_destroy(&game);
        return 1;
    }

    cli_run(&cli);

    cli_destroy(&cli);
    game_destroy(&game);

    printf("\nThanks for playing!\n");
    return 0;
}
