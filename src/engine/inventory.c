#include "engine/inventory.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int inventory_add(Entity *owner, Item *it) {
    if (!owner || !it) return -1;
    if (owner->inventory_count >= owner->inventory_capacity) {
        int new_cap = owner->inventory_capacity + 16;
        void **new_inv = (void **)realloc(owner->inventory, sizeof(void *) * new_cap);
        if (!new_inv) return -1;
        owner->inventory = new_inv;
        owner->inventory_capacity = new_cap;
    }
    owner->inventory[owner->inventory_count++] = it;
    return 0;
}

int inventory_remove(Entity *owner, int slot) {
    if (!owner || slot < 0 || slot >= owner->inventory_count) return -1;
    for (int i = slot; i < owner->inventory_count - 1; i++) {
        owner->inventory[i] = owner->inventory[i + 1];
    }
    owner->inventory_count--;
    /* Update equipment slots: items above the removed slot shifted down by 1 */
    for (int s = 0; s < SLOT_COUNT; s++) {
        if (owner->equipment[s] == slot) {
            /* The equipped item was removed */
            owner->equipment[s] = -1;
        } else if (owner->equipment[s] > slot) {
            /* An equipped item shifted down */
            owner->equipment[s]--;
        }
    }
    return 0;
}

int inventory_find(const Entity *owner, uint32_t item_id) {
    if (!owner) return -1;
    for (int i = 0; i < owner->inventory_count; i++) {
        Item *it = (Item *)owner->inventory[i];
        if (it && it->id == item_id) return i;
    }
    return -1;
}

int inventory_count(const Entity *owner) {
    return owner ? owner->inventory_count : 0;
}

Item *inventory_get(const Entity *owner, int slot) {
    if (!owner || slot < 0 || slot >= owner->inventory_count) return NULL;
    return (Item *)owner->inventory[slot];
}

bool inventory_has_space(const Entity *owner, int count) {
    return owner && (owner->inventory_count + count <= owner->inventory_capacity);
}

int inventory_equip(Entity *owner, int slot) {
    if (!owner || slot < 0 || slot >= owner->inventory_count) return -1;
    Item *it = inventory_get(owner, slot);
    if (!it || !item_is_equippable(it)) return -1;

    int eslot = item_get_equip_slot(it);
    if (eslot < 0 || eslot >= SLOT_COUNT) return -1;

    if (owner->equipment[eslot] >= 0) {
        inventory_unequip(owner, eslot);
    }

    owner->equipment[eslot] = slot;
    it->equipped = true;
    return 0;
}

int inventory_unequip(Entity *owner, int equip_slot) {
    if (!owner || equip_slot < 0 || equip_slot >= SLOT_COUNT) return -1;
    int slot = owner->equipment[equip_slot];
    if (slot < 0) return -1;

    Item *it = inventory_get(owner, slot);
    if (it) it->equipped = false;
    owner->equipment[equip_slot] = -1;
    return 0;
}

int inventory_use(Entity *owner, int slot) {
    if (!owner || slot < 0 || slot >= owner->inventory_count) return -1;
    Item *it = inventory_get(owner, slot);
    if (!it || !item_is_usable(it)) return -1;

    if (it->type == ITEM_POTION) {
        entity_heal(owner, it->heal_amount);
        Item *used = (Item *)owner->inventory[slot];
        inventory_remove(owner, slot);
        item_free(used);
        return 1;
    }

    if (it->type == ITEM_FOOD) {
        entity_heal(owner, 5);
        Item *used = (Item *)owner->inventory[slot];
        inventory_remove(owner, slot);
        item_free(used);
        return 1;
    }

    return 0;
}

void inventory_drop_all(Entity *owner) {
    if (!owner) return;
    owner->inventory_count = 0;
    memset(owner->equipment, -1, sizeof(owner->equipment));
}

int inventory_get_equipped_count(const Entity *owner, EquipmentSlot slot) {
    if (!owner || slot < 0 || slot >= SLOT_COUNT) return 0;
    return owner->equipment[slot] >= 0 ? 1 : 0;
}
