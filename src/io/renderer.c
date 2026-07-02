#include "io/renderer.h"
#include <string.h>
#include <stdio.h>

void render_init(RenderBuffer *rb, int w, int h) {
    if (w > RENDER_BUF_W) w = RENDER_BUF_W;
    if (h > RENDER_BUF_H) h = RENDER_BUF_H;
    rb->width = w;
    rb->height = h;
    rb->offset_x = 0;
    rb->offset_y = 0;
    render_clear(rb, ' ');
}

void render_clear(RenderBuffer *rb, char ch) {
    for (int y = 0; y < rb->height; y++)
        for (int x = 0; x < rb->width; x++)
            rb->chars[y][x] = ch;
    memset(rb->colors, 0, sizeof(int) * RENDER_BUF_H * RENDER_BUF_W);
}

void render_map(RenderBuffer *rb, const Map *map, int px, int py, int radius) {
    int half_w = rb->width / 2;
    int half_h = rb->height / 2;

    for (int sy = 0; sy < rb->height; sy++) {
        for (int sx = 0; sx < rb->width; sx++) {
            int mx = px + (sx - half_w);
            int my = py + (sy - half_h);

            if (!map_in_bounds(map, mx, my)) {
                rb->chars[sy][sx] = ' ';
                continue;
            }

            if (map->visible[mx][my]) {
                rb->chars[sy][sx] = map->tiles[mx][my];
            } else if (map->explored[mx][my]) {
                rb->chars[sy][sx] = map->tiles[mx][my];
                if (rb->chars[sy][sx] == TILE_FLOOR) rb->chars[sy][sx] = '.';
                else if (rb->chars[sy][sx] == TILE_WALL) rb->chars[sy][sx] = '#';
            } else {
                rb->chars[sy][sx] = ' ';
            }
        }
    }
}

void render_entities(RenderBuffer *rb, const Entity *player, const Entity *const *monsters, int mcount, const Map *map) {
    int half_w = rb->width / 2;
    int half_h = rb->height / 2;

    if (player && player->alive) {
        int sx = (player->x - (player->x - half_w)) + (half_w - 0);
        int sy = (player->y - (player->y - half_h)) + (half_h - 0);

        sx = half_w;
        sy = half_h;

        rb->chars[sy][sx] = '@';
        rb->colors[sy][sx] = 1;
    }

    if (!monsters) return;

    for (int i = 0; i < mcount; i++) {
        const Entity *m = monsters[i];
        if (!m || !m->alive) continue;

        int sx = half_w + (m->x - player->x);
        int sy = half_h + (m->y - player->y);

        if (sx < 0 || sx >= rb->width || sy < 0 || sy >= rb->height) continue;

        if (map && map_is_visible(map, m->x, m->y)) {
            rb->chars[sy][sx] = m->glyph;
            rb->colors[sy][sx] = 2;
        }
    }
}

void render_items(RenderBuffer *rb, const Item *const *items, int icount, const Map *map) {
    (void)items;
    (void)icount;
    (void)map;
    (void)rb;
}

void render_put_char(RenderBuffer *rb, int x, int y, char ch) {
    if (x >= 0 && x < rb->width && y >= 0 && y < rb->height)
        rb->chars[y][x] = ch;
}

void render_put_string(RenderBuffer *rb, int x, int y, const char *str) {
    int i = 0;
    while (str[i] && x + i < rb->width) {
        render_put_char(rb, x + i, y, str[i]);
        i++;
    }
}

void render_draw_border(RenderBuffer *rb, int x, int y, int w, int h) {
    for (int cx = x; cx < x + w && cx < rb->width; cx++) {
        if (y >= 0 && y < rb->height) rb->chars[y][cx] = '-';
        if (y + h - 1 >= 0 && y + h - 1 < rb->height) rb->chars[y + h - 1][cx] = '-';
    }
    for (int cy = y; cy < y + h && cy < rb->height; cy++) {
        if (x >= 0 && x < rb->width) rb->chars[cy][x] = '|';
        if (x + w - 1 >= 0 && x + w - 1 < rb->width) rb->chars[cy][x + w - 1] = '|';
    }
    if (x >= 0 && y >= 0 && x < rb->width && y < rb->height) rb->chars[y][x] = '+';
    if (x + w - 1 >= 0 && y >= 0 && x + w - 1 < rb->width && y < rb->height) rb->chars[y][x + w - 1] = '+';
    if (x >= 0 && y + h - 1 >= 0 && x < rb->width && y + h - 1 < rb->height) rb->chars[y + h - 1][x] = '+';
    if (x + w - 1 >= 0 && y + h - 1 >= 0 && x + w - 1 < rb->width && y + h - 1 < rb->height) rb->chars[y + h - 1][x + w - 1] = '+';
}

void render_fill_rect(RenderBuffer *rb, int x, int y, int w, int h, char ch) {
    for (int cy = y; cy < y + h && cy < rb->height; cy++)
        for (int cx = x; cx < x + w && cx < rb->width; cx++)
            rb->chars[cy][cx] = ch;
}

void render_draw_bar(RenderBuffer *rb, int x, int y, int w, int val, int max, char fill, char empty) {
    int filled = max > 0 ? (val * w) / max : 0;
    if (filled > w) filled = w;
    for (int i = 0; i < w && x + i < rb->width; i++) {
        if (i < filled) rb->chars[y][x + i] = fill;
        else rb->chars[y][x + i] = empty;
    }
}

void render_blit(RenderBuffer *rb, int dx, int dy, const RenderBuffer *src) {
    for (int sy = 0; sy < src->height && dy + sy < rb->height; sy++)
        for (int sx = 0; sx < src->width && dx + sx < rb->width; sx++)
            if (src->chars[sy][sx] != ' ')
                rb->chars[dy + sy][dx + sx] = src->chars[sy][sx];
}

void render_scroll_to(RenderBuffer *rb, int target_x, int target_y, int view_w, int view_h) {
    rb->offset_x = target_x - view_w / 2;
    rb->offset_y = target_y - view_h / 2;
}

void render_draw_minimap(RenderBuffer *rb, const Map *map, int px, int py, int w, int h) {
    if (w > rb->width) w = rb->width;
    if (h > rb->height) h = rb->height;

    for (int my = 0; my < h; my++) {
        for (int mx = 0; mx < w; mx++) {
            int mx2 = px - w / 2 + mx;
            int my2 = py - h / 2 + my;
            if (map_in_bounds(map, mx2, my2)) {
                if (mx2 == px && my2 == py) {
                    rb->chars[my][mx] = '@';
                } else if (map->explored[mx2][my2]) {
                    char t = map->tiles[mx2][my2];
                    if (t == TILE_FLOOR || t == TILE_DOOR) rb->chars[my][mx] = '.';
                    else if (t == TILE_STAIRS_DOWN || t == TILE_STAIRS_UP) rb->chars[my][mx] = '>';
                    else rb->chars[my][mx] = '#';
                } else {
                    rb->chars[my][mx] = ' ';
                }
            } else {
                rb->chars[my][mx] = ' ';
            }
        }
    }
}
