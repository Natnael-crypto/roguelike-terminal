#include "core/game.h"
#include "engine/entity.h"
#include "engine/combat.h"
#include "engine/dungeon.h"
#include "engine/inventory.h"
#include "engine/ai.h"
#include "engine/item.h"
#include "engine/map.h"
#include "systems/event.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "engine/equipment.h"
#include "engine/ranged.h"
#include "engine/trap.h"
#include "engine/shop.h"

static void generate_monsters_for_level(Game *game, int depth, int count) {
    for (int i = 0; i < count && i < MAX_MONSTERS; i++) {
        int mx, my;
        if (dungeon_find_random_floor(game->world.map, &mx, &my, &game->rng)) {
            Entity *mon = entity_create_monster_for_depth(depth, mx, my, &game->rng);
            if (mon) {
                world_add_monster(&game->world, mon);
            }
        }
    }
}

static void generate_items_for_level(Game *game, int depth, int count) {
    for (int i = 0; i < count && i < MAX_ITEMS; i++) {
        int ix, iy;
        if (dungeon_find_random_floor(game->world.map, &ix, &iy, &game->rng)) {
            Item *it = NULL;
            int roll = rng_range(&game->rng, 0, 100);
            if (roll < 10 && depth > 2) {
                it = item_create("", '=', ITEM_RING);
                if (it) {
                    equipment_generate_random(it, depth, &game->rng);
                }
            } else {
                it = item_generate_for_depth(depth, &game->rng);
            }
            if (it) {
                it->x = ix;
                it->y = iy;
                world_add_item(&game->world, it);
            }
        }
    }
}

static void apply_theme_to_dungeon(Game *game, int depth) {
    int theme = dungeon_get_theme_for_depth(depth);
    const char *theme_name = dungeon_get_theme_name(depth);

    /* Add decorations based on theme */
    dungeon_add_decoration(game->world.map, (DungeonTheme)theme, &game->rng);

    /* Add vault room occasionally */
    if (rng_range(&game->rng, 0, 100) < 15) {
        Dungeon fake_d;
        fake_d.room_count = rng_range(&game->rng, 1, 3);
        dungeon_add_vault_room(&fake_d, game->world.map, &game->rng);
    }

    game_add_message(game, "This level feels like %s...", theme_name);
}

static int generate_dungeon_level(Game *game, int depth) {
    if (game->world.map) {
        free(game->world.map);
        game->world.map = NULL;
    }

    game->world.map = (Map *)malloc(sizeof(Map));
    if (!game->world.map) return -1;

    map_init(game->world.map, 60, 20);

    Dungeon dd;
    dungeon_generate(&dd, 60, 20,
        game->config.dungeon.min_rooms,
        game->config.dungeon.max_rooms,
        &game->rng);
    dungeon_place_on_map(&dd, game->world.map, depth);
    dungeon_connect_rooms(&dd, game->world.map, &game->rng);
    dungeon_place_stairs(&dd, game->world.map, depth < game->config.max_dungeon_level, &game->rng);

    if (game->config.enable_fog) {
        for (int y = 0; y < game->world.map->height; y++)
            for (int x = 0; x < game->world.map->width; x++)
                game->world.map->visible[x][y] = false;
    }

    if (game->world.player) {
        int rx, ry;
        dungeon_find_random_floor(game->world.map, &rx, &ry, &game->rng);
        game->world.player->x = rx;
        game->world.player->y = ry;
    }

    for (int i = 0; i < game->world.monster_count; i++) {
        if (game->world.monsters[i]) {
            entity_destroy(game->world.monsters[i]);
            free(game->world.monsters[i]);
            game->world.monsters[i] = NULL;
        }
    }
    game->world.monster_count = 0;

    int mon_count = game->config.dungeon.monster_density * 3 + rng_range(&game->rng, 0, 5);
    for (int i = 0; i < mon_count && i < MAX_MONSTERS; i++) {
        int mx, my;
        if (dungeon_find_random_floor(game->world.map, &mx, &my, &game->rng)) {
            const char *mnames[] = {"rat", "goblin", "skeleton", "orc", "wraith", "bat"};
            char glyphs[] = {'r', 'g', 's', 'o', 'W', 'b'};
            int idx = (depth + i) % 6;
            Entity *mon = entity_create(mnames[idx], glyphs[idx], mx, my);
            if (mon) {
                mon->is_monster = true;
                mon->alive = true;
                mon->blocks_movement = true;
                mon->health = 10 + depth * 3 + rng_range(&game->rng, 0, 5);
                mon->max_health = mon->health;
                mon->attack = 3 + depth * 2;
                mon->defense = 1 + depth;
                mon->strength = 8 + depth;
                mon->dexterity = 8 + depth / 2;
                mon->damage_dice = 1;
                mon->damage_sides = 4 + depth;
                mon->damage_bonus = depth;
                mon->xp_value = 10 + depth * 5;
                mon->behavior = AI_WANDER;
                mon->alert_range = 6 + depth;
                mon->speed = 80 + rng_range(&game->rng, 0, 40);
                mon->vision_range = 8;
                entity_apply_status(mon, 0, 0, 0, 0);
                world_add_monster(&game->world, mon);
            }
        }
    }

    for (int i = 0; i < game->world.item_count; i++) {
        if (game->world.items[i]) {
            item_destroy(game->world.items[i]);
            free(game->world.items[i]);
            game->world.items[i] = NULL;
        }
    }
    game->world.item_count = 0;

    int item_count = game->config.dungeon.item_density * 2 + rng_range(&game->rng, 0, 3);
    for (int i = 0; i < item_count && i < MAX_ITEMS; i++) {
        int ix, iy;
        if (dungeon_find_random_floor(game->world.map, &ix, &iy, &game->rng)) {
            const char *inames[] = {"potion", "scroll", "gold", "dagger", "shield"};
            char iglyphs[] = {'!', '?', '$', '/', '['};
            ItemType itypes[] = {ITEM_POTION, ITEM_SCROLL, ITEM_GOLD, ITEM_WEAPON, ITEM_ARMOR};
            int idx = rng_range(&game->rng, 0, 5);
            Item *it = item_create(inames[idx], iglyphs[idx], itypes[idx]);
            if (it) {
                it->x = ix;
                it->y = iy;

                if (itypes[idx] == ITEM_POTION) {
                    it->heal_amount = rng_range(&game->rng, 5, 20);
                } else if (itypes[idx] == ITEM_GOLD) {
                    it->value = rng_range(&game->rng, 5, 50);
                } else if (itypes[idx] == ITEM_WEAPON) {
                    it->damage_dice = 1;
                    it->damage_sides = 4 + depth / 2;
                    it->damage_bonus = depth / 2;
                } else if (itypes[idx] == ITEM_ARMOR) {
                    it->armor_value = 1 + depth / 3;
                    it->armor_slot = SLOT_CHEST;
                }

                world_add_item(&game->world, it);
            }
        }
    }

    game_update_fov(game);
    game_add_message(game, "You enter dungeon level %d.", depth);
    return 0;
}

int game_init(Game *game, GameConfig *cfg) {
    memset(game, 0, sizeof(Game));
    game->config = *cfg;

    if (cfg->rng_seed == 0) {
        cfg->rng_seed = (uint32_t)(__TIME__[0] * 100000 + __TIME__[1] * 10000 +
                                    __TIME__[3] * 1000 + __TIME__[4] * 100 +
                                    __TIME__[6] * 10 + __TIME__[7]);
    }
    rng_init(&game->rng, cfg->rng_seed);

    world_init(&game->world, cfg);
    event_queue_init(&game->events);
    game->state = GS_PLAYING;
    game->tick_count = 0;
    game->needs_render = true;

    game->world.player = entity_create("player", '@', 10, 10);
    if (!game->world.player) return -1;

    game->world.player->is_player = true;
    game->world.player->alive = true;
    game->world.player->blocks_movement = true;
    game->world.player->health = cfg->start_health;
    game->world.player->max_health = cfg->start_health;
    game->world.player->mana = cfg->start_mana;
    game->world.player->max_mana = cfg->start_mana;
    game->world.player->strength = 10;
    game->world.player->dexterity = 10;
    game->world.player->intelligence = 10;
    game->world.player->attack = 5;
    game->world.player->defense = 3;
    game->world.player->armor = 0;
    game->world.player->speed = 100;
    game->world.player->luck = 10;
    game->world.player->level = 1;
    game->world.player->experience = 0;
    game->world.player->next_level_exp = 20;
    game->world.player->damage_dice = 1;
    game->world.player->damage_sides = 6;
    game->world.player->damage_bonus = 0;
    game->world.player->inventory_capacity = MAX_INVENTORY_SLOTS;
    game->world.player->inventory_count = 0;
    game->world.player->inventory = (void **)malloc(sizeof(void *) * MAX_INVENTORY_SLOTS);
    game->world.player->vision_range = FOV_RADIUS;

    int rc = generate_dungeon_level(game, 1);
    if (rc != 0) return rc;

    game_add_message(game, "Welcome to the Roguelike Dungeon!");
    game_add_message(game, "Use WASD to move, ? for help.");
    return 0;
}

void game_destroy(Game *game) {
    if (game->world.player) {
        entity_destroy(game->world.player);
        free(game->world.player);
        game->world.player = NULL;
    }
    world_destroy(&game->world);
}

void game_add_message(Game *game, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if (game->message_count < MAX_LOG_LINES) {
        vsnprintf(game->message_log[game->message_count], 128, fmt, args);
        game->message_count++;
        if (game->message_count > 20)
            game->message_scroll = game->message_count - 20;
    }
    va_end(args);
    game->needs_render = true;
}

const char *game_get_message(const Game *game, int index) {
    if (index < 0 || index >= game->message_count) return "";
    return game->message_log[index];
}

void game_update_fov(Game *game) {
    if (!game->world.map || !game->world.player) return;
    map_compute_fov(game->world.map,
        game->world.player->x,
        game->world.player->y,
        game->config.fov_radius);
    game->needs_render = true;
}

int game_tick(Game *game) {
    if (game->state != GS_PLAYING) return -1;
    if (!game->world.player || !game->world.player->alive) {
        game->state = GS_DEAD;
        return -1;
    }
    game->tick_count++;
    game->world.turn_count++;

    for (int i = 0; i < game->world.monster_count; i++) {
        Entity *mon = game->world.monsters[i];
        if (!mon || !mon->alive) continue;

        entity_tick_status(mon);

        if (mon->health <= 0) {
            mon->alive = false;
            game_add_message(game, "The %s is defeated!", mon->name);
            if (game->world.player) {
                game->world.player->experience += mon->xp_value;
                game->world.player->gold += mon->gold;
                if (game->world.player->experience >= game->world.player->next_level_exp) {
                    game->world.player->level++;
                    game->world.player->next_level_exp += game->world.player->level * 10;
                    game->world.player->max_health += 5;
                    game->world.player->health = game->world.player->max_health;
                    game->world.player->strength += 1;
                    game->world.player->dexterity += 1;
                    game->world.player->attack += 1;
                    game_add_message(game, "Level up! You are now level %d!", game->world.player->level);
                }
            }
            continue;
        }

        if (mon->stun_turns > 0) {
            mon->stun_turns--;
            continue;
        }

        ai_update(mon, &game->world);
    }

    game_update_fov(game);

    GameEvent evt = event_make(EV_TICK, 0, 0, 0, 0, game->tick_count, game->world.turn_count);
    event_queue_push(&game->events, &evt);

    game->needs_render = true;
    return 0;
}

void game_handle_input(Game *game, int key) {
    if (!game->world.player || !game->world.player->alive) return;

    int nx = game->world.player->x;
    int ny = game->world.player->y;
    bool moved = false;
    char action[16];

    switch (key) {
        case 'w': case 'W': ny--; moved = true; strcpy(action, "move_n"); break;
        case 's': case 'S': ny++; moved = true; strcpy(action, "move_s"); break;
        case 'a': case 'A': nx--; moved = true; strcpy(action, "move_w"); break;
        case 'd': case 'D': nx++; moved = true; strcpy(action, "move_e"); break;
        case 'q': nx--; ny--; moved = true; strcpy(action, "move_nw"); break;
        case 'e': nx++; ny--; moved = true; strcpy(action, "move_ne"); break;
        case 'z': nx--; ny++; moved = true; strcpy(action, "move_sw"); break;
        case 'x': nx++; ny++; moved = true; strcpy(action, "move_se"); break;
        case '.': moved = true; strcpy(action, "wait"); break;
        case 'g': case 'G': {
            Item *it = world_get_item_at(&game->world, game->world.player->x, game->world.player->y);
            if (it) {
                inventory_add(game->world.player, it);
                game_add_message(game, "You pick up the %s.", it->name);
                for (int i = 0; i < game->world.item_count; i++) {
                    if (game->world.items[i] == it) {
                        game->world.items[i] = NULL;
                        break;
                    }
                }
            } else {
                game_add_message(game, "There's nothing here to pick up.");
            }
            moved = true;
            strcpy(action, "get");
            break;
        }
        case 'i': case 'I': {
            game_add_message(game, "=== Inventory ===");
            for (int s = 0; s < game->world.player->inventory_count && s < game->world.player->inventory_capacity; s++) {
                Item *it = ((Item **)game->world.player->inventory)[s];
                if (it) game_add_message(game, "  %d: %s", s, it->name);
            }
            game->needs_render = true;
            return;
        }
        case '>': {
            if (game->world.map &&
                game->world.player->x == game->world.map->stairs_down_x &&
                game->world.player->y == game->world.map->stairs_down_y) {
                world_next_level(&game->world);
                generate_dungeon_level(game, game->world.current_depth);
            } else {
                game_add_message(game, "There are no stairs down here.");
            }
            moved = true;
            strcpy(action, "down");
            break;
        }
        case '<': {
            if (game->world.map &&
                game->world.player->x == game->world.map->stairs_up_x &&
                game->world.player->y == game->world.map->stairs_up_y) {
                world_prev_level(&game->world);
                generate_dungeon_level(game, game->world.current_depth);
            } else {
                game_add_message(game, "There are no stairs up here.");
            }
            moved = true;
            strcpy(action, "up");
            break;
        }
        case '?': {
            game_add_message(game, "WASD: move, q/e/z/x: diagonals");
            game_add_message(game, ". wait, g get, i inventory");
            game_add_message(game, "< > stairs, ? help, Q quit");
            game->needs_render = true;
            return;
        }
        case 'Q': {
            game->state = GS_QUIT;
            return;
        }
        default:
            return;
    }

    if (moved) {
        if (key != 'g' && key != '>' && key != '<' && key != '.') {
            Entity *target = world_get_entity_at(&game->world, nx, ny);
            if (target && target != game->world.player && target->alive && target->is_monster) {
                CombatResult cr;
                combat_attack(game->world.player, target, &cr);
                if (cr.hit) {
                    game_add_message(game, "You hit %s for %d damage!", target->name, cr.total_damage);
                    if (!target->alive) {
                        game_add_message(game, "The %s is slain!", target->name);
                    }
                } else {
                    game_add_message(game, "You miss the %s.", target->name);
                }
            } else if (!world_is_blocked(&game->world, nx, ny)) {
                world_move_entity(&game->world, game->world.player, nx, ny);
            } else {
                game_add_message(game, "Something blocks your path.");
            }
        }

        world_record_action(&game->world, action);
        game_tick(game);
    }
}
