#include "engine/entity.h"
#include "engine/map.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static uint32_t next_entity_id = 1;

Entity *entity_create(const char *name, char glyph, int x, int y) {
    Entity *e = (Entity *)malloc(sizeof(Entity));
    if (!e) return NULL;

    memset(e, 0, sizeof(Entity));
    e->id = next_entity_id++;
    strncpy(e->name, name, MAX_NAME_LEN - 1);
    e->glyph = glyph;
    e->x = x;
    e->y = y;
    e->prev_x = x;
    e->prev_y = y;
    e->alive = true;
    e->blocks_movement = false;
    e->blocks_sight = false;
    e->is_player = false;
    e->is_monster = false;
    e->is_item = false;
    e->health = 10;
    e->max_health = 10;
    e->mana = 5;
    e->max_mana = 5;
    e->strength = 10;
    e->dexterity = 10;
    e->intelligence = 10;
    e->attack = 3;
    e->defense = 1;
    e->armor = 0;
    e->speed = 100;
    e->luck = 5;
    e->level = 1;
    e->experience = 0;
    e->gold = 0;
    e->behavior = AI_WANDER;
    e->alert_range = 5;
    e->move_counter = 0;
    e->damage_dice = 1;
    e->damage_sides = 4;
    e->damage_bonus = 0;
    e->xp_value = 5;
    e->faction_id = 0;
    e->vision_range = 6;
    e->path_step = 0;
    e->path_len = 0;
    e->target = NULL;
    e->inventory = NULL;
    e->inventory_count = 0;
    e->inventory_capacity = 0;
    e->ai_data = NULL;

    memset(e->equipment, -1, sizeof(e->equipment));
    memset(e->path, 0, sizeof(e->path));

    return e;
}

void entity_destroy(Entity *e) {
    if (!e) return;
    if (e->inventory) {
        for (int i = 0; i < e->inventory_count; i++) {
            Item *it = (Item *)e->inventory[i];
            if (it) {
                item_free(it);
            }
        }
        free(e->inventory);
        e->inventory = NULL;
    }
    if (e->ai_data) {
        free(e->ai_data);
        e->ai_data = NULL;
    }
}

void entity_take_damage(Entity *e, int amount, DamageType dtype) {
    (void)dtype;

    int actual = amount - e->defense - e->armor;
    if (actual < 1) actual = 1;

    e->health -= actual;
    if (e->health <= 0) {
        e->health = 0;
        e->alive = false;
    }
}

void entity_heal(Entity *e, int amount) {
    e->health += amount;
    if (e->health > e->max_health) e->health = e->max_health;
}

void entity_apply_status(Entity *e, int poison, int burn, int slow, int haste) {
    e->poison_turns += poison;
    e->burn_turns += burn;
    e->slow_turns += slow;
    e->haste_turns += haste;
}

void entity_tick_status(Entity *e) {
    if (e->poison_turns > 0) {
        e->health -= 1;
        e->poison_turns--;
    }
    if (e->burn_turns > 0) {
        e->health -= 2;
        e->burn_turns--;
    }
    if (e->slow_turns > 0) e->slow_turns--;
    if (e->haste_turns > 0) e->haste_turns--;

    if (e->health < 0) e->health = 0;
    if (e->health == 0) e->alive = false;
}

bool entity_is_alive(const Entity *e) {
    return e && e->alive;
}

int entity_get_stat(const Entity *e, StatModType stat) {
    switch (stat) {
        case MOD_HEALTH: return e->max_health;
        case MOD_MANA: return e->max_mana;
        case MOD_STRENGTH: return e->strength;
        case MOD_DEXTERITY: return e->dexterity;
        case MOD_INTELLIGENCE: return e->intelligence;
        case MOD_ARMOR: return e->armor;
        case MOD_SPEED: return e->speed;
        case MOD_ATTACK: return e->attack;
        case MOD_DEFENSE: return e->defense;
        case MOD_LUCK: return e->luck;
        default: return 0;
    }
}

int entity_roll_attack(const Entity *e) {
    return e->attack + (rand() % 6) - 3;
}

int entity_roll_damage(const Entity *e) {
    int dmg = 0;
    for (int i = 0; i < e->damage_dice; i++)
        dmg += (rand() % e->damage_sides) + 1;
    return dmg + e->damage_bonus;
}

bool entity_can_see(const Entity *e, int tx, int ty, const Map *map) {
    if (!map) return false;
    int dx = tx - e->x;
    int dy = ty - e->y;
    int dist = dx * dx + dy * dy;
    if (dist > e->vision_range * e->vision_range) return false;
    return map_is_visible(map, tx, ty);
}

/* Monster template definitions for procedural generation */

typedef struct {
    const char *name;
    char glyph;
    int min_depth;
    int max_depth;
    int base_health;
    int base_attack;
    int base_defense;
    int damage_dice;
    int damage_sides;
    int damage_bonus;
    AIBehavior behavior;
    int alert_range;
    int xp_value;
    int faction_id;
    int speed;
} MonsterTemplate;

static const MonsterTemplate monster_templates[] = {
    /* T1: Depth 1-3 */
    {"giant rat",      'r', 1, 10, 8,  3, 0, 1, 3, 0, AI_WANDER,    5, 5,  0, 100},
    {"bat",            'b', 1, 5,  6,  2, 0, 1, 2, 0, AI_WANDER,    8, 3,  0, 120},
    {"goblin",         'g', 1, 8,  12, 5, 1, 1, 4, 0, AI_AGGRESSIVE,6, 10, 1, 90},
    {"fire beetle",    'f', 1, 5,  10, 4, 1, 1, 3, 1, AI_WANDER,    4, 6,  0, 80},
    {"snake",          's', 1, 6,  8,  6, 0, 1, 3, 1, AI_WANDER,    5, 7,  0, 110},

    /* T2: Depth 3-6 */
    {"skeleton",       'S', 3, 15, 18, 7, 2, 1, 5, 1, AI_AGGRESSIVE,7, 15, 2, 80},
    {"orc",            'o', 3, 12, 22, 8, 3, 1, 6, 1, AI_AGGRESSIVE,6, 18, 1, 85},
    {"giant spider",   'z', 3, 10, 16, 6, 1, 1, 4, 2, AI_PATROL,    7, 14, 0, 95},
    {"wraith",         'W', 3, 15, 15, 9, 1, 1, 5, 2, AI_COWARD,    9, 20, 3, 90},
    {"kobold",         'k', 3, 8,  14, 6, 1, 1, 4, 0, AI_WANDER,    5, 8,  0, 105},

    /* T3: Depth 5-10 */
    {"troll",          'T', 5, 20, 35, 10, 4, 2, 5, 3, AI_AGGRESSIVE,8, 35, 1, 70},
    {"vampire bat",    'V', 5, 15, 20, 9, 2, 1, 6, 2, AI_AGGRESSIVE,10,25, 3, 130},
    {"zombie",         'Z', 5, 12, 30, 7, 2, 1, 4, 1, AI_PATROL,    5, 20, 2, 50},
    {"dark elf",       'D', 5, 15, 20, 11, 3, 1, 6, 2, AI_SNIPER,   9, 28, 1, 100},
    {"giant scorpion", 'c', 5, 12, 25, 9, 4, 1, 5, 3, AI_GUARD,    7, 30, 0, 75},

    /* T4: Depth 8-15 */
    {"minotaur",       'M', 8, 25, 50, 14, 5, 2, 6, 4, AI_BERSERK,  10, 50, 1, 80},
    {"lich",           'L', 8, 25, 30, 12, 3, 1, 8, 5, AI_SUMMONER, 10, 55, 3, 75},
    {"shadow",         'h', 8, 20, 22, 10, 2, 1, 5, 3, AI_PACK,     10, 30, 3, 110},
    {"golem",          'G', 8, 30, 60, 8, 8, 2, 5, 3, AI_GUARD,    6, 45, 0, 40},
    {"harpy",          'H', 8, 18, 24, 11, 2, 1, 6, 2, AI_FLEE,     10, 28, 0, 120},

    /* T5: Depth 12-25 */
    {"dragon whelp",   'd', 12, 30, 70, 16, 7, 2, 8, 6, AI_AGGRESSIVE,12, 80, 1, 75},
    {"demon",          'X', 12, 30, 55, 15, 6, 2, 7, 5, AI_BERSERK,  12, 70, 3, 85},
    {"giant worm",     'w', 12, 25, 65, 13, 5, 1, 6, 4, AI_WANDER,   7, 55, 0, 60},
    {"necromancer",    'N', 12, 30, 35, 14, 4, 1, 9, 6, AI_SUMMONER, 11, 75, 3, 80},
    {"spectre",        'E', 12, 30, 28, 13, 3, 1, 7, 5, AI_PACK,     12, 45, 3, 100},

    /* T6: Bosses, Depth 15+ */
    {"ancient dragon", 'A', 15, 50, 120, 22, 12, 2, 10, 8, AI_AGGRESSIVE,15, 200, 1, 70},
    {"balrog",         'B', 18, 50, 150, 25, 14, 3, 10, 10, AI_BERSERK,  15, 250, 3, 80},
    {"elven lord",     'E', 15, 30, 80, 20, 9, 1, 8, 6, AI_SNIPER,   14, 150, 1, 110},
};

static const int monster_template_count =
    sizeof(monster_templates) / sizeof(monster_templates[0]);

int entity_get_monster_template_count(void) {
    return monster_template_count;
}

const char *entity_get_monster_name(int template_idx) {
    if (template_idx < 0 || template_idx >= monster_template_count) return NULL;
    return monster_templates[template_idx].name;
}

Entity *entity_create_monster(const char *name, char glyph, int x, int y,
                              int depth, int template_idx) {
    Entity *e = entity_create(name, glyph, x, y);
    if (!e) return NULL;

    e->is_monster = true;
    e->alive = true;
    e->blocks_movement = true;

    if (template_idx >= 0 && template_idx < monster_template_count) {
        const MonsterTemplate *mt = &monster_templates[template_idx];
        e->health = mt->base_health + depth * 2;
        e->max_health = e->health;
        e->attack = mt->base_attack + depth;
        e->defense = mt->base_defense + depth / 2;
        e->damage_dice = mt->damage_dice;
        e->damage_sides = mt->damage_sides + depth / 3;
        e->damage_bonus = mt->damage_bonus + depth / 2;
        e->xp_value = mt->xp_value + depth * 3;
        e->behavior = mt->behavior;
        e->alert_range = mt->alert_range + depth / 2;
        e->speed = mt->speed;
        e->faction_id = mt->faction_id;
        e->dexterity = 8 + depth / 2;
        e->strength = 8 + depth;
        e->intelligence = 5 + depth / 2;
        e->armor = depth / 3;
        e->luck = 5;
        e->gold = depth * 5 + 10;
        e->vision_range = 8;
        e->stun_turns = 0;
        e->poison_turns = 0;
        e->burn_turns = 0;
        e->slow_turns = 0;
        e->haste_turns = 0;
    }

    return e;
}

Entity *entity_create_monster_for_depth(int depth, int x, int y, RNGState *rng) {
    int eligible[MAX_MONSTERS];
    int eligible_count = 0;

    for (int i = 0; i < monster_template_count; i++) {
        if (depth >= monster_templates[i].min_depth &&
            depth <= monster_templates[i].max_depth) {
            eligible[eligible_count++] = i;
        }
    }

    if (eligible_count == 0) {
        eligible[0] = 0;
        eligible_count = 1;
    }

    int idx = eligible[rng_range(rng, 0, eligible_count)];
    return entity_create_monster(monster_templates[idx].name,
                                 monster_templates[idx].glyph,
                                 x, y, depth, idx);
}

Entity *entity_create_theme_monster(int depth, int x, int y, int theme,
                                     RNGState *rng) {
    int eligible[MAX_MONSTERS];
    int eligible_count = 0;
    int theme_faction = (theme % 4) + 1;

    for (int i = 0; i < monster_template_count; i++) {
        if (depth >= monster_templates[i].min_depth &&
            depth <= monster_templates[i].max_depth) {
            bool faction_match = (monster_templates[i].faction_id == theme_faction);
            if (faction_match || rng_range(rng, 0, 100) < 40) {
                eligible[eligible_count++] = i;
            }
        }
    }

    if (eligible_count == 0) {
        int idx = rng_range(rng, 0, monster_template_count);
        if (idx < monster_template_count)
            eligible[eligible_count++] = idx;
    }

    if (eligible_count == 0) {
        return entity_create_monster("rat", 'r', x, y, depth, 0);
    }

    int idx = eligible[rng_range(rng, 0, eligible_count)];
    return entity_create_monster(monster_templates[idx].name,
                                 monster_templates[idx].glyph,
                                 x, y, depth, idx);
}

int entity_get_xp_value(const Entity *e) {
    return e ? e->xp_value : 0;
}

bool entity_has_status_effect(const Entity *e) {
    return e && (e->poison_turns > 0 || e->burn_turns > 0 ||
                 e->slow_turns > 0 || e->haste_turns > 0 ||
                 e->stun_turns > 0);
}

int entity_distance(const Entity *a, const Entity *b) {
    if (!a || !b) return 9999;
    int dx = a->x - b->x;
    int dy = a->y - b->y;
    return (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy);
}

void entity_reset_path(Entity *e) {
    if (e) {
        e->path_len = 0;
        e->path_step = 0;
    }
}

bool entity_has_path(const Entity *e) {
    return e && e->path_len > 0;
}

int entity_next_path_step(Entity *e) {
    if (!e || e->path_step >= e->path_len) return -1;
    int step = e->path[e->path_step];
    e->path_step++;
    return step;
}
