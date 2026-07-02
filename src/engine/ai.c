#include "engine/ai.h"
#include "engine/combat.h"
#include "engine/pathfinding.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "systems/rng.h"

void ai_update(Entity *e, World *w) {
    if (!e || !e->alive || !w) return;

    switch (e->behavior) {
        case AI_WANDER:
            ai_wander(e, w);
            break;
        case AI_PATROL:
            ai_patrol(e, w);
            break;
        case AI_AGGRESSIVE:
            ai_aggressive(e, w);
            break;
        case AI_COWARD:
            ai_coward(e, w);
            break;
        case AI_GUARD:
            ai_guard(e, w);
            break;
        case AI_BERSERK:
            ai_berserk(e, w);
            break;
        case AI_PACK:
            ai_pack(e, w);
            break;
        case AI_SNIPER:
            ai_sniper(e, w);
            break;
        case AI_SUMMONER:
            ai_summoner(e, w);
            break;
        case AI_FLEE:
            ai_flee(e, w);
            break;
    }
}

bool ai_can_see_player(const Entity *e, const World *w) {
    if (!w->player || !w->player->alive) return false;
    if (!w->map) return false;
    int dx = w->player->x - e->x;
    int dy = w->player->y - e->y;
    int dist = dx * dx + dy * dy;
    return dist <= e->alert_range * e->alert_range;
}

bool ai_player_in_range(const Entity *e, const World *w, int range) {
    if (!w->player) return false;
    int dx = w->player->x - e->x;
    int dy = w->player->y - e->y;
    return (dx * dx + dy * dy) <= range * range;
}

bool ai_adjacent_to_player(const Entity *e, const World *w) {
    return ai_player_in_range(e, w, 2);
}

void ai_melee_attack(Entity *e, World *w) {
    if (!w->player || !w->player->alive) return;
    CombatResult cr;
    combat_attack(e, w->player, &cr);
}

void ai_move_toward(Entity *e, World *w, int tx, int ty) {
    int dx = (tx > e->x) - (tx < e->x);
    int dy = (ty > e->y) - (ty < e->y);

    int nx = e->x + dx;
    int ny = e->y + dy;

    if (!world_is_blocked(w, nx, ny)) {
        world_move_entity(w, e, nx, ny);
        return;
    }

    if (dx != 0) {
        nx = e->x + dx;
        ny = e->y;
        if (!world_is_blocked(w, nx, ny)) {
            world_move_entity(w, e, nx, ny);
            return;
        }
    }

    if (dy != 0) {
        nx = e->x;
        ny = e->y + dy;
        if (!world_is_blocked(w, nx, ny)) {
            world_move_entity(w, e, nx, ny);
            return;
        }
    }
}

void ai_move_away(Entity *e, World *w, int tx, int ty) {
    int dx = (e->x > tx) - (e->x < tx);
    int dy = (e->y > ty) - (e->y < ty);
    int nx = e->x + dx;
    int ny = e->y + dy;
    if (!world_is_blocked(w, nx, ny))
        world_move_entity(w, e, nx, ny);
}

void ai_wander(Entity *e, World *w) {
    if (ai_can_see_player(e, w)) {
        e->behavior = AI_AGGRESSIVE;
        ai_aggressive(e, w);
        return;
    }

    e->move_counter--;
    if (e->move_counter <= 0) {
        e->move_counter = 5 + rand() % 10;
        int dir = rand() % 4;
        int nx = e->x, ny = e->y;
        switch (dir) {
            case 0: ny--; break;
            case 1: ny++; break;
            case 2: nx--; break;
            case 3: nx++; break;
        }
        if (!world_is_blocked(w, nx, ny))
            world_move_entity(w, e, nx, ny);
    }
}

void ai_patrol(Entity *e, World *w) {
    if (ai_can_see_player(e, w)) {
        e->behavior = AI_AGGRESSIVE;
        ai_aggressive(e, w);
        return;
    }
    ai_wander(e, w);
}

void ai_aggressive(Entity *e, World *w) {
    if (!w->player || !w->player->alive) {
        e->behavior = AI_WANDER;
        return;
    }

    if (ai_adjacent_to_player(e, w)) {
        ai_melee_attack(e, w);
        return;
    }

    if (!ai_can_see_player(e, w)) {
        e->behavior = AI_WANDER;
        return;
    }

    ai_move_toward(e, w, w->player->x, w->player->y);
}

void ai_coward(Entity *e, World *w) {
    if (!w->player || !w->player->alive) return;

    if (ai_player_in_range(e, w, e->alert_range)) {
        ai_move_away(e, w, w->player->x, w->player->y);
    } else {
        ai_wander(e, w);
    }
}

void ai_guard(Entity *e, World *w) {
    if (ai_adjacent_to_player(e, w)) {
        ai_melee_attack(e, w);
    } else if (ai_can_see_player(e, w)) {
        ai_move_toward(e, w, w->player->x, w->player->y);
    }
}

void ai_use_ability(Entity *e, World *w) {
    (void)e;
    (void)w;
}

/* === New Tactical AI Behaviors === */

void ai_berserk(Entity *e, World *w) {
    if (!w->player || !w->player->alive) {
        e->behavior = AI_WANDER;
        return;
    }

    /* Berserk: charge at player ignoring everything, bonus damage */
    if (ai_adjacent_to_player(e, w)) {
        ai_melee_attack(e, w);
        /* Bonus damage from berserk rage */
        if (w->player && w->player->alive && rng_range(&w->rng, 0, 100) < 30) {
            CombatResult cr2;
            combat_attack(e, w->player, &cr2);
        }
        return;
    }

    if (!ai_can_see_player(e, w)) {
        e->behavior = AI_WANDER;
        return;
    }

    /* Move toward player diagonally (shortest path) */
    int dx = (w->player->x > e->x) - (w->player->x < e->x);
    int dy = (w->player->y > e->y) - (w->player->y < e->y);
    if (!world_is_blocked(w, e->x + dx, e->y + dy))
        world_move_entity(w, e, e->x + dx, e->y + dy);
    else if (!world_is_blocked(w, e->x + dx, e->y))
        world_move_entity(w, e, e->x + dx, e->y);
    else if (!world_is_blocked(w, e->x, e->y + dy))
        world_move_entity(w, e, e->x, e->y + dy);
}

void ai_pack(Entity *e, World *w) {
    if (!w->player || !w->player->alive) {
        e->behavior = AI_WANDER;
        return;
    }

    /* Pack: attack as a group, flank the player */
    if (ai_adjacent_to_player(e, w)) {
        ai_melee_attack(e, w);
        return;
    }

    int allies_near = ai_count_allies_nearby(e, w, 5);

    if (ai_can_see_player(e, w) && allies_near >= 1) {
        /* With allies, move to flank: approach from different angle */
        int pdx = w->player->x - e->x;
        int pdy = w->player->y - e->y;
        if (allies_near >= 3) {
            /* Enough allies, surround from opposite side */
            ai_move_toward(e, w,
                w->player->x + (pdx > 0 ? -1 : 1),
                w->player->y + (pdy > 0 ? -1 : 1));
        } else {
            ai_move_toward(e, w, w->player->x, w->player->y);
        }
    } else if (ai_can_see_player(e, w)) {
        /* No allies nearby but can see player - call for help */
        ai_move_toward(e, w, w->player->x, w->player->y);
    } else {
        ai_wander(e, w);
    }
}

void ai_sniper(Entity *e, World *w) {
    if (!w->player || !w->player->alive) {
        e->behavior = AI_WANDER;
        return;
    }

    /* Sniper: keep distance and attack ranged */
    if (ai_adjacent_to_player(e, w)) {
        /* Too close! Try to flee to range */
        ai_move_away(e, w, w->player->x, w->player->y);
        return;
    }

    if (!ai_can_see_player(e, w)) {
        e->behavior = AI_PATROL;
        return;
    }

    int dist = (w->player->x - e->x) * (w->player->x - e->x) +
               (w->player->y - e->y) * (w->player->y - e->y);

    if (dist >= 16 && dist <= 64) {
        /* In optimal range, attack */
        CombatResult cr;
        combat_attack(e, w->player, &cr);
        if (cr.hit) {
            /* Extra damage from precision */
            int extra = e->dexterity / 4;
            w->player->health -= extra;
            if (w->player->health <= 0) {
                w->player->health = 0;
                w->player->alive = false;
            }
        }
    } else if (dist < 16) {
        ai_move_away(e, w, w->player->x, w->player->y);
    } else {
        ai_move_toward(e, w, w->player->x, w->player->y);
    }
}

void ai_summoner(Entity *e, World *w) {
    if (!w->player || !w->player->alive) {
        e->behavior = AI_WANDER;
        return;
    }

    /* Summoner: stays at range and summons minions */
    if (ai_adjacent_to_player(e, w)) {
        ai_move_away(e, w, w->player->x, w->player->y);
        return;
    }

    if (!ai_can_see_player(e, w)) {
        e->behavior = AI_PATROL;
        return;
    }

    int allies_near = ai_count_allies_nearby(e, w, 6);

    if (allies_near < 3 && rng_range(&w->rng, 0, 100) < 25) {
        /* Summon a minion */
        int sx = e->x + (rng_range(&w->rng, 0, 2) == 0 ? -1 : 1);
        int sy = e->y + (rng_range(&w->rng, 0, 2) == 0 ? -1 : 1);
        if (map_in_bounds(w->map, sx, sy) && !world_is_blocked(w, sx, sy)) {
            Entity *minion = entity_create_monster("summoned skeleton", 's', sx, sy, w->current_depth, 5);
            if (minion) {
                minion->is_monster = true;
                minion->alive = true;
                minion->blocks_movement = true;
                minion->faction_id = e->faction_id;
                minion->behavior = AI_AGGRESSIVE;
                minion->health = 5 + w->current_depth;
                minion->max_health = minion->health;
                world_add_monster(w, minion);
            }
        }
    }

    if (ai_player_in_range(e, w, 5)) {
        ai_move_away(e, w, w->player->x, w->player->y);
    }
}

void ai_flee(Entity *e, World *w) {
    if (!w->player || !w->player->alive) return;

    /* Flee: run away from the player until safe, then patrol */
    if (!ai_player_in_range(e, w, e->alert_range)) {
        e->behavior = AI_PATROL;
        return;
    }

    if (ai_adjacent_to_player(e, w)) {
        /* Desperate hit when cornered */
        ai_melee_attack(e, w);
    }

    ai_move_away(e, w, w->player->x, w->player->y);
}

/* === Tactical Helpers === */

int ai_count_allies_nearby(const Entity *e, const World *w, int range) {
    int count = 0;
    for (int i = 0; i < w->monster_count; i++) {
        Entity *other = w->monsters[i];
        if (!other || !other->alive || other == e) continue;
        if (other->faction_id != e->faction_id) continue;
        int dx = other->x - e->x;
        int dy = other->y - e->y;
        if ((dx * dx + dy * dy) <= range * range)
            count++;
    }
    return count;
}

int ai_count_enemies_nearby(const Entity *e, const World *w, int range) {
    int count = 0;
    for (int i = 0; i < w->monster_count; i++) {
        Entity *other = w->monsters[i];
        if (!other || !other->alive) continue;
        if (other->faction_id == e->faction_id) continue;
        int dx = other->x - e->x;
        int dy = other->y - e->y;
        if ((dx * dx + dy * dy) <= range * range)
            count++;
    }
    if (w->player && w->player->alive && w->player->faction_id != e->faction_id) {
        int dx = w->player->x - e->x;
        int dy = w->player->y - e->y;
        if ((dx * dx + dy * dy) <= range * range)
            count++;
    }
    return count;
}

bool ai_has_path_to(const Entity *e, const World *w, int tx, int ty) {
    (void)e;
    (void)w;
    (void)tx;
    (void)ty;
    /* Simple stub: would use pathfinding in full implementation */
    return true;
}

void ai_move_to_safe_position(Entity *e, World *w, int from_x, int from_y) {
    int best_x = e->x, best_y = e->y;
    int best_dist = -1;

    for (int dy = -3; dy <= 3; dy++) {
        for (int dx = -3; dx <= 3; dx++) {
            int nx = e->x + dx;
            int ny = e->y + dy;
            if (!world_is_blocked(w, nx, ny)) {
                int d = (nx - from_x) * (nx - from_x) + (ny - from_y) * (ny - from_y);
                if (d > best_dist) {
                    best_dist = d;
                    best_x = nx;
                    best_y = ny;
                }
            }
        }
    }

    if (best_dist > 0) {
        world_move_entity(w, e, best_x, best_y);
    }
}

Entity *ai_find_nearest_ally(Entity *e, const World *w, int range) {
    Entity *nearest = NULL;
    int best_dist = range * range + 1;
    for (int i = 0; i < w->monster_count; i++) {
        Entity *other = w->monsters[i];
        if (!other || !other->alive || other == e) continue;
        if (other->faction_id != e->faction_id) continue;
        int dx = other->x - e->x;
        int dy = other->y - e->y;
        int dist = dx * dx + dy * dy;
        if (dist < best_dist) {
            best_dist = dist;
            nearest = other;
        }
    }
    return nearest;
}

Entity *ai_find_weakest_enemy(Entity *e, const World *w, int range) {
    Entity *weakest = NULL;
    int min_hp = 9999;
    for (int i = 0; i < w->monster_count; i++) {
        Entity *other = w->monsters[i];
        if (!other || !other->alive) continue;
        if (other->faction_id == e->faction_id) continue;
        int dx = other->x - e->x;
        int dy = other->y - e->y;
        if ((dx * dx + dy * dy) <= range * range && other->health < min_hp) {
            min_hp = other->health;
            weakest = other;
        }
    }
    if (w->player && w->player->alive &&
        w->player->faction_id != e->faction_id) {
        int dx = w->player->x - e->x;
        int dy = w->player->y - e->y;
        if ((dx * dx + dy * dy) <= range * range && w->player->health < min_hp) {
            weakest = w->player;
        }
    }
    return weakest;
}
