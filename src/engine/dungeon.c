#include "engine/dungeon.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define CHUNK_BUF_SIZE (MAX_MAP_WIDTH * MAX_MAP_HEIGHT)
static char chunk_buf[CHUNK_BUF_SIZE];
static int chunk_width = MAX_MAP_WIDTH;
static int chunk_height = MAX_MAP_HEIGHT;

void dungeon_set_chunk_size(int w, int h) {
    chunk_width = w;
    chunk_height = h;
}

static void chunk_write_tile(int x, int y, char tile) {
    if (x < 0 || y < 0 || x >= chunk_width || y >= chunk_height) return;
    int idx = x * chunk_height + y;
    if (idx < 0 || idx >= CHUNK_BUF_SIZE) return;
    chunk_buf[idx] = tile;
}

static char chunk_read_tile(int x, int y) {
    if (x < 0 || y < 0 || x >= chunk_width || y >= chunk_height) return TILE_WALL;
    int idx = x * chunk_height + y;
    if (idx < 0 || idx >= CHUNK_BUF_SIZE) return TILE_WALL;
    return chunk_buf[idx];
}

static void dungeon_generate_chunk_terrain(const Dungeon *d) {
    if (chunk_width <= 0 || chunk_height <= 0) return;
    memset(chunk_buf, TILE_WALL, CHUNK_BUF_SIZE);
    for (int i = 0; i < d->room_count; i++) {
        const Room *r = &d->rooms[i];
        for (int y = r->y; y < r->y + r->h && y < chunk_height; y++) {
            for (int x = r->x; x < r->x + r->w && x < chunk_width; x++) {
                chunk_write_tile(x, y, TILE_FLOOR);
            }
        }
    }
}

static void dungeon_apply_chunk_to_map(Map *m) {
    for (int y = 0; y < m->height && y < chunk_height; y++) {
        for (int x = 0; x < m->width && x < chunk_width; x++) {
            char tile = chunk_read_tile(x, y);
            if (tile != TILE_WALL) {
                map_set_tile(m, x, y, tile);
            }
        }
    }
}

typedef enum {
    DT_DEFAULT,
    DT_CAVE,
    DT_CRYPT,
    DT_FOREST,
    DT_LAVA,
    DT_ICE,
    DT_SEWER,
    DT_TEMPLE
} DungeonTheme;

static const char *theme_names[] = {
    "ruined halls", "natural caverns", "ancient crypt",
    "overgrown forest", "volcanic tunnels", "frozen caves",
    "dark sewers", "abandoned temple"
};

static int theme_water_chance(DungeonTheme theme) {
    switch (theme) {
        case DT_SEWER: return 15;
        case DT_CAVE: return 8;
        case DT_FOREST: return 12;
        default: return 3;
    }
}

static int theme_trap_chance(DungeonTheme theme) {
    switch (theme) {
        case DT_CRYPT: return 12;
        case DT_CAVE: return 8;
        case DT_TEMPLE: return 10;
        default: return 5;
    }
}

static int theme_monster_density(DungeonTheme theme) {
    switch (theme) {
        case DT_CAVE: return 4;
        case DT_CRYPT: return 5;
        case DT_SEWER: return 6;
        case DT_FOREST: return 3;
        default: return 3;
    }
}

int dungeon_generate(Dungeon *d, int width, int height, int min_rooms, int max_rooms, RNGState *rng) {
    d->width = width;
    d->height = height;
    d->room_count = 0;

    dungeon_set_chunk_size(width, height);

    int target = rng_range(rng, min_rooms, max_rooms + 1);
    int attempts = 0;

    while (d->room_count < target && attempts < 100) {
        attempts++;
        int rw = rng_range(rng, MIN_ROOM_W, MAX_ROOM_W + 1);
        int rh = rng_range(rng, MIN_ROOM_H, MAX_ROOM_H + 1);

        if (rw > width - 2) rw = width - 2;
        if (rh > height - 2) rh = height - 2;

        int rx = rng_range(rng, 1, width - rw - 1);
        int ry = rng_range(rng, 1, height - rh - 1);

        if (!dungeon_room_overlaps(d, rx, ry, rw, rh)) {
            dungeon_add_room(d, rx, ry, rw, rh);
        }
    }

    return d->room_count;
}

bool dungeon_room_overlaps(const Dungeon *d, int x, int y, int w, int h) {
    for (int i = 0; i < d->room_count; i++) {
        const Room *r = &d->rooms[i];
        if (x - 1 < r->x + r->w && x + w + 1 > r->x &&
            y - 1 < r->y + r->h && y + h + 1 > r->y)
            return true;
    }
    return false;
}

void dungeon_add_room(Dungeon *d, int x, int y, int w, int h) {
    if (d->room_count >= MAX_DUNGEON_ROOMS) return;
    Room *r = &d->rooms[d->room_count++];
    r->x = x;
    r->y = y;
    r->w = w;
    r->h = h;
    r->cx = x + w / 2;
    r->cy = y + h / 2;
}

int dungeon_place_on_map(const Dungeon *d, Map *m, int depth) {
    (void)depth;

    for (int i = 0; i < d->room_count; i++) {
        const Room *r = &d->rooms[i];
        map_carve_room(m, r->x, r->y, r->w, r->h);
    }

    for (int i = 0; i < d->room_count; i++) {
        if (i + 1 < d->room_count) {
            const Room *r1 = &d->rooms[i];
            const Room *r2 = &d->rooms[i + 1];
            map_carve_corridor(m, r1->cx, r1->cy, r2->cx, r2->cy);
        }
    }

    dungeon_generate_chunk_terrain(d);
    dungeon_apply_chunk_to_map(m);

    m->room_count = d->room_count;
    for (int i = 0; i < d->room_count && i < MAX_DUNGEON_ROOMS; i++) {
        m->rooms[i].x = d->rooms[i].x;
        m->rooms[i].y = d->rooms[i].y;
        m->rooms[i].w = d->rooms[i].w;
        m->rooms[i].h = d->rooms[i].h;
        m->rooms[i].cx = d->rooms[i].cx;
        m->rooms[i].cy = d->rooms[i].cy;
    }
    return 0;
}

int dungeon_connect_rooms(Dungeon *d, Map *m, RNGState *rng) {
    (void)rng;
    for (int i = 0; i < d->room_count - 1; i++) {
        const Room *r1 = &d->rooms[i];
        const Room *r2 = &d->rooms[i + 1];
        map_carve_corridor(m, r1->cx, r1->cy, r2->cx, r2->cy);
    }
    if (d->room_count > 2) {
        const Room *r1 = &d->rooms[0];
        const Room *r2 = &d->rooms[d->room_count - 1];
        map_carve_corridor(m, r1->cx, r1->cy, r2->cx, r2->cy);
    }
    return 0;
}

void dungeon_place_stairs(Dungeon *d, Map *m, bool down, RNGState *rng) {
    if (d->room_count == 0) return;
    int idx = rng_range(rng, 0, d->room_count);
    const Room *r = &d->rooms[idx];
    int sx = r->cx;
    int sy = r->cy;

    if (down) {
        m->stairs_down_x = sx;
        m->stairs_down_y = sy;
        m->tiles[sx][sy] = TILE_STAIRS_DOWN;
    } else {
        m->stairs_up_x = sx;
        m->stairs_up_y = sy;
        m->tiles[sx][sy] = TILE_STAIRS_UP;
    }
}

int dungeon_find_random_floor(const Map *m, int *x, int *y, RNGState *rng) {
    int attempts = 0;
    while (attempts < 100) {
        attempts++;
        int tx = rng_range(rng, 0, m->width);
        int ty = rng_range(rng, 0, m->height);
        if (map_is_walkable(m, tx, ty)) {
            *x = tx;
            *y = ty;
            return 1;
        }
    }
    for (int yy = 0; yy < m->height; yy++)
        for (int xx = 0; xx < m->width; xx++)
            if (map_is_walkable(m, xx, yy)) {
                *x = xx;
                *y = yy;
                return 1;
            }
    return 0;
}

const char *dungeon_get_theme_name(int depth) {
    int idx = (depth - 1) % 8;
    if (idx < 0) idx = 0;
    if (idx > 7) idx = 7;
    return theme_names[idx];
}

void dungeon_carve_crypt_room(Dungeon *d, Map *m, const Room *r, RNGState *rng) {
    for (int y = r->y; y < r->y + r->h; y++) {
        for (int x = r->x; x < r->x + r->w; x++) {
            m->tiles[x][y] = map_in_bounds(m, x, y) ? ':' : TILE_FLOOR;
        }
    }
    for (int y = r->y; y < r->y + r->h; y++) {
        for (int x = r->x; x < r->x + r->w; x++) {
            if ((x == r->x || x == r->x + r->w - 1 || y == r->y || y == r->y + r->h - 1) &&
                rng_range(rng, 0, 100) < 30) {
                m->tiles[x][y] = '#';
            }
        }
    }
    if (rng_range(rng, 0, 100) < 20) {
        int cx = r->cx, cy = r->cy;
        m->tiles[cx][cy] = '&';
    }
}

void dungeon_carve_forest_room(Dungeon *d, Map *m, const Room *r, RNGState *rng) {
    for (int y = r->y; y < r->y + r->h; y++) {
        for (int x = r->x; x < r->x + r->w; x++) {
            if (map_in_bounds(m, x, y)) {
                m->tiles[x][y] = rng_range(rng, 0, 100) < 30 ? '\"' : TILE_FLOOR;
            }
        }
    }
    if (rng_range(rng, 0, 100) < 15) {
        int tx = r->x + rng_range(rng, 1, r->w - 1);
        int ty = r->y + rng_range(rng, 1, r->h - 1);
        if (map_in_bounds(m, tx, ty)) m->tiles[tx][ty] = 'T';
    }
}

void dungeon_carve_lava_room(Dungeon *d, Map *m, const Room *r, RNGState *rng) {
    for (int y = r->y; y < r->y + r->h; y++) {
        for (int x = r->x; x < r->x + r->w; x++) {
            if (map_in_bounds(m, x, y)) {
                m->tiles[x][y] = rng_range(rng, 0, 100) < 15 ? '^' : TILE_FLOOR;
            }
        }
    }
}

void dungeon_carve_ice_room(Dungeon *d, Map *m, const Room *r, RNGState *rng) {
    for (int y = r->y; y < r->y + r->h; y++) {
        for (int x = r->x; x < r->x + r->w; x++) {
            if (map_in_bounds(m, x, y)) {
                m->tiles[x][y] = rng_range(rng, 0, 100) < 20 ? ':' : TILE_FLOOR;
            }
        }
    }
}

void dungeon_carve_sewer_room(Dungeon *d, Map *m, const Room *r, RNGState *rng) {
    for (int y = r->y; y < r->y + r->h; y++) {
        for (int x = r->x; x < r->x + r->w; x++) {
            if (map_in_bounds(m, x, y)) {
                m->tiles[x][y] = rng_range(rng, 0, 100) < 25 ? '~' : TILE_FLOOR;
            }
        }
    }
}

void dungeon_carve_temple_room(Dungeon *d, Map *m, const Room *r, RNGState *rng) {
    for (int y = r->y; y < r->y + r->h; y++) {
        for (int x = r->x; x < r->x + r->w; x++) {
            if (map_in_bounds(m, x, y)) {
                m->tiles[x][y] = TILE_FLOOR;
            }
        }
    }
    int altar_x = r->cx, altar_y = r->cy;
    m->tiles[altar_x][altar_y] = '_';
    if (rng_range(rng, 0, 100) < 30) {
        if (altar_x + 1 < m->width) m->tiles[altar_x + 1][altar_y] = '.';
    }
}

void dungeon_carve_theme_room(Dungeon *d, Map *m, const Room *r, int theme, RNGState *rng) {
    switch ((DungeonTheme)theme) {
        case DT_CRYPT:
            dungeon_carve_crypt_room(d, m, r, rng);
            break;
        case DT_FOREST:
            dungeon_carve_forest_room(d, m, r, rng);
            break;
        case DT_LAVA:
            dungeon_carve_lava_room(d, m, r, rng);
            break;
        case DT_ICE:
            dungeon_carve_ice_room(d, m, r, rng);
            break;
        case DT_SEWER:
            dungeon_carve_sewer_room(d, m, r, rng);
            break;
        case DT_TEMPLE:
            dungeon_carve_temple_room(d, m, r, rng);
            break;
        default:
            break;
    }
}

void dungeon_carve_theme_corridor(Map *m, int x1, int y1, int x2, int y2, int theme, RNGState *rng) {
    char floor = '.';
    char wall = '#';
    switch ((DungeonTheme)theme) {
        case DT_CRYPT:
            floor = ':'; wall = '#';
            break;
        case DT_FOREST:
            floor = '.'; wall = '%';
            break;
        case DT_LAVA:
            floor = '.'; wall = '#';
            if (rng_range(rng, 0, 100) < 10) floor = '^';
            break;
        case DT_ICE:
            floor = ':'; wall = '#';
            break;
        case DT_SEWER:
            floor = '~'; wall = '#';
            break;
        case DT_TEMPLE:
            floor = '.'; wall = '#';
            break;
        default:
            break;
    }
    int cx = x1, cy = y1;
    while (cx != x2) {
        if (map_in_bounds(m, cx, cy))
            m->tiles[cx][cy] = floor;
        cx += (cx < x2) ? 1 : -1;
    }
    while (cy != y2) {
        if (map_in_bounds(m, cx, cy))
            m->tiles[cx][cy] = floor;
        cy += (cy < y2) ? 1 : -1;
    }
    (void)wall;
}

void dungeon_apply_theme_placement(const Dungeon *d, Map *m, int theme, RNGState *rng) {
    for (int i = 0; i < d->room_count; i++) {
        dungeon_carve_theme_room((Dungeon *)d, m, &d->rooms[i], theme, rng);
    }
    for (int i = 0; i < d->room_count - 1; i++) {
        dungeon_carve_theme_corridor(m,
            d->rooms[i].cx, d->rooms[i].cy,
            d->rooms[i+1].cx, d->rooms[i+1].cy,
            theme, rng);
    }
}

void dungeon_add_decoration(Map *m, int theme, RNGState *rng) {
    int wc = theme_water_chance(theme);
    for (int y = 1; y < m->height - 1; y++) {
        for (int x = 1; x < m->width - 1; x++) {
            if (m->tiles[x][y] == TILE_FLOOR && rng_range(rng, 0, 100) < wc) {
                bool adjacent_to_wall =
                    m->tiles[x-1][y] == TILE_WALL ||
                    m->tiles[x+1][y] == TILE_WALL ||
                    m->tiles[x][y-1] == TILE_WALL ||
                    m->tiles[x][y+1] == TILE_WALL;
                if (!adjacent_to_wall) {
                    m->tiles[x][y] = TILE_WATER;
                }
            }
        }
    }
    if (theme == DT_LAVA) {
        for (int y = 1; y < m->height - 1; y++) {
            for (int x = 1; x < m->width - 1; x++) {
                if (m->tiles[x][y] == TILE_FLOOR && rng_range(rng, 0, 100) < 5) {
                    m->tiles[x][y] = TILE_LAVA;
                }
            }
        }
    }
    if (theme == DT_FOREST) {
        for (int y = 1; y < m->height - 1; y++) {
            for (int x = 1; x < m->width - 1; x++) {
                if (m->tiles[x][y] == TILE_FLOOR && rng_range(rng, 0, 100) < 10) {
                    m->tiles[x][y] = TILE_GRASS;
                }
            }
        }
    }
}

void dungeon_add_vault_room(Dungeon *d, Map *m, RNGState *rng) {
    if (d->room_count <= 0 || d->room_count >= MAX_DUNGEON_ROOMS) return;
    int idx = rng_range(rng, 0, d->room_count);
    const Room *base = &d->rooms[idx];
    int vx = base->cx - 2;
    int vy = base->cy - 2;
    int vw = 5;
    int vh = 5;
    if (vx < 1 || vy < 1 || vx + vw >= m->width - 1 || vy + vh >= m->height - 1) return;
    for (int y = vy; y < vy + vh; y++) {
        for (int x = vx; x < vx + vw; x++) {
            if (x == vx || x == vx + vw - 1 || y == vy || y == vy + vh - 1) {
                m->tiles[x][y] = TILE_WALL;
            } else {
                m->tiles[x][y] = TILE_FLOOR;
            }
        }
    }
    int door_x = vx + vw / 2;
    int door_y = vy;
    m->tiles[door_x][door_y] = TILE_DOOR;
}

int dungeon_get_theme_for_depth(int depth) {
    int idx = (depth - 1) % 8;
    if (idx < 0) idx = 0;
    if (idx > 7) idx = 7;
    return idx;
}

int dungeon_get_monster_density(int depth) {
    DungeonTheme theme = dungeon_get_theme_for_depth(depth);
    return theme_monster_density(theme) + depth / 3;
}

int dungeon_get_trap_chance(int depth) {
    DungeonTheme theme = dungeon_get_theme_for_depth(depth);
    return theme_trap_chance(theme) + depth / 5;
}
