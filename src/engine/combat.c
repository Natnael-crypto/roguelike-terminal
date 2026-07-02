#include "engine/combat.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static RNGState *combat_rng = NULL;

void combat_set_rng(RNGState *rng) {
    combat_rng = rng;
}

static RNGState *get_rng(void) {
    static RNGState fallback;
    static bool fallback_init = false;
    if (combat_rng) return combat_rng;
    if (!fallback_init) {
        rng_init(&fallback, 42);
        fallback_init = true;
    }
    return &fallback;
}

int combat_attack(Entity *attacker, Entity *defender, CombatResult *result) {
    memset(result, 0, sizeof(CombatResult));
    RNGState *rng = get_rng();

    int atk = combat_get_attack_rating(attacker);
    int def = combat_get_defense(defender);

    result->hit = combat_check_hit(atk, def, rng);
    if (!result->hit) return 0;

    result->critical = combat_check_critical(attacker->luck, rng);

    int dmg = combat_calculate_damage(atk, def,
        attacker->damage_dice,
        attacker->damage_sides,
        attacker->damage_bonus, rng);

    if (result->critical) dmg *= 2;

    result->total_damage = dmg;
    result->dtype = DT_MELEE;

    combat_apply_damage(defender, dmg, DT_MELEE);
    return 1;
}

int combat_ranged_attack(Entity *attacker, Entity *defender, int range, CombatResult *result) {
    memset(result, 0, sizeof(CombatResult));
    RNGState *rng = get_rng();

    int dx = defender->x - attacker->x;
    int dy = defender->y - attacker->y;
    int dist = dx * dx + dy * dy;
    int max_dist = range * range;
    if (dist > max_dist) return -1;

    int atk = combat_get_attack_rating(attacker) - 2;
    int def = combat_get_defense(defender);

    result->hit = combat_check_hit(atk, def, rng);
    if (!result->hit) return 0;

    int dmg = rng_dice(rng, 6) + attacker->damage_bonus;
    if (dmg < 1) dmg = 1;

    result->total_damage = dmg;
    result->dtype = DT_RANGED;

    combat_apply_damage(defender, dmg, DT_RANGED);
    return 1;
}

int combat_calculate_damage(int attack, int defense, int dice, int sides, int bonus, RNGState *rng) {
    int raw = 0;
    for (int i = 0; i < dice; i++)
        raw += rng_dice(rng, sides);
    raw += bonus;
    int reduction = defense / 3;
    int final = raw - reduction;
    if (final < 1) final = 1;
    return final;
}

bool combat_check_hit(int attack, int defense, RNGState *rng) {
    int chance = 50 + (attack - defense) * 5;
    if (chance < 10) chance = 10;
    if (chance > 90) chance = 90;
    return rng_range(rng, 0, 100) < chance;
}

bool combat_check_critical(int luck, RNGState *rng) {
    int chance = 5 + luck / 5;
    if (chance > 25) chance = 25;
    return rng_range(rng, 0, 100) < chance;
}

void combat_apply_damage(Entity *target, int amount, DamageType dtype) {
    entity_take_damage(target, amount, dtype);
}

int combat_get_defense(const Entity *e) {
    return e->defense + e->armor + e->dexterity / 5;
}

int combat_get_attack_rating(const Entity *e) {
    return e->attack + e->strength / 5;
}
