#include "engine/equipment.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

const char *equipment_slot_name(int slot) {
    switch (slot) {
        case SLOT_HEAD: return "head";
        case SLOT_CHEST: return "chest";
        case SLOT_LEGS: return "legs";
        case SLOT_FEET: return "feet";
        case SLOT_WEAPON: return "weapon hand";
        case SLOT_SHIELD: return "off hand";
        case SLOT_HANDS: return "hands";
        case SLOT_RING1: return "ring finger";
        case SLOT_RING2: return "ring finger 2";
        default: return "unknown";
    }
}

const char *equipment_quality_name(int quality) {
    switch (quality) {
        case 0: return "crude";
        case 1: return "common";
        case 2: return "fine";
        case 3: return "superior";
        case 4: return "excellent";
        case 5: return "masterwork";
        case 6: return "legendary";
        default: return "common";
    }
}

int equipment_roll_quality(RNGState *rng, int depth) {
    int roll = rng_range(rng, 0, 100);
    roll += depth * 2;
    if (roll >= 98) return 6;
    if (roll >= 90) return 5;
    if (roll >= 75) return 4;
    if (roll >= 55) return 3;
    if (roll >= 30) return 2;
    if (roll >= 10) return 1;
    return 0;
}

const char *ring_affix_name(RingAffix affix) {
    switch (affix) {
        case RAF_STRENGTH: return "of Strength";
        case RAF_DEXTERITY: return "of Dexterity";
        case RAF_INTELLIGENCE: return "of Intelligence";
        case RAF_ATTACK: return "of Power";
        case RAF_DEFENSE: return "of Protection";
        case RAF_ARMOR: return "of Hardening";
        case RAF_LUCK: return "of Fortune";
        case RAF_HEALTH: return "of Vitality";
        case RAF_MANA: return "of Sorcery";
        case RAF_REGENERATION: return "of Renewal";
        default: return "";
    }
}

static int ring_affix_value(RingAffix affix, int quality) {
    return (quality + 1) * (affix + 1);
}

int equipment_create_ring(Item *it, int depth, RNGState *rng) {
    item_init(it, "iron ring", '=', ITEM_RING);
    it->armor_slot = SLOT_RING1;
    
    int quality = equipment_roll_quality(rng, depth);
    RingAffix affix = (RingAffix)(rng_range(rng, RAF_STRENGTH, RAF_REGENERATION + 1));
    
    it->value = (quality + 1) * (depth + 1) * 5;
    it->stat_mods[0] = ring_affix_value(affix, quality) * 2;
    it->stat_mods[affix] = ring_affix_value(affix, quality);
    
    char buf[MAX_NAME_LEN];
    const char *qname = equipment_quality_name(quality);
    const char *aname = ring_affix_name(affix);
    snprintf(buf, sizeof(buf), "%s ring %s", qname, aname);
    strncpy(it->name, buf, MAX_NAME_LEN - 1);
    
    equipment_build_description(it);
    return quality;
}

int equipment_create_amulet(Item *it, int depth, RNGState *rng) {
    item_init(it, "bronze amulet", '"', ITEM_RING);
    it->armor_slot = SLOT_RING2;
    it->glyph = '"';
    
    int quality = equipment_roll_quality(rng, depth);
    int stat = rng_range(rng, 0, MOD_LUCK + 1);
    int bonus = (quality + 1) * (depth / 2 + 1) + rng_range(rng, 1, 5);
    
    it->value = (quality + 1) * (depth + 1) * 8;
    it->stat_mods[stat] = bonus;
    
    static const char *stat_names[] = {
        "health", "mana", "strength", "dexterity", "intelligence",
        "armor", "speed", "attack", "defense", "luck"
    };
    char buf[MAX_NAME_LEN];
    const char *qname = equipment_quality_name(quality);
    snprintf(buf, sizeof(buf), "%s amulet of %s", qname,
             stat_names[stat < 10 ? stat : 0]);
    strncpy(it->name, buf, MAX_NAME_LEN - 1);
    
    equipment_build_description(it);
    return quality;
}

int equipment_create_boots(Item *it, int depth, RNGState *rng) {
    item_init(it, "leather boots", '[', ITEM_ARMOR);
    it->armor_slot = SLOT_FEET;
    it->glyph = '[';
    
    int quality = equipment_roll_quality(rng, depth);
    it->armor_value = (quality + 1) + depth / 3;
    it->value = (quality + 1) * (depth + 1) * 4;
    
    if (rng_range(rng, 0, 100) < 30 + depth) {
        it->stat_mods[MOD_SPEED] = (quality + 1) * 2;
    }
    if (rng_range(rng, 0, 100) < 20 + depth) {
        it->stat_mods[MOD_DEXTERITY] = (quality + 1);
    }
    
    char buf[MAX_NAME_LEN];
    const char *qname = equipment_quality_name(quality);
    snprintf(buf, sizeof(buf), "%s boots", qname);
    strncpy(it->name, buf, MAX_NAME_LEN - 1);
    
    equipment_build_description(it);
    return quality;
}

int equipment_create_gloves(Item *it, int depth, RNGState *rng) {
    item_init(it, "cloth gloves", '[', ITEM_ARMOR);
    it->armor_slot = SLOT_HANDS;
    it->glyph = '[';
    
    int quality = equipment_roll_quality(rng, depth);
    it->armor_value = (quality + 1) / 2 + depth / 4;
    it->value = (quality + 1) * (depth + 1) * 3;
    
    if (rng_range(rng, 0, 100) < 40 + depth) {
        it->stat_mods[MOD_ATTACK] = (quality + 1);
    }
    if (rng_range(rng, 0, 100) < 25 + depth) {
        it->stat_mods[MOD_DEXTERITY] = (quality + 1);
    }
    
    char buf[MAX_NAME_LEN];
    const char *qname = equipment_quality_name(quality);
    snprintf(buf, sizeof(buf), "%s gloves", qname);
    strncpy(it->name, buf, MAX_NAME_LEN - 1);
    
    equipment_build_description(it);
    return quality;
}

int equipment_create_helmet(Item *it, int depth, RNGState *rng) {
    item_init(it, "cloth cap", '[', ITEM_ARMOR);
    it->armor_slot = SLOT_HEAD;
    it->glyph = '[';
    
    int quality = equipment_roll_quality(rng, depth);
    it->armor_value = (quality + 1) + depth / 3;
    it->value = (quality + 1) * (depth + 1) * 5;
    
    if (rng_range(rng, 0, 100) < 30 + depth) {
        it->stat_mods[MOD_INTELLIGENCE] = (quality + 1);
    }
    
    char buf[MAX_NAME_LEN];
    const char *qname = equipment_quality_name(quality);
    snprintf(buf, sizeof(buf), "%s helm", qname);
    strncpy(it->name, buf, MAX_NAME_LEN - 1);
    
    equipment_build_description(it);
    return quality;
}

int equipment_create_shield(Item *it, int depth, RNGState *rng) {
    item_init(it, "wooden shield", '[', ITEM_ARMOR);
    it->armor_slot = SLOT_SHIELD;
    it->glyph = '[';
    
    int quality = equipment_roll_quality(rng, depth);
    it->armor_value = (quality + 1) * 2 + depth / 2;
    it->value = (quality + 1) * (depth + 1) * 6;
    
    if (rng_range(rng, 0, 100) < 35 + depth) {
        it->stat_mods[MOD_DEFENSE] = (quality + 1);
    }
    
    char buf[MAX_NAME_LEN];
    const char *qname = equipment_quality_name(quality);
    snprintf(buf, sizeof(buf), "%s shield", qname);
    strncpy(it->name, buf, MAX_NAME_LEN - 1);
    
    equipment_build_description(it);
    return quality;
}

int equipment_generate_random(Item *it, int depth, RNGState *rng) {
    int roll = rng_range(rng, 0, 100);
    if (roll < 25) return equipment_create_ring(it, depth, rng);
    if (roll < 40) return equipment_create_amulet(it, depth, rng);
    if (roll < 55) return equipment_create_boots(it, depth, rng);
    if (roll < 70) return equipment_create_gloves(it, depth, rng);
    if (roll < 85) return equipment_create_helmet(it, depth, rng);
    return equipment_create_shield(it, depth, rng);
}

void equipment_apply_stats(Entity *e, const Item *it, bool equipped) {
    if (!e || !it) return;
    int sign = equipped ? 1 : -1;
    
    for (int i = 0; i <= MOD_LUCK; i++) {
        if (it->stat_mods[i] != 0) {
            switch ((StatModType)i) {
                case MOD_HEALTH: e->max_health += it->stat_mods[i] * sign; break;
                case MOD_MANA: e->max_mana += it->stat_mods[i] * sign; break;
                case MOD_STRENGTH: e->strength += it->stat_mods[i] * sign; break;
                case MOD_DEXTERITY: e->dexterity += it->stat_mods[i] * sign; break;
                case MOD_INTELLIGENCE: e->intelligence += it->stat_mods[i] * sign; break;
                case MOD_ARMOR: e->armor += it->stat_mods[i] * sign; break;
                case MOD_SPEED: e->speed += it->stat_mods[i] * sign; break;
                case MOD_ATTACK: e->attack += it->stat_mods[i] * sign; break;
                case MOD_DEFENSE: e->defense += it->stat_mods[i] * sign; break;
                case MOD_LUCK: e->luck += it->stat_mods[i] * sign; break;
            }
        }
    }
    if (it->armor_value > 0) {
        e->armor += it->armor_value * sign;
    }
}

void equipment_build_description(Item *it) {
    char desc[256] = {0};
    char temp[64];
    int pos = 0;
    
    if (it->armor_value > 0) {
        pos += snprintf(desc + pos, sizeof(desc) - pos,
                        "Armor: %d. ", it->armor_value);
    }
    for (int i = 0; i <= MOD_LUCK; i++) {
        if (it->stat_mods[i] != 0) {
            static const char *snames[] = {
                "HP", "MP", "Str", "Dex", "Int",
                "Armor", "Spd", "Atk", "Def", "Luck"
            };
            pos += snprintf(desc + pos, sizeof(desc) - pos,
                            "%s%+d ", snames[i < 10 ? i : 0], it->stat_mods[i]);
        }
    }
    
    strncpy(it->description, desc, 255);
}

bool equipment_is_ring(const Item *it) {
    return it && it->type == ITEM_RING && it->armor_slot == SLOT_RING1;
}

bool equipment_is_amulet(const Item *it) {
    return it && it->type == ITEM_RING && it->armor_slot == SLOT_RING2;
}

bool equipment_is_boots(const Item *it) {
    return it && it->type == ITEM_ARMOR && it->armor_slot == SLOT_FEET;
}

bool equipment_is_gloves(const Item *it) {
    return it && it->type == ITEM_ARMOR && it->armor_slot == SLOT_HANDS;
}

bool equipment_is_helmet(const Item *it) {
    return it && it->type == ITEM_ARMOR && it->armor_slot == SLOT_HEAD;
}

bool equipment_is_shield(const Item *it) {
    return it && it->type == ITEM_ARMOR && it->armor_slot == SLOT_SHIELD;
}

/* === Item Set System === */

typedef struct {
    const char *name;
    int pieces;                    /* Number of pieces in set */
    int slots[6];                  /* Which slots the set fills */
    int bonus_per_piece[4];        /* Stat bonuses for each step (2,3,4,5+ pieces) */
    StatModType bonus_type;        /* Which stat gets boosted */
    const char *description;
} ItemSet;

static const ItemSet item_sets[] = {
    {
        "Knight's Valor", 3,
        {SLOT_HEAD, SLOT_CHEST, SLOT_WEAPON},
        {3, 5, 8}, MOD_ARMOR,
        "A set of knightly gear that grants additional armor as more pieces are worn."
    },
    {
        "Shadow Stalker", 3,
        {SLOT_FEET, SLOT_HANDS, SLOT_WEAPON},
        {2, 4, 7}, MOD_DEXTERITY,
        "Shadowy gear that enhances agility and stealth with each piece."
    },
    {
        "Arcane Sage", 3,
        {SLOT_HEAD, SLOT_RING1, SLOT_RING2},
        {4, 7, 10}, MOD_INTELLIGENCE,
        "Arcane artifacts that boost magical power when worn together."
    },
    {
        "Guardian's Bulwark", 2,
        {SLOT_CHEST, SLOT_SHIELD},
        {4, 8}, MOD_DEFENSE,
        "A defensive combination that greatly increases protection."
    },
    {
        "Berserker's Fury", 2,
        {SLOT_HANDS, SLOT_RING1},
        {3, 6}, MOD_ATTACK,
        "Fierce equipment that fuels rage and increases attack power."
    },
    {
        "Dragon Lord", 4,
        {SLOT_HEAD, SLOT_CHEST, SLOT_LEGS, SLOT_RING1},
        {3, 5, 8, 12}, MOD_STRENGTH,
        "Legendary dragon-themed gear granting immense strength."
    },
    {
        "Frost Walker", 2,
        {SLOT_FEET, SLOT_LEGS},
        {3, 5}, MOD_SPEED,
        "Lightweight cold-weather gear that improves movement speed."
    },
    {
        "Holy Champion", 3,
        {SLOT_HEAD, SLOT_SHIELD, SLOT_RING2},
        {3, 6, 10}, MOD_HEALTH,
        "Blessed equipment that increases the wearer's vitality."
    },
};

static const int item_set_count = sizeof(item_sets) / sizeof(item_sets[0]);

int equipment_get_set_count(void) {
    return item_set_count;
}

const char *equipment_get_set_name(int set_idx) {
    if (set_idx < 0 || set_idx >= item_set_count) return NULL;
    return item_sets[set_idx].name;
}

int equipment_get_set_pieces(int set_idx) {
    if (set_idx < 0 || set_idx >= item_set_count) return 0;
    return item_sets[set_idx].pieces;
}

int equipment_count_set_pieces_equipped(int set_idx, const Entity *e) {
    if (set_idx < 0 || set_idx >= item_set_count || !e) return 0;
    int count = 0;
    const ItemSet *set = &item_sets[set_idx];
    for (int s = 0; s < set->pieces; s++) {
        int slot = set->slots[s];
        int item_idx = e->equipment[slot];
        if (item_idx >= 0 && item_idx < e->inventory_count) {
            Item *it = (Item *)e->inventory[item_idx];
            if (it && (it->armor_slot == slot || item_get_equip_slot(it) == slot)) {
                count++;
            }
        }
    }
    return count;
}

int equipment_get_set_bonus(int set_idx, int pieces_equipped) {
    if (set_idx < 0 || set_idx >= item_set_count) return 0;
    const ItemSet *set = &item_sets[set_idx];
    if (pieces_equipped <= 0) return 0;
    if (pieces_equipped > set->pieces) pieces_equipped = set->pieces;
    int idx = pieces_equipped - 2;
    if (idx < 0) idx = 0;
    if (idx >= 4) idx = 3;
    return set->bonus_per_piece[idx];
}

StatModType equipment_get_set_bonus_type(int set_idx) {
    if (set_idx < 0 || set_idx >= item_set_count) return MOD_HEALTH;
    return item_sets[set_idx].bonus_type;
}

int equipment_find_set_for_item(const Item *it) {
    if (!it) return -1;
    int slot = item_get_equip_slot(it);
    if (slot < 0) return -1;
    for (int s = 0; s < item_set_count; s++) {
        for (int p = 0; p < item_sets[s].pieces; p++) {
            if (item_sets[s].slots[p] == slot) {
                /* Check if item name contains set keywords */
                const char *set_name = item_sets[s].name;
                if (strstr(it->name, "knight") && strstr(set_name, "Knight")) return s;
                if (strstr(it->name, "shadow") && strstr(set_name, "Shadow")) return s;
                if (strstr(it->name, "arcane") && strstr(set_name, "Arcane")) return s;
                if (strstr(it->name, "dragon") && strstr(set_name, "Dragon")) return s;
                if (strstr(it->name, "elven") && strstr(set_name, "Dragon")) continue;
                if (strstr(it->name, "iron") || strstr(it->name, "steel")) return 0;
                if (strstr(it->name, "leather") || strstr(it->name, "boots")) return 1;
                if (strstr(it->name, "crystal") || strstr(it->name, "sapphire")) return 2;
                return s;
            }
        }
    }
    return -1;
}
