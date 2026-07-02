#include "io/serializer.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int serializer_write_int32(FILE *f, int32_t val) {
    uint8_t buf[4];
    buf[0] = (uint8_t)(val >> 24);
    buf[1] = (uint8_t)(val >> 16);
    buf[2] = (uint8_t)(val >> 8);
    buf[3] = (uint8_t)(val);
    return fwrite(buf, 1, 4, f) == 4 ? 0 : -1;
}

int serializer_write_uint32(FILE *f, uint32_t val) {
    uint8_t buf[4];
    buf[0] = (uint8_t)(val >> 24);
    buf[1] = (uint8_t)(val >> 16);
    buf[2] = (uint8_t)(val >> 8);
    buf[3] = (uint8_t)(val);
    return fwrite(buf, 1, 4, f) == 4 ? 0 : -1;
}

int serializer_write_int16(FILE *f, int16_t val) {
    uint8_t buf[2];
    buf[0] = (uint8_t)(val >> 8);
    buf[1] = (uint8_t)(val);
    return fwrite(buf, 1, 2, f) == 2 ? 0 : -1;
}

int serializer_write_uint16(FILE *f, uint16_t val) {
    uint8_t buf[2];
    buf[0] = (uint8_t)(val >> 8);
    buf[1] = (uint8_t)(val);
    return fwrite(buf, 1, 2, f) == 2 ? 0 : -1;
}

int serializer_write_int8(FILE *f, int8_t val) {
    return fwrite(&val, 1, 1, f) == 1 ? 0 : -1;
}

int serializer_write_uint8(FILE *f, uint8_t val) {
    return fwrite(&val, 1, 1, f) == 1 ? 0 : -1;
}

int serializer_write_raw(FILE *f, const void *data, size_t len) {
    return fwrite(data, 1, len, f) == len ? 0 : -1;
}

int serializer_write_string(FILE *f, const char *str) {
    uint16_t len = (uint16_t)strlen(str);
    if (serializer_write_uint16(f, len) != 0) return -1;
    return fwrite(str, 1, len, f) == len ? 0 : -1;
}

int serializer_read_int32(FILE *f, int32_t *val) {
    uint8_t buf[4];
    if (fread(buf, 1, 4, f) != 4) return -1;
    *val = (int32_t)((uint32_t)buf[0] << 24 | (uint32_t)buf[1] << 16 |
           (uint32_t)buf[2] << 8 | buf[3]);
    return 0;
}

int serializer_read_uint32(FILE *f, uint32_t *val) {
    uint8_t buf[4];
    if (fread(buf, 1, 4, f) != 4) return -1;
    *val = (uint32_t)buf[0] << 24 | (uint32_t)buf[1] << 16 |
           (uint32_t)buf[2] << 8 | buf[3];
    return 0;
}

int serializer_read_int16(FILE *f, int16_t *val) {
    uint8_t buf[2];
    if (fread(buf, 1, 2, f) != 2) return -1;
    *val = (int16_t)((uint16_t)buf[0] << 8 | buf[1]);
    return 0;
}

int serializer_read_uint16(FILE *f, uint16_t *val) {
    uint8_t buf[2];
    if (fread(buf, 1, 2, f) != 2) return -1;
    *val = (uint16_t)buf[0] << 8 | buf[1];
    return 0;
}

int serializer_read_int8(FILE *f, int8_t *val) {
    return fread(val, 1, 1, f) == 1 ? 0 : -1;
}

int serializer_read_uint8(FILE *f, uint8_t *val) {
    return fread(val, 1, 1, f) == 1 ? 0 : -1;
}

int serializer_read_raw(FILE *f, void *data, size_t len) {
    return fread(data, 1, len, f) == len ? 0 : -1;
}

int serializer_read_string(FILE *f, char *str, size_t max_len) {
    uint16_t len;
    if (serializer_read_uint16(f, &len) != 0) return -1;
    if (len >= max_len) return -1;
    if (fread(str, 1, len, f) != len) return -1;
    str[len] = '\0';
    return 0;
}

int serializer_save_game(const World *w, const GameStats *stats, const char *path) {
    return state_save_to_file(w, stats, path);
}

int serializer_load_game(World *w, GameStats *stats, const char *path) {
    return state_load_from_file(w, stats, path);
}
