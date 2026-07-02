#include "systems/event.h"
#include <string.h>
#include <stdlib.h>

void event_queue_init(EventQueue *eq) {
    eq->head = 0;
    eq->tail = 0;
    eq->count = 0;
    eq->next_id = 1;
    eq->timestamp = 0;
    memset(eq->events, 0, sizeof(eq->events));
}

int event_queue_push(EventQueue *eq, const GameEvent *evt) {
    if (eq->count >= MAX_EVENTS) return -1;
    eq->events[eq->tail] = *evt;
    eq->events[eq->tail].timestamp = eq->timestamp++;
    eq->events[eq->tail].event_data = NULL;
    eq->events[eq->tail].own_data = false;
    eq->tail = (eq->tail + 1) % MAX_EVENTS;
    eq->count++;
    return 0;
}

int event_queue_push_heap(EventQueue *eq, const GameEvent *evt, void *data) {
    if (eq->count >= MAX_EVENTS) return -1;
    eq->events[eq->tail] = *evt;
    eq->events[eq->tail].timestamp = eq->timestamp++;
    eq->events[eq->tail].event_data = data;
    eq->events[eq->tail].own_data = true;
    eq->tail = (eq->tail + 1) % MAX_EVENTS;
    eq->count++;
    return 0;
}

void event_free_data(GameEvent *evt) {
    if (evt && evt->event_data && evt->own_data) {
        free(evt->event_data);
        evt->event_data = NULL;
        evt->own_data = false;
    }
}

int event_queue_pop(EventQueue *eq, GameEvent *evt) {
    if (eq->count <= 0) return -1;
    *evt = eq->events[eq->head];
    eq->head = (eq->head + 1) % MAX_EVENTS;
    eq->count--;
    return 0;
}

int event_queue_peek(const EventQueue *eq, GameEvent *evt) {
    if (eq->count <= 0) return -1;
    *evt = eq->events[eq->head];
    return 0;
}

int event_queue_count(const EventQueue *eq) {
    return eq->count;
}

void event_queue_clear(EventQueue *eq) {
    for (int i = 0; i < MAX_EVENTS; i++) {
        if (eq->events[i].event_data && eq->events[i].own_data) {
            free(eq->events[i].event_data);
            eq->events[i].event_data = NULL;
            eq->events[i].own_data = false;
        }
    }
    eq->head = 0;
    eq->tail = 0;
    eq->count = 0;
}

bool event_queue_has_type(const EventQueue *eq, EventType type) {
    for (int i = 0; i < eq->count; i++) {
        int idx = (eq->head + i) % MAX_EVENTS;
        if (eq->events[idx].type == type) return true;
    }
    return false;
}

int event_queue_remove_type(EventQueue *eq, EventType type) {
    int removed = 0;
    int new_head = eq->head;
    int new_count = 0;
    int temp[MAX_EVENTS];

    for (int i = 0; i < eq->count; i++) {
        int idx = (eq->head + i) % MAX_EVENTS;
        if (eq->events[idx].type == type) {
            removed++;
        } else {
            temp[new_count++] = idx;
        }
    }

    for (int i = 0; i < new_count; i++) {
        eq->events[(new_head + i) % MAX_EVENTS] = eq->events[temp[i]];
    }

    eq->head = new_head;
    eq->count = new_count;
    return removed;
}

GameEvent event_make(EventType type, uint32_t source, uint32_t target, int x, int y, int v1, int v2) {
    GameEvent evt;
    memset(&evt, 0, sizeof(evt));
    evt.type = type;
    evt.source_id = source;
    evt.target_id = target;
    evt.x = x;
    evt.y = y;
    evt.value1 = v1;
    evt.value2 = v2;
    evt.event_data = NULL;
    evt.own_data = false;
    return evt;
}
