#include "engine/map.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int map_init(Map *m, int width, int height) {
    if (width > MAX_MAP_WIDTH) width = MAX_MAP_WIDTH;
    if (height > MAX_MAP_HEIGHT) height = MAX_MAP_HEIGHT;
    m->width = width;
    m->height = height;
    m->room_count = 0;
    m->stairs_down_x = -1;
    m->stairs_down_y = -1;
    m->stairs_up_x = -1;
    m->stairs_up_y = -1;
    map_clear(m, TILE_WALL);
    return 0;
}

void map_clear(Map *m, char fill) {
    for (int y = 0; y < m->height; y++)
        for (int x = 0; x < m->width; x++)
            m->tiles[x][y] = fill;
    memset(m->visible, 0, sizeof(m->visible));
    memset(m->explored, 0, sizeof(m->explored));
    memset(m->light, 0, sizeof(m->light));
    memset(m->flags, 0, sizeof(m->flags));
}

bool map_in_bounds(const Map *m, int x, int y) {
    return x >= 0 && x < m->width && y >= 0 && y < m->height;
}

bool map_is_walkable(const Map *m, int x, int y) {
    if (!map_in_bounds(m, x, y)) return false;
    char t = m->tiles[x][y];
    return t == TILE_FLOOR || t == TILE_DOOR || t == TILE_STAIRS_DOWN ||
           t == TILE_STAIRS_UP || t == TILE_GRASS;
}

bool map_is_transparent(const Map *m, int x, int y) {
    if (!map_in_bounds(m, x, y)) return false;
    char t = m->tiles[x][y];
    return t != TILE_WALL && t != TILE_LAVA;
}

void map_set_tile(Map *m, int x, int y, char tile) {
    if (map_in_bounds(m, x, y)) m->tiles[x][y] = tile;
}

char map_get_tile(const Map *m, int x, int y) {
    if (!map_in_bounds(m, x, y)) return TILE_WALL;
    return m->tiles[x][y];
}

int map_get_room_at(const Map *m, int x, int y) {
    for (int i = 0; i < m->room_count; i++) {
        if (x >= m->rooms[i].x && x < m->rooms[i].x + m->rooms[i].w &&
            y >= m->rooms[i].y && y < m->rooms[i].y + m->rooms[i].h)
            return i;
    }
    return -1;
}

void map_carve_room(Map *m, int x, int y, int w, int h) {
    for (int r = y; r < y + h && r < m->height; r++)
        for (int c = x; c < x + w && c < m->width; c++)
            m->tiles[c][r] = TILE_FLOOR;
}

void map_carve_corridor(Map *m, int x1, int y1, int x2, int y2) {
    int cx = x1, cy = y1;
    while (cx != x2) {
        if (map_in_bounds(m, cx, cy))
            m->tiles[cx][cy] = TILE_FLOOR;
        cx += (cx < x2) ? 1 : -1;
    }
    while (cy != y2) {
        if (map_in_bounds(m, cx, cy))
            m->tiles[cx][cy] = TILE_FLOOR;
        cy += (cy < y2) ? 1 : -1;
    }
}

static void cast_ray(Map *m, int x0, int y0, int radius, float dx, float dy) {
    float cx = (float)x0 + 0.5f;
    float cy = (float)y0 + 0.5f;
    for (int i = 0; i <= radius; i++) {
        int tx = (int)cx;
        int ty = (int)cy;
        if (!map_in_bounds(m, tx, ty)) break;
        m->visible[tx][ty] = true;
        m->explored[tx][ty] = true;
        if (!map_is_transparent(m, tx, ty) && !(tx == x0 && ty == y0)) break;
        cx += dx;
        cy += dy;
    }
}

void map_compute_fov(Map *m, int px, int py, int radius) {
    memset(m->visible, 0, sizeof(m->visible));

    if (!map_in_bounds(m, px, py)) return;

    m->visible[px][py] = true;
    m->explored[px][py] = true;

    int steps = radius * 8;
    for (int i = 0; i < steps; i++) {
        float angle = (float)i * 3.14159f * 2.0f / (float)steps;
        cast_ray(m, px, py, radius, (float)cos(angle), (float)sin(angle));
    }
}

void map_set_visible(Map *m, int x, int y, bool v) {
    if (map_in_bounds(m, x, y)) m->visible[x][y] = v;
}

bool map_is_visible(const Map *m, int x, int y) {
    if (!map_in_bounds(m, x, y)) return false;
    return m->visible[x][y];
}

void map_copy(Map *dst, const Map *src) {
    memcpy(dst, src, sizeof(Map));
}

int map_count_tile_type(const Map *m, char tile) {
    int count = 0;
    for (int y = 0; y < m->height; y++)
        for (int x = 0; x < m->width; x++)
            if (m->tiles[x][y] == tile) count++;
    return count;
}

int map_count_walkable_tiles(const Map *m) {
    int count = 0;
    for (int y = 0; y < m->height; y++)
        for (int x = 0; x < m->width; x++)
            if (map_is_walkable(m, x, y)) count++;
    return count;
}

void map_flood_fill(Map *m, int sx, int sy, char fill, char target) {
    if (!map_in_bounds(m, sx, sy)) return;
    if (m->tiles[sx][sy] != target) return;
    
    /* Iterative flood fill using stack */
    int stack[MAX_MAP_WIDTH * MAX_MAP_HEIGHT * 2];
    int sp = 0;
    stack[sp++] = sx;
    stack[sp++] = sy;
    
    while (sp > 0) {
        int cy = stack[--sp];
        int cx = stack[--sp];
        if (!map_in_bounds(m, cx, cy)) continue;
        if (m->tiles[cx][cy] != target) continue;
        m->tiles[cx][cy] = fill;
        stack[sp++] = cx - 1; stack[sp++] = cy;
        stack[sp++] = cx + 1; stack[sp++] = cy;
        stack[sp++] = cx; stack[sp++] = cy - 1;
        stack[sp++] = cx; stack[sp++] = cy + 1;
        if (sp >= MAX_MAP_WIDTH * MAX_MAP_HEIGHT * 2 - 4) break;
    }
}

void map_place_terrain_cluster(Map *m, char tile, int cx, int cy,
                                int radius, int density, RNGState *rng) {
    for (int y = cy - radius; y <= cy + radius; y++) {
        for (int x = cx - radius; x <= cx + radius; x++) {
            if (map_in_bounds(m, x, y) && m->tiles[x][y] == TILE_FLOOR) {
                int dist = (x - cx) * (x - cx) + (y - cy) * (y - cy);
                if (dist <= radius * radius && rng_range(rng, 0, 100) < density) {
                    m->tiles[x][y] = tile;
                }
            }
        }
    }
}

void map_carve_room_with_tiles(Map *m, int x, int y, int w, int h,
                                char floor_tile, char wall_tile) {
    for (int r = y - 1; r <= y + h; r++) {
        for (int c = x - 1; c <= x + w; c++) {
            if (!map_in_bounds(m, c, r)) continue;
            if (r == y - 1 || r == y + h || c == x - 1 || c == x + w) {
                m->tiles[c][r] = wall_tile;
            } else {
                m->tiles[c][r] = floor_tile;
            }
        }
    }
}

void map_carve_river(Map *m, int sx, int sy, int ex, int ey, int width, RNGState *rng) {
    int cx = sx, cy = sy;
    while (cx != ex || cy != ey) {
        for (int w = -width / 2; w <= width / 2; w++) {
            if (cx + w >= 0 && cx + w < m->width && cy >= 0 && cy < m->height) {
                m->tiles[cx + w][cy] = TILE_WATER;
            }
        }
        if (cx < ex && (rng_range(rng, 0, 100) < 60 || cy == ey)) cx++;
        else if (cx > ex && (rng_range(rng, 0, 100) < 60 || cy == ey)) cx--;
        else if (cy < ey) cy++;
        else if (cy > ey) cy--;
    }
}

bool map_is_adjacent_to(const Map *m, int x, int y, char tile) {
    if (!map_in_bounds(m, x, y)) return false;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            int nx = x + dx, ny = y + dy;
            if (map_in_bounds(m, nx, ny) && m->tiles[nx][ny] == tile)
                return true;
        }
    }
    return false;
}

int map_find_nearest_tile(const Map *m, int x, int y, char tile, int max_dist,
                           int *ox, int *oy) {
    for (int d = 1; d <= max_dist; d++) {
        for (int dy = -d; dy <= d; dy++) {
            for (int dx = -d; dx <= d; dx++) {
                if (abs(dx) != d && abs(dy) != d) continue;
                int nx = x + dx, ny = y + dy;
                if (map_in_bounds(m, nx, ny) && m->tiles[nx][ny] == tile) {
                    if (ox) *ox = nx;
                    if (oy) *oy = ny;
                    return d;
                }
            }
        }
    }
    return -1;
}

void map_add_tile_flags(Map *m, int x, int y, uint32_t flags) {
    if (map_in_bounds(m, x, y))
        m->flags[x][y] |= flags;
}

void map_remove_tile_flags(Map *m, int x, int y, uint32_t flags) {
    if (map_in_bounds(m, x, y))
        m->flags[x][y] &= ~flags;
}

bool map_has_tile_flags(const Map *m, int x, int y, uint32_t flags) {
    if (!map_in_bounds(m, x, y)) return false;
    return (m->flags[x][y] & flags) == flags;
}

void map_set_light(Map *m, int x, int y, int intensity) {
    if (map_in_bounds(m, x, y))
        m->light[x][y] = intensity;
}

int map_get_light(const Map *m, int x, int y) {
    if (!map_in_bounds(m, x, y)) return 0;
    return m->light[x][y];
}
