#include "engine/ranged.h"
#include "engine/combat.h"
#include "engine/map.h"
#include "engine/inventory.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

int ranged_init_ammo_defs(AmmoDef *defs, int *count) {
    *count = 7;
    AmmoDef a[7];
    memset(a, 0, sizeof(a));
    a[0] = (AmmoDef){AM_ARROW, "arrow", '/', 1, 6, 0, 8, 0, "A wooden shaft with a metal tip."};
    a[1] = (AmmoDef){AM_BOLT, "crossbow bolt", '=', 1, 8, 1, 10, 1, "A heavy bolt for a crossbow."};
    a[2] = (AmmoDef){AM_DART, "dart", '~', 1, 4, 0, 6, 2, "A small throwing dart."};
    a[3] = (AmmoDef){AM_BULLET, "sling bullet", 'o', 1, 4, 1, 7, 0, "A rounded stone for a sling."};
    a[4] = (AmmoDef){AM_ROCK, "throwing rock", 'O', 1, 3, 0, 5, -1, "A simple thrown rock."};
    a[5] = (AmmoDef){AM_THROWING_KNIFE, "throwing knife", '(', 1, 6, 1, 6, 3, "A balanced blade for throwing."};
    a[6] = (AmmoDef){AM_JAVELIN, "javelin", '|', 1, 8, 2, 8, 0, "A light spear designed for throwing."};
    memcpy(defs, a, sizeof(a));
    return 0;
}

int ranged_get_ammo_index(AmmoType type, const AmmoDef *defs, int count) {
    for (int i = 0; i < count; i++)
        if (defs[i].type == type) return i;
    return -1;
}

int ranged_create_weapon(RangedWeapon *wpn, const char *name, char glyph, int dice, int sides, int bonus, int range, AmmoType ammo) {
    memset(wpn, 0, sizeof(RangedWeapon));
    strncpy(wpn->name, name, MAX_NAME_LEN - 1);
    wpn->glyph = glyph;
    wpn->damage_dice = dice;
    wpn->damage_sides = sides;
    wpn->damage_bonus = bonus;
    wpn->range = range;
    wpn->ammo_type = ammo;
    wpn->accuracy_bonus = 0;
    wpn->shot_cost = 100;
    wpn->two_handed = 1;
    return 0;
}

bool ranged_target_in_line_of_sight(const World *w, int x1, int y1, int x2, int y2) {
    if (!w || !w->map) return false;
    int dx = x2 - x1;
    int dy = y2 - y1;
    int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);
    if (steps == 0) return true;
    float step_x = (float)dx / (float)steps;
    float step_y = (float)dy / (float)steps;
    float cx = (float)x1 + 0.5f;
    float cy = (float)y1 + 0.5f;
    for (int i = 0; i < steps; i++) {
        int tx = (int)cx;
        int ty = (int)cy;
        if (tx == x1 && ty == y1) { cx += step_x; cy += step_y; continue; }
        if (!map_is_transparent(w->map, tx, ty)) return false;
        cx += step_x;
        cy += step_y;
    }
    return true;
}

int ranged_calculate_hit_chance(const Entity *shooter, const Entity *target, const RangedWeapon *wpn, int range) {
    int dx = target->x - shooter->x;
    int dy = target->y - shooter->y;
    int dist = dx * dx + dy * dy;
    int max_dist = range * range;
    if (dist > max_dist) return 0;
    int base = 60 + shooter->dexterity / 2;
    int range_penalty = (int)(sqrtf((float)dist) * 5);
    int hit = base - range_penalty + wpn->accuracy_bonus;
    if (hit < 5) hit = 5;
    if (hit > 95) hit = 95;
    return hit;
}

int ranged_find_ammo_in_inventory(const Entity *e, AmmoType type) {
    for (int i = 0; i < e->inventory_count; i++) {
        Item *it = (Item *)e->inventory[i];
        if (it) {
            if (type == AM_ARROW && it->type == ITEM_WEAPON && it->damage_sides > 0) return i;
        }
    }
    return -1;
}

int ranged_get_ammo_count(const Entity *e, AmmoType type) {
    (void)type;
    int count = 0;
    for (int i = 0; i < e->inventory_count; i++) {
        Item *it = (Item *)e->inventory[i];
        if (it && it->type == ITEM_WEAPON && it->damage_dice > 0) count += it->stack_count;
    }
    return count;
}

int ranged_consume_ammo(Entity *e, AmmoType type) {
    int idx = ranged_find_ammo_in_inventory(e, type);
    if (idx < 0) return -1;
    Item *it = (Item *)e->inventory[idx];
    if (it) {
        it->stack_count--;
        if (it->stack_count <= 0) {
            inventory_remove(e, idx);
            return 0;
        }
    }
    return 0;
}

int ranged_fire_weapon(Entity *shooter, Entity *target, const RangedWeapon *wpn, const AmmoDef *ammo, World *w, RNGState *rng) {
    if (!shooter || !target || !wpn || !ammo || !w || !rng) return -1;
    if (!ranged_target_in_line_of_sight(w, shooter->x, shooter->y, target->x, target->y)) return -1;
    int hit_chance = ranged_calculate_hit_chance(shooter, target, wpn, wpn->range);
    bool hit = rng_range(rng, 0, 100) < hit_chance;
    if (!hit) return 0;
    int dmg = 0;
    for (int d = 0; d < wpn->damage_dice; d++) dmg += rng_dice(rng, wpn->damage_sides);
    dmg += wpn->damage_bonus + ammo->damage_bonus;
    if (dmg < 1) dmg = 1;
    entity_take_damage(target, dmg, DT_RANGED);
    return dmg;
}

bool ranged_is_ranged_weapon(const Item *it) {
    if (!it || it->type != ITEM_WEAPON) return false;
    return strstr(it->name, "bow") || strstr(it->name, "crossbow") ||
           strstr(it->name, "sling") || strstr(it->name, "blowgun");
}

int ranged_get_weapon_range(const Item *it) {
    (void)it;
    return 8;
}
