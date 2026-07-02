#include "engine/pathfinding.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void pf_init(Pathfinder *pf) {
    memset(pf, 0, sizeof(Pathfinder));
    pf->generation = 1;
    pf->open_count = 0;
    pf->path_len = 0;
}

static int heuristic(int x1, int y1, int x2, int y2) {
    int dx = x1 > x2 ? x1 - x2 : x2 - x1;
    int dy = y1 > y2 ? y1 - y2 : y2 - y1;
    return (dx + dy) * 10;
}

int pf_find_path(Pathfinder *pf, const Map *map, int sx, int sy, int ex, int ey,
                 bool (*is_walkable)(const Map *, int, int)) {
    if (!map || !is_walkable || !map_in_bounds(map, sx, sy) || !map_in_bounds(map, ex, ey))
        return -1;
    if (sx == ex && sy == ey) return 0;

    if (pf->generation == 0) {
        memset(pf->nodes, 0, sizeof(pf->nodes));
        pf->generation = 1;
    } else {
        pf->generation++;
        if (pf->generation == 0) pf->generation = 1;
    }
    pf->open_count = 0;
    pf->path_len = 0;

    PFNode *start = &pf->nodes[sx][sy];
    start->x = sx;
    start->y = sy;
    start->g = 0;
    start->h = heuristic(sx, sy, ex, ey);
    start->f = start->g + start->h;
    start->parent = -1;
    start->closed = false;
    start->open = true;
    pf->open_list[pf->open_count++] = sx + sy * MAX_MAP_WIDTH;

    static const int dx[] = {0, 1, 0, -1, 1, 1, -1, -1};
    static const int dy[] = {-1, 0, 1, 0, -1, 1, 1, -1};
    static const int costs[] = {10, 10, 10, 10, 14, 14, 14, 14};
    int dirs = 4;

    while (pf->open_count > 0) {
        int best_idx = 0;
        int best_f = pf->nodes[pf->open_list[0] % MAX_MAP_WIDTH][pf->open_list[0] / MAX_MAP_WIDTH].f;
        for (int i = 1; i < pf->open_count; i++) {
            int idx = pf->open_list[i];
            int nx = idx % MAX_MAP_WIDTH;
            int ny = idx / MAX_MAP_WIDTH;
            if (pf->nodes[nx][ny].f < best_f) {
                best_f = pf->nodes[nx][ny].f;
                best_idx = i;
            }
        }

        int cur_idx = pf->open_list[best_idx];
        int cx = cur_idx % MAX_MAP_WIDTH;
        int cy = cur_idx / MAX_MAP_WIDTH;

        pf->open_list[best_idx] = pf->open_list[--pf->open_count];
        pf->nodes[cx][cy].open = false;
        pf->nodes[cx][cy].closed = true;

        if (cx == ex && cy == ey) {
            for (int p = cx + cy * MAX_MAP_WIDTH; p != -1; ) {
                int px = p % MAX_MAP_WIDTH;
                int py = p / MAX_MAP_WIDTH;
                pf->path[pf->path_len][0] = px;
                pf->path[pf->path_len][1] = py;
                pf->path_len++;
                if (pf->path_len >= PF_MAX_PATH) break;
                p = pf->nodes[px][py].parent;
            }
            for (int i = 0; i < pf->path_len / 2; i++) {
                int tmp_x = pf->path[i][0];
                int tmp_y = pf->path[i][1];
                pf->path[i][0] = pf->path[pf->path_len - 1 - i][0];
                pf->path[i][1] = pf->path[pf->path_len - 1 - i][1];
                pf->path[pf->path_len - 1 - i][0] = tmp_x;
                pf->path[pf->path_len - 1 - i][1] = tmp_y;
            }
            return pf->path_len;
        }

        for (int d = 0; d < dirs; d++) {
            int nx = cx + dx[d];
            int ny = cy + dy[d];

            if (!map_in_bounds(map, nx, ny)) continue;
            if (!is_walkable(map, nx, ny) && !(nx == ex && ny == ey)) continue;

            PFNode *neighbor = &pf->nodes[nx][ny];
            if (neighbor->closed) continue;

            int ng = pf->nodes[cx][cy].g + costs[d];

            if (!neighbor->open) {
                neighbor->x = nx;
                neighbor->y = ny;
                neighbor->g = ng;
                neighbor->h = heuristic(nx, ny, ex, ey);
                neighbor->f = ng + neighbor->h;
                neighbor->parent = cur_idx;
                neighbor->open = true;
                neighbor->closed = false;
                if (pf->open_count < PF_MAX_NODES) {
                    pf->open_list[pf->open_count++] = nx + ny * MAX_MAP_WIDTH;
                }
            } else if (ng < neighbor->g) {
                neighbor->g = ng;
                neighbor->f = ng + neighbor->h;
                neighbor->parent = cur_idx;
            }
        }
    }

    return -1;
}

int pf_get_path(const Pathfinder *pf, int step, int *x, int *y) {
    if (step < 0 || step >= pf->path_len) return -1;
    *x = pf->path[step][0];
    *y = pf->path[step][1];
    return 0;
}

bool pf_path_exists(const Pathfinder *pf) {
    return pf->path_len > 0;
}

int pf_path_length(const Pathfinder *pf) {
    return pf->path_len;
}

void pf_clear(Pathfinder *pf) {
    memset(pf, 0, sizeof(Pathfinder));
}
