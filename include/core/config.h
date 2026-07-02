#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_PLAYERS 1
#define MAX_MONSTERS 128
#define MAX_ITEMS 256
#define MAX_INVENTORY_SLOTS 64
#define MAX_MAP_WIDTH 80
#define MAX_MAP_HEIGHT 25
#define MAX_DUNGEON_ROOMS 30
#define MAX_ROOM_W 12
#define MAX_ROOM_H 8
#define MIN_ROOM_W 4
#define MIN_ROOM_H 3
#define FOV_RADIUS 8
#define MAX_NAME_LEN 64
#define MAX_ENTITY_TYPES 32
#define MAX_SCHEDULER_EVENTS 256
#define MAX_REPLAY_COMMANDS 65536
#define MAX_LOG_LINES 1024
#define SAVE_MAGIC 0x524F4752
#define SAVE_VERSION 2

typedef enum {
    TILE_WALL = '#',
    TILE_FLOOR = '.',
    TILE_DOOR = '+',
    TILE_STAIRS_DOWN = '>',
    TILE_STAIRS_UP = '<',
    TILE_WATER = '~',
    TILE_LAVA = '^',
    TILE_GRASS = '"',
    TILE_TRAP = '^'
} TileType;

typedef enum {
    DIR_NORTH,
    DIR_SOUTH,
    DIR_EAST,
    DIR_WEST,
    DIR_NORTHEAST,
    DIR_NORTHWEST,
    DIR_SOUTHEAST,
    DIR_SOUTHWEST
} Direction;

typedef enum {
    DT_MELEE,
    DT_RANGED,
    DT_MAGIC,
    DT_FIRE,
    DT_POISON,
    DT_HEAL
} DamageType;

typedef enum {
    AI_WANDER,
    AI_PATROL,
    AI_AGGRESSIVE,
    AI_COWARD,
    AI_GUARD,
    AI_BERSERK,
    AI_PACK,
    AI_SNIPER,
    AI_SUMMONER,
    AI_FLEE
} AIBehavior;

typedef enum {
    ITEM_WEAPON,
    ITEM_ARMOR,
    ITEM_POTION,
    ITEM_SCROLL,
    ITEM_FOOD,
    ITEM_KEY,
    ITEM_GOLD,
    ITEM_RING,
    ITEM_WAND,
    ITEM_GEM
} ItemType;

typedef enum {
    SLOT_HEAD,
    SLOT_CHEST,
    SLOT_LEGS,
    SLOT_FEET,
    SLOT_WEAPON,
    SLOT_SHIELD,
    SLOT_HANDS,
    SLOT_RING1,
    SLOT_RING2,
    SLOT_COUNT
} EquipmentSlot;

typedef enum {
    MOD_HEALTH,
    MOD_MANA,
    MOD_STRENGTH,
    MOD_DEXTERITY,
    MOD_INTELLIGENCE,
    MOD_ARMOR,
    MOD_SPEED,
    MOD_ATTACK,
    MOD_DEFENSE,
    MOD_LUCK
} StatModType;

typedef struct {
    int level;
    int health;
    int max_health;
    int mana;
    int max_mana;
    int strength;
    int dexterity;
    int intelligence;
    int armor;
    int attack;
    int defense;
    int speed;
    int luck;
    int experience;
    int next_level_exp;
    int gold;
} PlayerStats;

typedef struct {
    int room_count;
    int min_rooms;
    int max_rooms;
    int min_room_size;
    int max_room_size;
    int monster_density;
    int item_density;
    int depth;
    int seed;
    bool has_stairs;
    bool dark_sight;
    bool enable_traps;
} DungeonConfig;

typedef struct {
    int player_count;
    int start_health;
    int start_mana;
    int max_dungeon_level;
    int fov_radius;
    int tick_speed_ms;
    bool debug_mode;
    bool god_mode;
    bool show_fov;
    bool enable_fog;
    bool deterministic_mode;
    uint32_t rng_seed;
    char save_file[MAX_NAME_LEN];
    char replay_file[MAX_NAME_LEN];
    DungeonConfig dungeon;
} GameConfig;

void config_init_defaults(GameConfig *cfg);
void config_print(const GameConfig *cfg);

#endif
