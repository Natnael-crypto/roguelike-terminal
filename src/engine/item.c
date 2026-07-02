#include "engine/item.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static uint32_t next_item_id = 1;

int item_init(Item *it, const char *name, char glyph, ItemType type) {
    memset(it, 0, sizeof(Item));
    it->id = next_item_id++;
    strncpy(it->name, name, MAX_NAME_LEN - 1);
    it->glyph = glyph;
    it->type = type;
    it->x = -1;
    it->y = -1;
    it->value = 0;
    it->weight = 1;
    it->stack_count = 1;
    it->max_stack = 1;
    it->identified = true;
    it->cursed = false;
    it->equipped = false;
    it->heal_amount = 0;
    it->mana_restore = 0;
    it->armor_value = 0;
    it->damage_dice = 0;
    it->damage_sides = 0;
    it->damage_bonus = 0;
    it->flags = 0;
    memset(it->stat_mods, 0, sizeof(it->stat_mods));
    it->description[0] = '\0';
    return 0;
}

Item *item_create(const char *name, char glyph, ItemType type) {
    Item *it = (Item *)malloc(sizeof(Item));
    if (!it) return NULL;
    item_init(it, name, glyph, type);
    return it;
}

Item *item_clone(const Item *src) {
    if (!src) return NULL;
    Item *it = item_create(src->name, src->glyph, src->type);
    if (!it) return NULL;
    memcpy(it, src, sizeof(Item));
    it->id = next_item_id++;
    return it;
}

void item_destroy(Item *it) {
    (void)it;
}

void item_free(Item *it) {
    if (it) {
        item_destroy(it);
        free(it);
    }
}

bool item_is_equippable(const Item *it) {
    return it->type == ITEM_WEAPON || it->type == ITEM_ARMOR ||
           it->type == ITEM_RING;
}

bool item_is_usable(const Item *it) {
    return it->type == ITEM_POTION || it->type == ITEM_SCROLL ||
           it->type == ITEM_FOOD || it->type == ITEM_WAND;
}

bool item_is_stackable(const Item *it) {
    return it->type == ITEM_GOLD || it->type == ITEM_GEM;
}

int item_get_equip_slot(const Item *it) {
    switch (it->type) {
        case ITEM_WEAPON: return SLOT_WEAPON;
        case ITEM_ARMOR:
            if (it->armor_slot >= 0 && it->armor_slot < SLOT_COUNT)
                return it->armor_slot;
            return SLOT_CHEST;
        case ITEM_RING: return SLOT_RING1;
        default: return -1;
    }
}

const char *item_type_name(ItemType type) {
    switch (type) {
        case ITEM_WEAPON: return "weapon";
        case ITEM_ARMOR: return "armor";
        case ITEM_POTION: return "potion";
        case ITEM_SCROLL: return "scroll";
        case ITEM_FOOD: return "food";
        case ITEM_KEY: return "key";
        case ITEM_GOLD: return "gold";
        case ITEM_RING: return "ring";
        case ITEM_WAND: return "wand";
        case ITEM_GEM: return "gem";
        default: return "unknown";
    }
}

Item *item_generate_for_depth(int depth, RNGState *rng) {
    int roll = rng_range(rng, 0, 100);
    Item *it = NULL;

    if (roll < 30) {
        /* Potions */
        static const char *potions[] = {
            "health potion", "mana potion", "greater health potion",
            "antidote", "speed potion", "strength potion"
        };
        int idx = rng_range(rng, 0, 6);
        it = item_create(potions[idx], '!', ITEM_POTION);
        if (it) {
            it->heal_amount = 10 + depth * 3 + rng_range(rng, 0, 10);
            if (idx == 1) it->mana_restore = 10 + depth * 2;
            if (idx == 2) it->heal_amount = 30 + depth * 5;
            it->value = 10 + depth * 2;
        }
    } else if (roll < 50) {
        /* Scrolls */
        static const char *scrolls[] = {
            "scroll of identify", "scroll of teleportation",
            "scroll of light", "scroll of protection",
            "scroll of magic mapping"
        };
        int idx = rng_range(rng, 0, 5);
        it = item_create(scrolls[idx], '?', ITEM_SCROLL);
        if (it) it->value = 15 + depth * 3;
    } else if (roll < 65) {
        /* Weapons */
        static const char *wnames[] = {
            "short sword", "long sword", "battle axe",
            "mace", "spear", "war hammer"
        };
        char glyphs[] = {'/', '/', '/', '/', '/', '/'};
        int idx = rng_range(rng, 0, 6);
        it = item_create(wnames[idx], glyphs[idx], ITEM_WEAPON);
        if (it) {
            it->damage_dice = 1 + depth / 5;
            it->damage_sides = 4 + depth / 2 + idx;
            it->damage_bonus = depth / 3;
            it->value = 20 + depth * 5;
        }
    } else if (roll < 80) {
        /* Armor */
        static const char *anames[] = {
            "leather armor", "chain mail", "scale mail",
            "plate armor", "studded leather", "hide armor"
        };
        int idx = rng_range(rng, 0, 6);
        it = item_create(anames[idx], '[', ITEM_ARMOR);
        if (it) {
            it->armor_value = 1 + depth / 3 + idx / 2;
            it->armor_slot = SLOT_CHEST;
            it->value = 15 + depth * 4;
        }
    } else if (roll < 90) {
        /* Gold */
        int amount = rng_range(rng, 5, 50) + depth * 5;
        it = item_create("gold coins", '$', ITEM_GOLD);
        if (it) {
            it->value = amount;
            it->stack_count = 1;
        }
    } else {
        /* Food */
        static const char *foods[] = {
            "bread", "meat", "apple", "mushroom", "cheese"
        };
        int idx = rng_range(rng, 0, 5);
        it = item_create(foods[idx], '%', ITEM_FOOD);
        if (it) {
            it->heal_amount = 5 + rng_range(rng, 0, 10);
            it->value = 3;
        }
    }

    return it;
}

void item_set_description(Item *it, const char *desc) {
    if (!it || !desc) return;
    strncpy(it->description, desc, 255);
}

bool item_has_flag(const Item *it, uint32_t flag) {
    return it && (it->flags & flag) != 0;
}

void item_set_flag(Item *it, uint32_t flag) {
    if (it) it->flags |= flag;
}

void item_clear_flag(Item *it, uint32_t flag) {
    if (it) it->flags &= ~flag;
}

const char *item_quality_label(int value) {
    if (value >= 1000) return "legendary";
    if (value >= 500) return "excellent";
    if (value >= 200) return "fine";
    if (value >= 50) return "common";
    return "cheap";
}

int item_compare_quality(const Item *a, const Item *b) {
    if (!a && !b) return 0;
    if (!a) return -1;
    if (!b) return 1;
    if (a->value > b->value) return 1;
    if (a->value < b->value) return -1;
    return 0;
}
