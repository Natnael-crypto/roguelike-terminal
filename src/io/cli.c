#include "io/cli.h"
#include "io/renderer.h"
#include "io/input.h"
#include "core/game.h"
#include "engine/inventory.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

int cli_init(CliContext *ctx, Game *game) {
    memset(ctx, 0, sizeof(CliContext));
    ctx->game = game;
    ctx->running = true;
    ctx->paused = false;
    ctx->verbose = VS_NORMAL;
    ctx->screen_width = 80;
    ctx->screen_height = 25;
    ctx->cursor_x = 0;
    ctx->cursor_y = 0;
    snprintf(ctx->status_line, sizeof(ctx->status_line), "Roguelike Terminal RPG v1.0");

    input_set_raw(1);
    game->needs_render = true;
    return 0;
}

void cli_destroy(CliContext *ctx) {
    (void)ctx;
    input_set_raw(0);
    printf("\033[?25h\033[2J\033[H");
    fflush(stdout);
}

#ifdef _WIN32
static void gotoxy(int x, int y) {
    COORD c = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}
#else
static void gotoxy(int x, int y) {
    printf("\033[%d;%dH", y + 1, x + 1);
}
#endif

void cli_draw_status(CliContext *ctx) {
    Game *g = ctx->game;
    if (!g || !g->world.player) return;

    char status[256];
    Entity *p = g->world.player;

    int hp_pct = p->max_health > 0 ? (p->health * 100) / p->max_health : 0;

    snprintf(status, sizeof(status),
        "HP:%3d/%3d(%d%%) MP:%3d/%3d LV:%d EXP:%d/%d D:%d G:%d",
        p->health, p->max_health, hp_pct,
        p->mana, p->max_mana,
        p->level, p->experience, p->next_level_exp,
        g->world.current_depth, p->gold);

    gotoxy(0, 0);
    printf("\033[1;37m%s\033[0m", status);
    for (int i = strlen(status); i < ctx->screen_width; i++) putchar(' ');
}

void cli_draw_messages(CliContext *ctx) {
    Game *g = ctx->game;
    if (!g) return;

    gotoxy(0, 22);
    int start = g->message_count - 3;
    if (start < 0) start = 0;

    for (int i = 0; i < 3; i++) {
        int idx = start + i;
        if (idx < g->message_count) {
            printf("\033[2K%s\n", g->message_log[idx]);
        } else {
            printf("\033[2K\n");
        }
    }
    fflush(stdout);
}

void cli_draw_minimap(CliContext *ctx) {
    (void)ctx;
}

void cli_render(CliContext *ctx) {
    Game *g = ctx->game;
    if (!g || !g->needs_render) return;
    g->needs_render = false;

    printf("\033[2J\033[H");

    cli_draw_status(ctx);

    if (g->world.map && g->world.player) {
        RenderBuffer rb;
        render_init(&rb, 60, 20);
        render_map(&rb, g->world.map, g->world.player->x, g->world.player->y, g->config.fov_radius);
        render_entities(&rb, g->world.player,
            (const Entity *const *)g->world.monsters,
            g->world.monster_count, g->world.map);

        for (int y = 0; y < 20 && y < rb.height; y++) {
            gotoxy(0, y + 1);
            for (int x = 0; x < 60 && x < rb.width; x++) {
                char c = rb.chars[y][x];
                if (c == '@') printf("\033[1;33m%c\033[0m", c);
                else if (c == '#') printf("\033[1;30m%c\033[0m", c);
                else if (c == '+' || c == '>') printf("\033[1;33m%c\033[0m", c);
                else if (c == 'r' || c == 'g') printf("\033[1;31m%c\033[0m", c);
                else if (c == '!' || c == '?') printf("\033[1;32m%c\033[0m", c);
                else if (c == '$') printf("\033[1;33m%c\033[0m", c);
                else putchar(c);
            }
        }
    }

    cli_draw_messages(ctx);
}

void cli_handle_key(CliContext *ctx, int key) {
    if (!ctx->game) return;

    if (key == KEY_ESCAPE) {
        ctx->running = false;
        return;
    }

    game_handle_input(ctx->game, key);
}

void cli_show_inventory(CliContext *ctx) {
    Game *g = ctx->game;
    if (!g || !g->world.player) return;

    printf("\033[2J\033[H");
    printf("=== Inventory ===\n");
    printf("Capacity: %d/%d\n\n", g->world.player->inventory_count, g->world.player->inventory_capacity);

    if (g->world.player->inventory_count == 0) {
        printf("(empty)\n");
    } else {
        for (int i = 0; i < g->world.player->inventory_count; i++) {
            Item *it = inventory_get(g->world.player, i);
            const char *iname = it ? it->name : "unknown";
            printf("%c) %s\n", 'a' + i, iname);
        }
    }

    printf("\nPress any key to continue...\n");
    fflush(stdout);
    input_get_key();
    g->needs_render = true;
}

void cli_show_equipment(CliContext *ctx) {
    (void)ctx;
}

void cli_show_character_sheet(CliContext *ctx) {
    Game *g = ctx->game;
    if (!g || !g->world.player) return;

    printf("\033[2J\033[H");
    Entity *p = g->world.player;
    printf("=== Character Sheet ===\n\n");
    printf("Name: Player\n");
    printf("Level: %d\n", p->level);
    printf("Experience: %d / %d\n", p->experience, p->next_level_exp);
    printf("Health: %d / %d\n", p->health, p->max_health);
    printf("Mana: %d / %d\n", p->mana, p->max_mana);
    printf("Strength: %d\n", p->strength);
    printf("Dexterity: %d\n", p->dexterity);
    printf("Intelligence: %d\n", p->intelligence);
    printf("Attack: %d\n", p->attack);
    printf("Defense: %d\n", p->defense);
    printf("Armor: %d\n", p->armor);
    printf("Speed: %d\n", p->speed);
    printf("Luck: %d\n", p->luck);
    printf("Gold: %d\n", p->gold);
    printf("Depth: %d\n", g->world.current_depth);

    printf("\nPress any key to continue...\n");
    fflush(stdout);
    input_get_key();
    g->needs_render = true;
}

void cli_show_help(CliContext *ctx) {
    printf("\033[2J\033[H");
    printf("=== Roguelike Terminal RPG - Help ===\n\n");
    printf("Movement:\n");
    printf("  w/W         Up\n");
    printf("  s/S         Down\n");
    printf("  a/A         Left\n");
    printf("  d/D         Right\n");
    printf("  q           Northwest\n");
    printf("  e           Northeast\n");
    printf("  z           Southwest\n");
    printf("  x           Southeast\n");
    printf("  .           Wait (skip turn)\n\n");
    printf("Actions:\n");
    printf("  g/G         Pick up item\n");
    printf("  i/I         Open inventory\n");
    printf("  u           Use item\n");
    printf("  f/F         Drop item\n");
    printf("  >           Descend stairs\n");
    printf("  <           Ascend stairs\n");
    printf("  c/C         Character sheet\n");
    printf("  ?           This help\n");
    printf("  Q           Quit game\n\n");
    printf("Press any key to continue...\n");
    fflush(stdout);
    input_get_key();
    if (ctx->game) ctx->game->needs_render = true;
}

void cli_show_message_log(CliContext *ctx) {
    (void)ctx;
}

void cli_show_debug(CliContext *ctx) {
    Game *g = ctx->game;
    if (!g) return;

    printf("\033[2J\033[H");
    printf("=== Debug Info ===\n\n");
    printf("Ticks: %d\n", g->tick_count);
    printf("Turn: %d\n", g->world.turn_count);
    printf("Depth: %d\n", g->world.current_depth);
    printf("Monsters: %d\n", g->world.monster_count);
    printf("Items: %d\n", g->world.item_count);
    printf("Player: (%d,%d)\n",
        g->world.player ? g->world.player->x : -1,
        g->world.player ? g->world.player->y : -1);
    printf("Seed: %llu\n", (unsigned long long)g->rng.seed);
    printf("FOV: %s\n", g->config.show_fov ? "on" : "off");
    printf("God Mode: %s\n", g->config.god_mode ? "on" : "off");
    printf("RNG Calls: %llu\n", (unsigned long long)g->rng.call_count);

    printf("\nPress any key to continue...\n");
    fflush(stdout);
    input_get_key();
    g->needs_render = true;
}

void cli_run(CliContext *ctx) {
    if (!ctx->game) return;

    cli_render(ctx);

    while (ctx->running && ctx->game->state == GS_PLAYING) {
        int key = input_get_key_timeout(50);

        if (key == KEY_NONE) {
            if (ctx->game->needs_render) cli_render(ctx);
            continue;
        }

        if (key == '!') {
            cli_show_debug(ctx);
            continue;
        }
        if (key == '?' || key == 'H') {
            cli_show_help(ctx);
            ctx->game->needs_render = true;
            continue;
        }
        if (key == 'i' || key == 'I') {
            cli_show_inventory(ctx);
            ctx->game->needs_render = true;
            continue;
        }
        if (key == 'c' || key == 'C') {
            cli_show_character_sheet(ctx);
            ctx->game->needs_render = true;
            continue;
        }
        if (key == 27 || key == 'Q') {
            ctx->game->state = GS_QUIT;
            ctx->running = false;
            continue;
        }

        cli_handle_key(ctx, key);
        cli_render(ctx);

        if (ctx->game->state == GS_DEAD) {
            cli_render(ctx);
            gotoxy(0, 21);
            printf("\033[1;31m=== YOU HAVE DIED ===\033[0m\n");
            printf("Press any key to exit...\n");
            fflush(stdout);
            input_get_key();
            break;
        }

        if (ctx->game->state == GS_WON) {
            cli_render(ctx);
            gotoxy(0, 21);
            printf("\033[1;32m=== YOU HAVE WON! ===\033[0m\n");
            printf("Press any key to exit...\n");
            fflush(stdout);
            input_get_key();
            break;
        }
    }
}
