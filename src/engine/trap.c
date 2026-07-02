#include "engine/trap.h"
#include "engine/map.h"
#include "engine/combat.h"
#include "engine/ai.h"
#include "systems/rng.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

const char *trap_name(TrapType type) {
    switch (type) {
        case TRAP_ARROW: return "arrow trap";
        case TRAP_POISON_DART: return "poison dart trap";
        case TRAP_PIT: return "pit trap";
        case TRAP_TELEPORT: return "teleport trap";
        case TRAP_CONFUSION: return "confusion trap";
        case TRAP_SLOW: return "slow trap";
        case TRAP_FIRE: return "fire trap";
        case TRAP_EXPLOSION: return "explosion trap";
        case TRAP_ALARM: return "alarm trap";
        case TRAP_SUMMON: return "summon trap";
        default: return "unknown trap";
    }
}

char trap_glyph(TrapType type) {
    (void)type;
    return '^';
}

int trap_damage(TrapType type, int depth) {
    switch (type) {
        case TRAP_ARROW: return 2 + depth;
        case TRAP_POISON_DART: return 1 + depth / 2;
        case TRAP_FIRE: return 3 + depth * 2;
        case TRAP_EXPLOSION: return 5 + depth * 3;
        default: return 0;
    }
}

int trap_detection_difficulty(TrapType type) {
    switch (type) {
        case TRAP_PIT: return 5;
        case TRAP_ALARM: return 8;
        case TRAP_SUMMON: return 12;
        case TRAP_EXPLOSION: return 10;
        default: return 7;
    }
}

void trap_init(Trap *t, TrapType type, int x, int y) {
    memset(t, 0, sizeof(Trap));
    t->x = x;
    t->y = y;
    t->type = type;
    t->triggered = false;
    t->visible = false;
    t->damage = 0;
    t->glyph = trap_glyph(type);
    t->detection_difficulty = trap_detection_difficulty(type);
    strncpy(t->name, trap_name(type), MAX_TRAP_NAME - 1);
}

void trap_list_init(TrapList *tl) {
    tl->count = 0;
    memset(tl->traps, 0, sizeof(tl->traps));
}

int trap_list_add(TrapList *tl, TrapType type, int x, int y) {
    if (tl->count >= MAX_TRAPS) return -1;
    trap_init(&tl->traps[tl->count], type, x, y);
    tl->count++;
    return 0;
}

Trap *trap_at(TrapList *tl, int x, int y) {
    for (int i = 0; i < tl->count; i++) {
        if (tl->traps[i].x == x && tl->traps[i].y == y && !tl->traps[i].triggered)
            return &tl->traps[i];
    }
    return NULL;
}

bool trap_detect(const Trap *t, int detect_power) {
    return detect_power >= t->detection_difficulty;
}

void trap_reveal_nearby(TrapList *tl, int px, int py, int radius) {
    for (int i = 0; i < tl->count; i++) {
        int dx = tl->traps[i].x - px;
        int dy = tl->traps[i].y - py;
        if (dx * dx + dy * dy <= radius * radius) {
            tl->traps[i].visible = true;
        }
    }
}

bool trap_trigger(Trap *t, Entity *victim, World *w) {
    if (!t || !victim || t->triggered) return false;
    t->triggered = true;

    switch (t->type) {
        case TRAP_ARROW: {
            int dmg = t->damage > 0 ? t->damage : 3;
            entity_take_damage(victim, dmg, DT_MELEE);
            break;
        }
        case TRAP_POISON_DART: {
            int dmg = t->damage > 0 ? t->damage : 2;
            entity_take_damage(victim, dmg, DT_POISON);
            entity_apply_status(victim, 5, 0, 0, 0);
            break;
        }
        case TRAP_PIT: {
            int dmg = 5;
            entity_take_damage(victim, dmg, DT_MELEE);
            victim->stun_turns += 2;
            break;
        }
        case TRAP_TELEPORT: {
            if (w && w->map) {
                int nx = victim->x + (rand() % 7 - 3);
                int ny = victim->y + (rand() % 7 - 3);
                if (map_in_bounds(w->map, nx, ny) && map_is_walkable(w->map, nx, ny)) {
                    victim->x = nx;
                    victim->y = ny;
                }
            }
            break;
        }
        case TRAP_CONFUSION: {
            victim->stun_turns += 5;
            break;
        }
        case TRAP_SLOW: {
            entity_apply_status(victim, 0, 0, 8, 0);
            break;
        }
        case TRAP_FIRE: {
            int dmg = t->damage > 0 ? t->damage : 6;
            entity_take_damage(victim, dmg, DT_FIRE);
            entity_apply_status(victim, 0, 3, 0, 0);
            break;
        }
        case TRAP_EXPLOSION: {
            int dmg = t->damage > 0 ? t->damage : 10;
            entity_take_damage(victim, dmg, DT_FIRE);
            if (w && w->map) {
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        if (dx == 0 && dy == 0) continue;
                        int ex = t->x + dx;
                        int ey = t->y + dy;
                        if (!map_in_bounds(w->map, ex, ey)) continue;
                        Entity *other = world_get_entity_at(w, ex, ey);
                        if (other && other != victim && other->alive) {
                            entity_take_damage(other, dmg / 2, DT_FIRE);
                        }
                    }
                }
            }
            break;
        }
        case TRAP_ALARM: {
            if (w) {
                for (int i = 0; i < w->monster_count; i++) {
                    if (w->monsters[i] && w->monsters[i]->alive) {
                        w->monsters[i]->behavior = AI_AGGRESSIVE;
                        w->monsters[i]->alert_range = 20;
                    }
                }
            }
            break;
        }
        case TRAP_SUMMON: {
            if (w && w->map) {
                for (int s = 0; s < 3; s++) {
                    int sx = t->x + (rand() % 5 - 2);
                    int sy = t->y + (rand() % 5 - 2);
                    if (map_in_bounds(w->map, sx, sy) && map_is_walkable(w->map, sx, sy)) {
                        Entity *summon = entity_create("summoned pest", 's', sx, sy);
                        if (summon) {
                            summon->is_monster = true;
                            summon->alive = true;
                            summon->blocks_movement = true;
                            summon->health = 8;
                            summon->max_health = 8;
                            summon->attack = 4;
                            summon->defense = 1;
                            summon->behavior = AI_AGGRESSIVE;
                            summon->alert_range = 20;
                            world_add_monster(w, summon);
                        }
                    }
                }
            }
            break;
        }
        default:
            return false;
    }
    return true;
}

int trap_place_random(TrapList *tl, const Map *m, int depth, RNGState *rng) {
    int x, y;
    int attempts = 0;
    while (attempts < 50) {
        attempts++;
        x = rng_range(rng, 0, m->width);
        y = rng_range(rng, 0, m->height);
        if (map_is_walkable(m, x, y) && map_get_room_at(m, x, y) >= 0) {
            TrapType types[] = {TRAP_ARROW, TRAP_POISON_DART, TRAP_PIT, TRAP_TELEPORT,
                               TRAP_CONFUSION, TRAP_SLOW, TRAP_FIRE, TRAP_EXPLOSION,
                               TRAP_ALARM, TRAP_SUMMON};
            int count = sizeof(types) / sizeof(types[0]);
            int idx = rng_range(rng, 0, count);
            trap_list_add(tl, types[idx], x, y);
            return 1;
        }
    }
    return 0;
}

void trap_generate_for_level(TrapList *tl, Map *m, int depth, RNGState *rng) {
    trap_list_init(tl);
    int num_traps = 3 + depth * 2;
    if (num_traps > MAX_TRAPS) num_traps = MAX_TRAPS;
    for (int i = 0; i < num_traps; i++) {
        trap_place_random(tl, m, depth, rng);
    }
}
