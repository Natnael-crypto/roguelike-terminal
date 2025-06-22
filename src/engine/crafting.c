#include "engine/crafting.h"
#include "engine/inventory.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

const char *crafting_station_name(CraftStation cs) {
    switch (cs) {
        case CS_ANVIL: return "anvil";
        case CS_CAULDRON: return "cauldron";
        case CS_ALTAR: return "altar";
        case CS_WORKBENCH: return "workbench";
        case CS_ENCHANTING_TABLE: return "enchanting table";
        case CS_FORGE: return "forge";
        case CS_ALCHEMY_BENCH: return "alchemy bench";
        default: return "unknown station";
    }
}

int crafting_init_recipes(CraftRecipe *recipes, int *count) {
    *count = 20;
    memset(recipes, 0, sizeof(CraftRecipe) * 20);

    int i = 0;

    recipes[i] = (CraftRecipe){i + 1, "wooden arrow", CS_WORKBENCH, 1, 1,
        {{ITEM_WEAPON, 0, 3, "wood"}}, {0}, 5, "Fashion three wooden shafts into arrows.", true};
    item_init(&recipes[i].result, "wooden arrow", '/', ITEM_WEAPON);
    recipes[i].result.damage_dice = 1;
    recipes[i].result.damage_sides = 4;
    recipes[i].result.stack_count = 5;
    i++;

    recipes[i] = (CraftRecipe){i + 1, "iron dagger", CS_FORGE, 2, 2,
        {{ITEM_WEAPON, 0, 2, "iron ore"}, {ITEM_WEAPON, 0, 1, "wood"}}, {0}, 1,
        "Smelt iron and shape a blade.", true};
    item_init(&recipes[i].result, "iron dagger", '/', ITEM_WEAPON);
    recipes[i].result.damage_dice = 1;
    recipes[i].result.damage_sides = 4;
    recipes[i].result.damage_bonus = 1;
    i++;

    recipes[i] = (CraftRecipe){i + 1, "health potion", CS_ALCHEMY_BENCH, 1, 3,
        {{ITEM_POTION, 0, 2, "herbs"}, {ITEM_POTION, 0, 1, "water vial"}}, {0}, 1,
        "Brew a restorative elixir from herbs.", true};
    item_init(&recipes[i].result, "health potion", '!', ITEM_POTION);
    recipes[i].result.heal_amount = 15;
    i++;

    recipes[i] = (CraftRecipe){i + 1, "leather armor", CS_WORKBENCH, 2, 2,
        {{ITEM_ARMOR, 0, 3, "leather strips"}, {ITEM_ARMOR, 0, 1, "thread"}}, {0}, 1,
        "Stitch leather strips into protective gear.", true};
    item_init(&recipes[i].result, "leather armor", '[', ITEM_ARMOR);
    recipes[i].result.armor_value = 3;
    recipes[i].result.armor_slot = SLOT_CHEST;
    i++;

    recipes[i] = (CraftRecipe){i + 1, "iron sword", CS_FORGE, 3, 3,
        {{ITEM_WEAPON, 0, 3, "iron ore"}, {ITEM_WEAPON, 0, 1, "wood"}, {ITEM_WEAPON, 0, 1, "leather strips"}}, {0}, 1,
        "A solid iron blade, well-balanced.", true};
    item_init(&recipes[i].result, "iron sword", '/', ITEM_WEAPON);
    recipes[i].result.damage_dice = 1;
    recipes[i].result.damage_sides = 8;
    recipes[i].result.damage_bonus = 2;
    i++;

    recipes[i] = (CraftRecipe){i + 1, "ring of protection", CS_ENCHANTING_TABLE, 4, 2,
        {{ITEM_RING, 0, 1, "gold ring base"}, {ITEM_GEM, 0, 1, "small ruby"}}, {0}, 1,
        "Imbue a ring with protective magic.", true};
    item_init(&recipes[i].result, "ring of protection", '=', ITEM_RING);
    recipes[i].result.armor_value = 2;
    recipes[i].result.stat_mods[MOD_ARMOR] = 3;
    i++;

    recipes[i] = (CraftRecipe){i + 1, "mana potion", CS_CAULDRON, 2, 2,
        {{ITEM_POTION, 0, 2, "mana herbs"}, {ITEM_POTION, 0, 1, "crystal dust"}}, {0}, 1,
        "Brew an energizing mana potion.", true};
    item_init(&recipes[i].result, "mana potion", '!', ITEM_POTION);
    recipes[i].result.mana_restore = 15;
    i++;

    recipes[i] = (CraftRecipe){i + 1, "steel sword", CS_FORGE, 5, 3,
        {{ITEM_WEAPON, 0, 4, "steel ingot"}, {ITEM_WEAPON, 0, 2, "wood"}, {ITEM_WEAPON, 0, 1, "leather"}}, {0}, 1,
        "A superior steel blade, masterfully crafted.", true};
    item_init(&recipes[i].result, "steel sword", '/', ITEM_WEAPON);
    recipes[i].result.damage_dice = 2;
    recipes[i].result.damage_sides = 6;
    recipes[i].result.damage_bonus = 3;
    i++;

    recipes[i] = (CraftRecipe){i + 1, "chain mail", CS_FORGE, 4, 3,
        {{ITEM_ARMOR, 0, 5, "iron links"}, {ITEM_ARMOR, 0, 2, "leather padding"}, {ITEM_ARMOR, 0, 1, "rivets"}}, {0}, 1,
        "Interlock iron links into flexible mail.", true};
    item_init(&recipes[i].result, "chain mail", '[', ITEM_ARMOR);
    recipes[i].result.armor_value = 5;
    recipes[i].result.armor_slot = SLOT_CHEST;
    i++;

    recipes[i] = (CraftRecipe){i + 1, "scroll of identify", CS_ALTAR, 3, 2,
        {{ITEM_SCROLL, 0, 1, "blank scroll"}, {ITEM_SCROLL, 0, 1, "magic ink"}}, {0}, 1,
        "Inscribe a scroll with revealing magic.", true};
    item_init(&recipes[i].result, "scroll of identify", '?', ITEM_SCROLL);
    i++;

    recipes[i] = (CraftRecipe){i + 1, "fire arrow", CS_WORKBENCH, 3, 2,
        {{ITEM_WEAPON, 0, 3, "wooden arrow"}, {ITEM_POTION, 0, 1, "oil"}}, {0}, 3,
        "Tips soaked in oil, ready to ignite.", true};
    item_init(&recipes[i].result, "fire arrow", '/', ITEM_WEAPON);
    recipes[i].result.damage_dice = 1;
    recipes[i].result.damage_sides = 6;
    recipes[i].result.damage_bonus = 1;
    recipes[i].result.stack_count = 3;
    i++;

    recipes[i] = (CraftRecipe){i + 1, "potion of strength", CS_CAULDRON, 4, 2,
        {{ITEM_POTION, 0, 2, "troll blood"}, {ITEM_POTION, 0, 1, "iron shavings"}}, {0}, 1,
        "A foul brew that grants great strength.", true};
    item_init(&recipes[i].result, "potion of strength", '!', ITEM_POTION);
    recipes[i].result.stat_mods[MOD_STRENGTH] = 3;
    i++;

    recipes[i] = (CraftRecipe){i + 1, "scale armor", CS_FORGE, 5, 3,
        {{ITEM_ARMOR, 0, 4, "dragon scale"}, {ITEM_ARMOR, 0, 2, "chain mail"}, {ITEM_ARMOR, 0, 1, "gold thread"}}, {0}, 1,
        "Dragon scales layered over chain mail.", true};
    item_init(&recipes[i].result, "scale armor", '[', ITEM_ARMOR);
    recipes[i].result.armor_value = 8;
    recipes[i].result.armor_slot = SLOT_CHEST;
    i++;

    recipes[i] = (CraftRecipe){i + 1, "amulet of wisdom", CS_ENCHANTING_TABLE, 5, 3,
        {{ITEM_GEM, 0, 1, "sapphire"}, {ITEM_RING, 0, 1, "gold chain"}, {ITEM_GEM, 0, 1, "diamond dust"}}, {0}, 1,
        "An amulet that sharpens the mind.", true};
    item_init(&recipes[i].result, "amulet of wisdom", '"', ITEM_GEM);
    recipes[i].result.stat_mods[MOD_INTELLIGENCE] = 4;
    recipes[i].result.stat_mods[MOD_MANA] = 15;
    i++;

    recipes[i] = (CraftRecipe){i + 1, "battle axe", CS_FORGE, 4, 3,
        {{ITEM_WEAPON, 0, 3, "steel ingot"}, {ITEM_WEAPON, 0, 2, "wood haft"}, {ITEM_WEAPON, 0, 1, "leather grip"}}, {0}, 1,
        "A heavy two-handed axe for devastating blows.", true};
    item_init(&recipes[i].result, "battle axe", '/', ITEM_WEAPON);
    recipes[i].result.damage_dice = 2;
    recipes[i].result.damage_sides = 6;
    recipes[i].result.damage_bonus = 4;
    i++;

    recipes[i] = (CraftRecipe){i + 1, "enchanted bow", CS_ENCHANTING_TABLE, 5, 3,
        {{ITEM_WEAPON, 0, 2, "yew wood"}, {ITEM_WEAPON, 0, 1, "sinew string"}, {ITEM_GEM, 0, 1, "small crystal"}}, {0}, 1,
        "A bow imbued with magical energy.", true};
    item_init(&recipes[i].result, "enchanted bow", '/', ITEM_WEAPON);
    recipes[i].result.damage_dice = 1;
    recipes[i].result.damage_sides = 10;
    recipes[i].result.damage_bonus = 2;
    recipes[i].result.stat_mods[MOD_DEXTERITY] = 2;
    i++;

    recipes[i] = (CraftRecipe){i + 1, "potion of healing", CS_ALCHEMY_BENCH, 3, 3,
        {{ITEM_POTION, 0, 3, "life root"}, {ITEM_POTION, 0, 2, "pure water"}, {ITEM_POTION, 0, 1, "phoenix feather"}}, {0}, 2,
        "A potent healing potion that restores vitality.", true};
    item_init(&recipes[i].result, "potion of healing", '!', ITEM_POTION);
    recipes[i].result.heal_amount = 35;
    i++;

    recipes[i] = (CraftRecipe){i + 1, "mithril chainmail", CS_FORGE, 6, 4,
        {{ITEM_ARMOR, 0, 4, "mithril ore"}, {ITEM_ARMOR, 0, 2, "silk padding"}, {ITEM_ARMOR, 0, 1, "gem dust"}, {ITEM_ARMOR, 0, 1, "gold thread"}}, {0}, 1,
        "Lightweight mithril chain, stronger than steel.", true};
    item_init(&recipes[i].result, "mithril chainmail", '[', ITEM_ARMOR);
    recipes[i].result.armor_value = 7;
    recipes[i].result.armor_slot = SLOT_CHEST;
    recipes[i].result.stat_mods[MOD_SPEED] = 5;
    i++;

    recipes[i] = (CraftRecipe){i + 1, "scroll of fireball", CS_ALTAR, 5, 3,
        {{ITEM_SCROLL, 0, 2, "parchment"}, {ITEM_SCROLL, 0, 1, "dragon ink"}, {ITEM_SCROLL, 0, 1, "sulfur"}}, {0}, 1,
        "A scroll containing a powerful fireball spell.", true};
    item_init(&recipes[i].result, "scroll of fireball", '?', ITEM_SCROLL);
    recipes[i].result.damage_dice = 3;
    recipes[i].result.damage_sides = 6;
    recipes[i].result.heal_amount = 0;
    i++;

    recipes[i] = (CraftRecipe){i + 1, "runed shield", CS_ENCHANTING_TABLE, 5, 3,
        {{ITEM_ARMOR, 0, 2, "steel shield base"}, {ITEM_GEM, 0, 1, "emerald"}, {ITEM_ARMOR, 0, 1, "enchanting essence"}}, {0}, 1,
        "A shield etched with protective runes.", true};
    item_init(&recipes[i].result, "runed shield", '[', ITEM_ARMOR);
    recipes[i].result.armor_value = 5;
    recipes[i].result.armor_slot = SLOT_SHIELD;
    recipes[i].result.stat_mods[MOD_ARMOR] = 3;
    i++;

    recipes[i] = (CraftRecipe){i + 1, "elven boots", CS_WORKBENCH, 4, 3,
        {{ITEM_ARMOR, 0, 2, "soft leather"}, {ITEM_ARMOR, 0, 1, "elven silk"}, {ITEM_ARMOR, 0, 1, "spring grass"}}, {0}, 1,
        "Boots that grant silent and swift movement.", true};
    item_init(&recipes[i].result, "elven boots", '[', ITEM_ARMOR);
    recipes[i].result.armor_value = 2;
    recipes[i].result.armor_slot = SLOT_FEET;
    recipes[i].result.stat_mods[MOD_SPEED] = 8;
    recipes[i].result.stat_mods[MOD_DEXTERITY] = 1;
    i++;

    recipes[i] = (CraftRecipe){i + 1, "potion of invisibility", CS_CAULDRON, 5, 3,
        {{ITEM_POTION, 0, 2, "ghost essence"}, {ITEM_POTION, 0, 1, "moonlight dew"}, {ITEM_POTION, 0, 1, "crystal vial"}}, {0}, 1,
        "Drink to become invisible to monsters.", true};
    item_init(&recipes[i].result, "potion of invisibility", '!', ITEM_POTION);
    recipes[i].result.stat_mods[MOD_DEXTERITY] = 5;
    i++;

    recipes[i] = (CraftRecipe){i + 1, "dragon scale armor", CS_FORGE, 7, 4,
        {{ITEM_ARMOR, 0, 5, "dragon scales"}, {ITEM_ARMOR, 0, 3, "mithril chain"}, {ITEM_ARMOR, 0, 2, "phoenix down"}, {ITEM_ARMOR, 0, 1, "giant's sinew"}}, {0}, 1,
        "Legendary armor forged from dragon scales.", true};
    item_init(&recipes[i].result, "dragon scale armor", '[', ITEM_ARMOR);
    recipes[i].result.armor_value = 12;
    recipes[i].result.armor_slot = SLOT_CHEST;
    recipes[i].result.stat_mods[MOD_STRENGTH] = 3;
    recipes[i].result.stat_mods[MOD_ARMOR] = 5;
    i++;

    recipes[i] = (CraftRecipe){i + 1, "staff of power", CS_ENCHANTING_TABLE, 6, 3,
        {{ITEM_WEAPON, 0, 2, "elder wood"}, {ITEM_GEM, 0, 2, "large ruby"}, {ITEM_GEM, 0, 1, "diamond"}}, {0}, 1,
        "A staff crackling with arcane energy.", true};
    item_init(&recipes[i].result, "staff of power", '/', ITEM_WEAPON);
    recipes[i].result.damage_dice = 2;
    recipes[i].result.damage_sides = 8;
    recipes[i].result.damage_bonus = 5;
    recipes[i].result.stat_mods[MOD_INTELLIGENCE] = 4;
    recipes[i].result.stat_mods[MOD_MANA] = 20;
    i++;

    recipes[i] = (CraftRecipe){i + 1, "potion of giant strength", CS_CAULDRON, 6, 3,
        {{ITEM_POTION, 0, 3, "troll heart"}, {ITEM_POTION, 0, 2, "stone giant blood"}, {ITEM_POTION, 0, 1, "wyvern claw"}}, {0}, 1,
        "Grants tremendous strength for a time.", true};
    item_init(&recipes[i].result, "potion of giant strength", '!', ITEM_POTION);
    recipes[i].result.stat_mods[MOD_STRENGTH] = 6;
    i++;

    recipes[i] = (CraftRecipe){i + 1, "crystal sword", CS_FORGE, 7, 3,
        {{ITEM_WEAPON, 0, 3, "crystal shard"}, {ITEM_WEAPON, 0, 2, "mithril alloy"}, {ITEM_GEM, 0, 1, "diamond"}}, {0}, 1,
        "A sword of pure crystal, impossibly sharp.", true};
    item_init(&recipes[i].result, "crystal sword", '/', ITEM_WEAPON);
    recipes[i].result.damage_dice = 2;
    recipes[i].result.damage_sides = 8;
    recipes[i].result.damage_bonus = 6;
    recipes[i].result.stat_mods[MOD_ATTACK] = 3;
    i++;

    recipes[i] = (CraftRecipe){i + 1, "crown of wisdom", CS_ENCHANTING_TABLE, 6, 3,
        {{ITEM_ARMOR, 0, 2, "gold circlet"}, {ITEM_GEM, 0, 2, "sapphire"}, {ITEM_GEM, 0, 1, "star ruby"}}, {0}, 1,
        "A crown that enhances the wearer's intellect.", true};
    item_init(&recipes[i].result, "crown of wisdom", '[', ITEM_ARMOR);
    recipes[i].result.armor_value = 3;
    recipes[i].result.armor_slot = SLOT_HEAD;
    recipes[i].result.stat_mods[MOD_INTELLIGENCE] = 5;
    recipes[i].result.stat_mods[MOD_MANA] = 25;
    i++;

    recipes[i] = (CraftRecipe){i + 1, "phase dagger", CS_ENCHANTING_TABLE, 6, 3,
        {{ITEM_WEAPON, 0, 2, "void crystal"}, {ITEM_WEAPON, 0, 2, "shadow silk"}, {ITEM_WEAPON, 0, 1, "phase spider fang"}}, {0}, 1,
        "A dagger that partially phases through armor.", true};
    item_init(&recipes[i].result, "phase dagger", '/', ITEM_WEAPON);
    recipes[i].result.damage_dice = 2;
    recipes[i].result.damage_sides = 4;
    recipes[i].result.damage_bonus = 8;
    i++;

    recipes[i] = (CraftRecipe){i + 1, "phoenix down", CS_ALTAR, 8, 4,
        {{ITEM_POTION, 0, 2, "phoenix feather"}, {ITEM_POTION, 0, 2, "life essence"}, {ITEM_POTION, 0, 1, "pure soul"}, {ITEM_POTION, 0, 1, "sunlight vial"}}, {0}, 1,
        "A legendary reagent that can revive the fallen.", true};
    item_init(&recipes[i].result, "phoenix down", '!', ITEM_POTION);
    recipes[i].result.heal_amount = 999;
    i++;

    *count = i;
    return 0;
}

int crafting_get_recipes_by_tier(const CraftRecipe *recipes, int count, int min_tier, uint32_t *out, int max_out) {
    int found = 0;
    for (int i = 0; i < count && found < max_out; i++) {
        int tier = recipes[i].skill_required / 2;
        if (tier >= min_tier && tier <= min_tier + 2 && recipes[i].discovered) {
            out[found++] = recipes[i].id;
        }
    }
    return found;
}

int crafting_get_recipes_by_type(const CraftRecipe *recipes, int count, ItemType type, uint32_t *out, int max_out) {
    int found = 0;
    for (int i = 0; i < count && found < max_out; i++) {
        if (recipes[i].result.type == type && recipes[i].discovered) {
            out[found++] = recipes[i].id;
        }
    }
    return found;
}

int crafting_get_recipe_count_for_station(const CraftRecipe *recipes, int count, CraftStation station) {
    int n = 0;
    for (int i = 0; i < count; i++) {
        if (recipes[i].station == station) n++;
    }
    return n;
}

int crafting_get_recipe_by_id(const CraftRecipe *recipes, int count, uint32_t id) {
    for (int i = 0; i < count; i++)
        if (recipes[i].id == id) return i;
    return -1;
}

int crafting_get_available_recipes(const CraftRecipe *recipes, int count, CraftStation station, uint32_t *out, int max_out) {
    int found = 0;
    for (int i = 0; i < count && found < max_out; i++) {
        if (recipes[i].station == station && recipes[i].discovered) {
            out[found++] = recipes[i].id;
        }
    }
    return found;
}

bool crafting_has_materials(const CraftRecipe *recipe, const Entity *crafter) {
    if (!recipe || !crafter) return false;
    for (int m = 0; m < recipe->materials_count; m++) {
        int found = 0;
        int needed = recipe->materials[m].quantity;
        for (int i = 0; i < crafter->inventory_count && found < needed; i++) {
            Item *it = (Item *)crafter->inventory[i];
            if (it && it->type == recipe->materials[m].type) {
                found += it->stack_count;
            }
        }
        if (found < needed) return false;
    }
    return true;
}

int crafting_can_craft(const CraftRecipe *recipe, const Entity *crafter) {
    if (!recipe || !crafter) return -1;
    if (recipe->skill_required > 10) return -2;
    if (!crafting_has_materials(recipe, crafter)) return -3;
    return 0;
}

int crafting_calculate_success(const CraftRecipe *recipe, int skill, RNGState *rng) {
    int base = 50 + skill * 5;
    int difficulty = recipe->skill_required * 5;
    int chance = base - difficulty;
    if (chance < 10) chance = 10;
    if (chance > 95) chance = 95;
    return rng_range(rng, 0, 100) < chance ? 1 : 0;
}

int crafting_execute(CraftRecipe *recipe, Entity *crafter, RNGState *rng) {
    if (!recipe || !crafter || !rng) return -1;
    int rc = crafting_can_craft(recipe, crafter);
    if (rc != 0) return rc;

    for (int m = 0; m < recipe->materials_count; m++) {
        int remaining = recipe->materials[m].quantity;
        for (int i = crafter->inventory_count - 1; i >= 0 && remaining > 0; i--) {
            Item *it = (Item *)crafter->inventory[i];
            if (it && it->type == recipe->materials[m].type) {
                int consume = remaining;
                if (consume > it->stack_count) consume = it->stack_count;
                it->stack_count -= consume;
                remaining -= consume;
                if (it->stack_count <= 0) {
                    inventory_remove(crafter, i);
                    item_free(it);
                }
            }
        }
    }

    int success = crafting_calculate_success(recipe, 5, rng);
    if (!success) return -4;

    for (int r = 0; r < recipe->result_quantity; r++) {
        Item *result = item_create(recipe->result.name, recipe->result.glyph, recipe->result.type);
        if (result) {
            result->damage_dice = recipe->result.damage_dice;
            result->damage_sides = recipe->result.damage_sides;
            result->damage_bonus = recipe->result.damage_bonus;
            result->armor_value = recipe->result.armor_value;
            result->heal_amount = recipe->result.heal_amount;
            result->stack_count = 1;
            memcpy(result->stat_mods, recipe->result.stat_mods, sizeof(result->stat_mods));
            if (inventory_add(crafter, result) != 0) {
                item_free(result);
            }
        }
    }
    return 0;
}

int crafting_discover_recipe(CraftRecipe *recipes, int count, uint32_t id) {
    for (int i = 0; i < count; i++) {
        if (recipes[i].id == id) {
            recipes[i].discovered = true;
            return 0;
        }
    }
    return -1;
}

int crafting_generate_random_recipe(CraftRecipe *recipes, int count, int level, RNGState *rng) {
    (void)level;
    if (count <= 0) return -1;
    int idx = rng_range(rng, 0, count);
    if (idx >= 0 && idx < count) {
        recipes[idx].discovered = true;
        return recipes[idx].id;
    }
    return -1;
}

void crafting_print_recipe(const CraftRecipe *recipe) {
    printf("=== %s ===\n", recipe->name);
    printf("Station: %s\n", crafting_station_name(recipe->station));
    printf("Skill required: %d\n", recipe->skill_required);
    printf("Materials:\n");
    for (int m = 0; m < recipe->materials_count; m++) {
        printf("  x%d %s\n", recipe->materials[m].quantity, recipe->materials[m].name);
    }
    printf("Result: %s x%d\n", recipe->result.name, recipe->result_quantity);
    printf("%s\n", recipe->description);
}
