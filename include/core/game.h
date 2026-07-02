#ifndef GAME_H
#define GAME_H

#include "core/config.h"
#include "core/world.h"
#include "systems/rng.h"
#include "systems/event.h"

typedef enum {
    GS_MENU,
    GS_PLAYING,
    GS_DEAD,
    GS_WON,
    GS_QUIT
} GameState;

typedef struct {
    GameConfig config;
    World world;
    RNGState rng;
    EventQueue events;
    GameState state;
    int tick_count;
    bool needs_render;
    char message_log[MAX_LOG_LINES][128];
    int message_count;
    int message_scroll;
} Game;

int game_init(Game *game, GameConfig *cfg);
void game_destroy(Game *game);
int game_tick(Game *game);
void game_handle_input(Game *game, int key);
void game_add_message(Game *game, const char *fmt, ...);
const char *game_get_message(const Game *game, int index);
void game_update_fov(Game *game);

#endif
