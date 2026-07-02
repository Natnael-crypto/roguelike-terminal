#include "systems/memory.h"
#include <string.h>
#include <stdio.h>

void mem_tracker_init(MemTracker *mt) {
    mt->count = 0;
    mt->total_allocated = 0;
    mt->total_freed = 0;
    mt->peak_usage = 0;
    mt->tracking = true;
    memset(mt->entries, 0, sizeof(mt->entries));
}

void mem_tracker_destroy(MemTracker *mt) {
    if (mt->tracking) {
        mem_check_leaks(mt);
    }
}

void *mem_alloc(MemTracker *mt, size_t size, const char *file, int line) {
    void *ptr = malloc(size);
    if (!ptr && size > 0) return NULL;

    if (mt->tracking && mt->count < MEM_TRACK_MAX) {
        mt->entries[mt->count].ptr = ptr;
        mt->entries[mt->count].size = size;
        mt->entries[mt->count].file = file;
        mt->entries[mt->count].line = line;
        mt->entries[mt->count].freed = false;
        mt->count++;
        mt->total_allocated += size;
        if (mt->total_allocated - mt->total_freed > mt->peak_usage) {
            mt->peak_usage = mt->total_allocated - mt->total_freed;
        }
    }
    return ptr;
}

void *mem_calloc(MemTracker *mt, size_t nmemb, size_t size, const char *file, int line) {
    void *ptr = calloc(nmemb, size);
    if (!ptr && nmemb > 0 && size > 0) return NULL;

    if (mt->tracking && mt->count < MEM_TRACK_MAX) {
        size_t total = nmemb * size;
        mt->entries[mt->count].ptr = ptr;
        mt->entries[mt->count].size = total;
        mt->entries[mt->count].file = file;
        mt->entries[mt->count].line = line;
        mt->entries[mt->count].freed = false;
        mt->count++;
        mt->total_allocated += total;
        if (mt->total_allocated - mt->total_freed > mt->peak_usage) {
            mt->peak_usage = mt->total_allocated - mt->total_freed;
        }
    }
    return ptr;
}

void *mem_realloc(MemTracker *mt, void *ptr, size_t new_size, const char *file, int line) {
    if (mt->tracking) {
        for (int i = 0; i < mt->count; i++) {
            if (mt->entries[i].ptr == ptr && !mt->entries[i].freed) {
                mt->total_freed += mt->entries[i].size;
                mt->entries[i].freed = true;
                break;
            }
        }
    }

    void *new_ptr = realloc(ptr, new_size);
    if (!new_ptr && new_size > 0) return NULL;

    if (mt->tracking && new_ptr && mt->count < MEM_TRACK_MAX) {
        mt->entries[mt->count].ptr = new_ptr;
        mt->entries[mt->count].size = new_size;
        mt->entries[mt->count].file = file;
        mt->entries[mt->count].line = line;
        mt->entries[mt->count].freed = false;
        mt->count++;
        mt->total_allocated += new_size;
        if (mt->total_allocated - mt->total_freed > mt->peak_usage) {
            mt->peak_usage = mt->total_allocated - mt->total_freed;
        }
    }
    return new_ptr;
}

void mem_free(MemTracker *mt, void *ptr) {
    if (!ptr) return;

    if (mt->tracking) {
        for (int i = 0; i < mt->count; i++) {
            if (mt->entries[i].ptr == ptr && !mt->entries[i].freed) {
                mt->entries[i].freed = true;
                mt->total_freed += mt->entries[i].size;
                break;
            }
        }
    }
    free(ptr);
}

void mem_check_leaks(const MemTracker *mt) {
    int leaked = 0;
    size_t leaked_bytes = 0;
    for (int i = 0; i < mt->count; i++) {
        if (!mt->entries[i].freed && mt->entries[i].ptr != NULL) {
            leaked++;
            leaked_bytes += mt->entries[i].size;
        }
    }
    if (leaked > 0) {
        fprintf(stderr, "Memory leak: %d allocations (%zu bytes) not freed\n", leaked, leaked_bytes);
    }
}

int mem_count_allocations(const MemTracker *mt) {
    int active = 0;
    for (int i = 0; i < mt->count; i++) {
        if (!mt->entries[i].freed) active++;
    }
    return active;
}

size_t mem_total_usage(const MemTracker *mt) {
    size_t usage = 0;
    for (int i = 0; i < mt->count; i++) {
        if (!mt->entries[i].freed) usage += mt->entries[i].size;
    }
    return usage;
}

void mem_set_tracking(MemTracker *mt, bool track) {
    mt->tracking = track;
}

static int debug_level = 0;
