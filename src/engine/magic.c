#include "engine/magic.h"
#include "engine/combat.h"
#include "engine/map.h"
#include "engine/dungeon.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static const SpellType SPELL_SCHOOL_OFFENSE[] = {SPT_MAGIC_MISSILE, SPT_FIREBALL, SPT_LIGHTNING, SPT_ICE_SHARD, SPT_CHAIN_LIGHTNING, SPT_DRAIN_LIFE};
static const SpellType SPELL_SCHOOL_DEFENSE[] = {SPT_SHIELD, SPT_BLINK, SPT_INVISIBILITY, SPT_STONE_SKIN, SPT_DISPEL_MAGIC};
static const SpellType SPELL_SCHOOL_HEALING[] = {SPT_HEAL, SPT_PURIFY, SPT_REGENERATION, SPT_LIGHT};
static const SpellType SPELL_SCHOOL_UTILITY[] = {SPT_TELEPORT, SPT_DETECT, SPT_REVEAL, SPT_SUMMON_ELEMENTAL, SPT_HASTE};
static const SpellType SPELL_SCHOOL_DEBUFF[] = {SPT_CONFUSE, SPT_SLOW, SPT_FEAR, SPT_POISON_CLOUD, SPT_BERSERK};

const char *magic_school_name(SpellSchool school) {
    switch (school) {
        case SC_OFFENSE: return "Offense";
        case SC_DEFENSE: return "Defense";
        case SC_HEALING: return "Healing";
        case SC_UTILITY: return "Utility";
        case SC_DEBUFF: return "Debuff";
        default: return "Unknown";
    }
}

int magic_get_school_spells(SpellSchool school, SpellType *out, int max_out) {
    const SpellType *list = NULL;
    int count = 0;
    switch (school) {
        case SC_OFFENSE:
            list = SPELL_SCHOOL_OFFENSE;
            count = sizeof(SPELL_SCHOOL_OFFENSE) / sizeof(SpellType);
            break;
        case SC_DEFENSE:
            list = SPELL_SCHOOL_DEFENSE;
            count = sizeof(SPELL_SCHOOL_DEFENSE) / sizeof(SpellType);
            break;
        case SC_HEALING:
            list = SPELL_SCHOOL_HEALING;
            count = sizeof(SPELL_SCHOOL_HEALING) / sizeof(SpellType);
            break;
        case SC_UTILITY:
            list = SPELL_SCHOOL_UTILITY;
            count = sizeof(SPELL_SCHOOL_UTILITY) / sizeof(SpellType);
            break;
        case SC_DEBUFF:
            list = SPELL_SCHOOL_DEBUFF;
            count = sizeof(SPELL_SCHOOL_DEBUFF) / sizeof(SpellType);
            break;
    }
    if (list && out) {
        int n = count < max_out ? count : max_out;
        for (int i = 0; i < n; i++) out[i] = list[i];
    }
    return count;
}

SpellSchool magic_get_spell_school(SpellType type) {
    for (int s = SC_OFFENSE; s <= SC_DEBUFF; s++) {
        SpellType spells[8];
        int n = magic_get_school_spells((SpellSchool)s, spells, 8);
        for (int i = 0; i < n; i++) {
            if (spells[i] == type) return (SpellSchool)s;
        }
    }
    return SC_UTILITY;
}

void magic_init_spells(Spell *spells, int *count) {
    *count = 23;
    Spell s[23];
    memset(s, 0, sizeof(s));

    s[SPT_MAGIC_MISSILE] = (Spell){SPT_MAGIC_MISSILE, "Magic Missile", 3, 1, SPELL_TARGET_ENEMY, 6, 1, 6, 8, 0, 0, 0, "A bolt of pure energy strikes a single target."};
    s[SPT_HEAL] = (Spell){SPT_HEAL, "Heal", 5, 1, SPELL_TARGET_SELF, 10, 2, 6, 0, 0, 0, 0, "Restores health to the caster."};
    s[SPT_FIREBALL] = (Spell){SPT_FIREBALL, "Fireball", 8, 3, SPELL_TARGET_AREA, 15, 3, 6, 6, 2, 0, 0, "A blazing sphere that damages all in the area."};
    s[SPT_LIGHTNING] = (Spell){SPT_LIGHTNING, "Lightning Bolt", 7, 2, SPELL_TARGET_ENEMY, 20, 2, 8, 10, 0, 0, 0, "A powerful bolt of lightning that arcs to the target."};
    s[SPT_TELEPORT] = (Spell){SPT_TELEPORT, "Teleport", 8, 3, SPELL_TARGET_SELF, 0, 0, 0, 0, 0, 0, 0, "Teleports the caster to a random location on the current level."};
    s[SPT_CONFUSE] = (Spell){SPT_CONFUSE, "Confusion", 4, 2, SPELL_TARGET_ENEMY, 0, 0, 0, 6, 0, 10, 1, "Confuses the target, causing it to move randomly."};
    s[SPT_SLOW] = (Spell){SPT_SLOW, "Slow", 3, 1, SPELL_TARGET_ENEMY, 0, 0, 0, 8, 0, 8, 2, "Slows the target's movement."};
    s[SPT_HASTE] = (Spell){SPT_HASTE, "Haste", 6, 3, SPELL_TARGET_SELF, 0, 0, 0, 0, 0, 8, 3, "Hastens the caster, allowing faster movement."};
    s[SPT_SHIELD] = (Spell){SPT_SHIELD, "Shield", 5, 2, SPELL_TARGET_SELF, 0, 0, 0, 0, 0, 15, 5, "Creates a magical barrier that increases armor."};
    s[SPT_BLINK] = (Spell){SPT_BLINK, "Blink", 3, 1, SPELL_TARGET_SELF, 0, 0, 0, 5, 0, 0, 0, "Short-range teleport to a visible location."};
    s[SPT_DETECT] = (Spell){SPT_DETECT, "Detect Monsters", 4, 1, SPELL_TARGET_SELF, 0, 0, 0, 0, 0, 20, 1, "Reveals all monsters on the current level."};
    s[SPT_FEAR] = (Spell){SPT_FEAR, "Fear", 5, 2, SPELL_TARGET_ENEMY, 0, 0, 0, 8, 0, 8, 4, "Fills the target with terror, causing it to flee."};
    s[SPT_POISON_CLOUD] = (Spell){SPT_POISON_CLOUD, "Poison Cloud", 7, 3, SPELL_TARGET_AREA, 0, 0, 0, 5, 2, 12, 5, "Creates a cloud of poison that damages over time."};
    s[SPT_BERSERK] = (Spell){SPT_BERSERK, "Berserk", 6, 3, SPELL_TARGET_SELF, 0, 0, 0, 0, 0, 10, 6, "Increases attack power but decreases defense."};
    s[SPT_REVEAL] = (Spell){SPT_REVEAL, "Reveal Map", 5, 2, SPELL_TARGET_SELF, 0, 0, 0, 0, 0, 0, 0, "Reveals the entire map layout."};

    /* 8 new spells: Ice Shard, Chain Lightning, Invisibility, Stone Skin, Summon Elemental, Drain Life, Light, Dispel Magic */
    s[SPT_ICE_SHARD] = (Spell){SPT_ICE_SHARD, "Ice Shard", 4, 1, SPELL_TARGET_ENEMY, 8, 1, 8, 7, 0, 0, 0, "A shard of ice pierces the target, potentially slowing it."};
    s[SPT_CHAIN_LIGHTNING] = (Spell){SPT_CHAIN_LIGHTNING, "Chain Lightning", 10, 4, SPELL_TARGET_ENEMY, 20, 3, 6, 8, 3, 0, 0, "Lightning arcs from the target to nearby enemies."};
    s[SPT_INVISIBILITY] = (Spell){SPT_INVISIBILITY, "Invisibility", 6, 2, SPELL_TARGET_SELF, 0, 0, 0, 0, 0, 15, 0, "Makes the caster invisible to enemies for a time."};
    s[SPT_STONE_SKIN] = (Spell){SPT_STONE_SKIN, "Stone Skin", 7, 3, SPELL_TARGET_SELF, 0, 0, 0, 0, 0, 20, 8, "Hardens the caster's skin, granting significant armor."};
    s[SPT_SUMMON_ELEMENTAL] = (Spell){SPT_SUMMON_ELEMENTAL, "Summon Elemental", 12, 5, SPELL_TARGET_SELF, 0, 0, 0, 0, 0, 0, 0, "Summons an elemental ally to fight for the caster."};
    s[SPT_DRAIN_LIFE] = (Spell){SPT_DRAIN_LIFE, "Drain Life", 6, 2, SPELL_TARGET_ENEMY, 12, 2, 6, 6, 0, 0, 0, "Drains life from the target, healing the caster."};
    s[SPT_LIGHT] = (Spell){SPT_LIGHT, "Light", 2, 1, SPELL_TARGET_SELF, 0, 0, 0, 0, 0, 0, 0, "Creates a bright light, increasing visibility."};
    s[SPT_DISPEL_MAGIC] = (Spell){SPT_DISPEL_MAGIC, "Dispel Magic", 5, 2, SPELL_TARGET_SELF, 0, 0, 0, 0, 0, 0, 0, "Removes negative magical effects from the caster."};

    memcpy(spells, s, sizeof(s));
}

void magic_init_spell_scrolls(Spell *scrolls, int *count) {
    *count = 8;
    Spell s[8];
    memset(s, 0, sizeof(s));

    s[0] = (Spell){SPT_FIREBALL, "Scroll of Fireball", 0, 0, SPELL_TARGET_AREA, 15, 3, 6, 6, 2, 0, 0, "A scroll containing the Fireball spell."};
    s[1] = (Spell){SPT_TELEPORT, "Scroll of Teleportation", 0, 0, SPELL_TARGET_SELF, 0, 0, 0, 0, 0, 0, 0, "A scroll of teleportation."};
    s[2] = (Spell){SPT_LIGHTNING, "Scroll of Lightning", 0, 0, SPELL_TARGET_ENEMY, 20, 2, 8, 10, 0, 0, 0, "A scroll containing a Lightning Bolt."};
    s[3] = (Spell){SPT_HEAL, "Scroll of Healing", 0, 0, SPELL_TARGET_SELF, 25, 0, 0, 0, 0, 0, 0, "A scroll of powerful healing."};
    s[4] = (Spell){SPT_ICE_SHARD, "Scroll of Ice Shard", 0, 0, SPELL_TARGET_ENEMY, 8, 1, 8, 7, 0, 0, 0, "A scroll of Ice Shard."};
    s[5] = (Spell){SPT_CHAIN_LIGHTNING, "Scroll of Chain Lightning", 0, 0, SPELL_TARGET_ENEMY, 20, 3, 6, 8, 3, 0, 0, "A scroll of Chain Lightning."};
    s[6] = (Spell){SPT_CONFUSE, "Scroll of Confusion", 0, 0, SPELL_TARGET_ENEMY, 0, 0, 0, 6, 0, 10, 1, "A scroll of Confusion."};
    s[7] = (Spell){SPT_REVEAL, "Scroll of Mapping", 0, 0, SPELL_TARGET_SELF, 0, 0, 0, 0, 0, 0, 0, "A scroll that reveals the map."};

    memcpy(scrolls, s, sizeof(s));
}

int magic_find_spell(const Spell *spells, int count, SpellType type) {
    for (int i = 0; i < count; i++)
        if (spells[i].type == type) return i;
    return -1;
}

bool magic_target_in_range(const Entity *caster, int tx, int ty, int range) {
    if (!caster) return false;
    int dx = tx - caster->x;
    int dy = ty - caster->y;
    return (dx * dx + dy * dy) <= range * range;
}

int magic_get_target_type(SpellType type) {
    switch (type) {
        case SPT_HEAL: case SPT_TELEPORT: case SPT_HASTE:
        case SPT_SHIELD: case SPT_BLINK: case SPT_DETECT:
        case SPT_BERSERK: case SPT_REVEAL:
            return SPELL_TARGET_SELF;
        case SPT_FIREBALL: case SPT_POISON_CLOUD:
            return SPELL_TARGET_AREA;
        default:
            return SPELL_TARGET_ENEMY;
    }
}

const char *magic_spell_name(SpellType type) {
    switch (type) {
        case SPT_FIREBALL: return "Fireball";
        case SPT_HEAL: return "Heal";
        case SPT_LIGHTNING: return "Lightning Bolt";
        case SPT_TELEPORT: return "Teleport";
        case SPT_CONFUSE: return "Confusion";
        case SPT_SLOW: return "Slow";
        case SPT_HASTE: return "Haste";
        case SPT_SHIELD: return "Shield";
        case SPT_MAGIC_MISSILE: return "Magic Missile";
        case SPT_BLINK: return "Blink";
        case SPT_DETECT: return "Detect Monsters";
        case SPT_FEAR: return "Fear";
        case SPT_POISON_CLOUD: return "Poison Cloud";
        case SPT_BERSERK: return "Berserk";
        case SPT_REVEAL: return "Reveal Map";
        default: return "Unknown";
    }
}

void magic_apply_effect(Entity *target, SpellType type, int power, int turns) {
    switch (type) {
        case SPT_CONFUSE:
            target->stun_turns += turns;
            break;
        case SPT_SLOW:
            entity_apply_status(target, 0, 0, turns, 0);
            break;
        case SPT_HASTE:
            entity_apply_status(target, 0, 0, 0, turns);
            break;
        case SPT_SHIELD:
            target->armor += power;
            break;
        case SPT_BERSERK:
            target->attack += 5;
            target->defense -= 3;
            if (target->defense < 0) target->defense = 0;
            break;
        case SPT_FEAR:
            target->stun_turns += turns;
            break;
        case SPT_POISON_CLOUD:
            entity_apply_status(target, turns, 0, 0, 0);
            break;
        default:
            break;
    }
}

void magic_area_effect(World *w, int cx, int cy, int radius, int damage, SpellType type) {
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            int tx = cx + dx;
            int ty = cy + dy;
            if (dx * dx + dy * dy > radius * radius) continue;
            if (!w->map || !map_in_bounds(w->map, tx, ty)) continue;

            Entity *target = world_get_entity_at(w, tx, ty);
            if (target && target->alive) {
                if (damage > 0) {
                    entity_take_damage(target, damage, DT_MAGIC);
                }
                if (type == SPT_FIREBALL) {
                    entity_apply_status(target, 0, 3, 0, 0);
                }
                if (type == SPT_POISON_CLOUD) {
                    entity_apply_status(target, 5, 0, 0, 0);
                }
            }
        }
    }
}

int magic_cast_spell(Entity *caster, Spell *spell, World *w, RNGState *rng) {
    if (!caster || !spell || !w) return -1;
    if (caster->mana < spell->mana_cost) return -1;
    caster->mana -= spell->mana_cost;

    switch (spell->type) {
        case SPT_MAGIC_MISSILE: {
            if (caster->target && caster->target->alive) {
                CombatResult cr;
                cr.total_damage = rng_roll(rng, spell->damage_dice, spell->damage_sides, 0);
                cr.hit = true;
                cr.dtype = DT_MAGIC;
                entity_take_damage(caster->target, cr.total_damage, DT_MAGIC);
                return cr.total_damage;
            }
            return -1;
        }
        case SPT_HEAL: {
            int amt = rng_roll(rng, spell->damage_dice, spell->damage_sides, 0);
            entity_heal(caster, amt);
            return amt;
        }
        case SPT_FIREBALL: {
            if (caster->target && caster->target->alive) {
                int dmg = rng_roll(rng, spell->damage_dice, spell->damage_sides, 0);
                magic_area_effect(w, caster->target->x, caster->target->y, spell->radius, dmg, spell->type);
                return dmg;
            }
            return -1;
        }
        case SPT_LIGHTNING: {
            if (caster->target && caster->target->alive) {
                int dmg = rng_roll(rng, spell->damage_dice, spell->damage_sides, 0);
                entity_take_damage(caster->target, dmg, DT_MAGIC);
                return dmg;
            }
            return -1;
        }
        case SPT_TELEPORT: {
            if (w->map) {
                int nx, ny;
                dungeon_find_random_floor(w->map, &nx, &ny, rng);
                caster->x = nx;
                caster->y = ny;
                return 1;
            }
            return -1;
        }
        case SPT_CONFUSE: case SPT_SLOW: case SPT_FEAR: {
            if (caster->target && caster->target->alive) {
                magic_apply_effect(caster->target, spell->type, spell->effect_power, spell->effect_turns);
                return 1;
            }
            return -1;
        }
        case SPT_HASTE: case SPT_SHIELD: case SPT_BERSERK: {
            magic_apply_effect(caster, spell->type, spell->effect_power, spell->effect_turns);
            return 1;
        }
        case SPT_BLINK: {
            if (w->map) {
                int nx = caster->x + rng_range(rng, -spell->range, spell->range + 1);
                int ny = caster->y + rng_range(rng, -spell->range, spell->range + 1);
                if (map_in_bounds(w->map, nx, ny) && map_is_walkable(w->map, nx, ny)) {
                    caster->x = nx;
                    caster->y = ny;
                    return 1;
                }
            }
            return -1;
        }
        case SPT_DETECT: {
            for (int i = 0; i < w->monster_count; i++) {
                if (w->monsters[i] && w->monsters[i]->alive) {
                    map_set_visible(w->map, w->monsters[i]->x, w->monsters[i]->y, true);
                    w->map->explored[w->monsters[i]->x][w->monsters[i]->y] = true;
                }
            }
            return 1;
        }
        case SPT_POISON_CLOUD: {
            if (caster->target && caster->target->alive) {
                magic_area_effect(w, caster->target->x, caster->target->y, spell->radius, 0, spell->type);
                return 1;
            }
            return -1;
        }
        case SPT_REVEAL: {
            if (w->map) {
                for (int y = 0; y < w->map->height; y++)
                    for (int x = 0; x < w->map->width; x++)
                        w->map->explored[x][y] = true;
            }
            return 1;
        }
        default:
            return -1;
    }
}

int magic_learn_spell(Entity *e, SpellType type) {
    (void)e;
    (void)type;
    return 0;
}

bool magic_has_spell(const Entity *e, SpellType type) {
    (void)e;
    (void)type;
    return false;
}

int magic_level_for_spell(SpellType type) {
    switch (type) {
        case SPT_MAGIC_MISSILE: case SPT_HEAL: case SPT_SLOW:
        case SPT_BLINK: case SPT_DETECT:
            return 1;
        case SPT_LIGHTNING: case SPT_CONFUSE: case SPT_SHIELD:
        case SPT_FEAR: case SPT_REVEAL:
            return 2;
        case SPT_FIREBALL: case SPT_TELEPORT: case SPT_HASTE:
        case SPT_POISON_CLOUD: case SPT_BERSERK:
            return 3;
        default:
            return 5;
    }
}

int magic_damage_for_level(SpellType type, int caster_level) {
    int base = 0;
    switch (type) {
        case SPT_MAGIC_MISSILE: base = 4 + caster_level * 2; break;
        case SPT_LIGHTNING: base = 10 + caster_level * 3; break;
        case SPT_FIREBALL: base = 8 + caster_level * 3; break;
        case SPT_ICE_SHARD: base = 6 + caster_level * 2; break;
        case SPT_CHAIN_LIGHTNING: base = 12 + caster_level * 4; break;
        case SPT_DRAIN_LIFE: base = 8 + caster_level * 3; break;
        case SPT_HEAL: base = 8 + caster_level * 2; break;
        default: base = 5; break;
    }
    return base;
}

int magic_generate_random_scroll(int depth, RNGState *rng) {
    (void)depth;
    return rng_range(rng, 0, 8);
}

int magic_count_spells_in_school(const Spell *spells, int total_count, SpellSchool school) {
    int n = 0;
    for (int i = 0; i < total_count; i++) {
        if (magic_get_spell_school(spells[i].type) == school) n++;
    }
    return n;
}

int magic_get_mana_cost(const Spell *spell, int caster_level) {
    if (!spell) return 0;
    int cost = spell->mana_cost;
    int reduction = caster_level / 3;
    if (reduction > cost / 2) reduction = cost / 2;
    return cost - reduction;
}

bool magic_can_cast(const Entity *caster, const Spell *spell) {
    if (!caster || !spell) return false;
    if (caster->mana < spell->mana_cost) return false;
    if (caster->level < spell->min_level) return false;
    return true;
}
