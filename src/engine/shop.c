#include "engine/shop.h"
#include "engine/inventory.h"
#include "engine/item.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

const char *shop_merchant_name(MerchantType type) {
    switch (type) {
        case MT_GENERAL: return "General Store";
        case MT_WEAPONSMITH: return "Weaponsmith";
        case MT_ARMORER: return "Armorer";
        case MT_POTION_SELLER: return "Potion Seller";
        case MT_SCROLL_SELLER: return "Scroll Merchant";
        case MT_MAGIC_SHOP: return "Magic Emporium";
        default: return "Merchant";
    }
}

int shop_init_merchant(Merchant *m, MerchantType type, int x, int y, int level) {
    memset(m, 0, sizeof(Merchant));
    m->type = type;
    strncpy(m->name, shop_merchant_name(type), MAX_SHOP_NAME - 1);
    m->x = x;
    m->y = y;
    m->level = level;
    m->item_count = 0;
    m->hostile = false;
    m->faction_id = 1;

    switch (type) {
        case MT_GENERAL: m->price_modifier = 1.2f; break;
        case MT_WEAPONSMITH: m->price_modifier = 1.5f; break;
        case MT_ARMORER: m->price_modifier = 1.5f; break;
        case MT_POTION_SELLER: m->price_modifier = 1.3f; break;
        case MT_SCROLL_SELLER: m->price_modifier = 1.4f; break;
        case MT_MAGIC_SHOP: m->price_modifier = 2.0f; break;
        default: m->price_modifier = 1.0f; break;
    }

    memset(m->items_for_sale, -1, sizeof(m->items_for_sale));
    return 0;
}

void shop_generate_inventory(Merchant *m, int level, RNGState *rng) {
    m->item_count = 0;
    int count = 4 + rng_range(rng, 0, 4);
    if (count > MAX_SHOP_ITEMS) count = MAX_SHOP_ITEMS;

    for (int i = 0; i < count; i++) {
        m->items_for_sale[i] = i + 1;
        m->item_count++;
    }
}

int shop_get_price(const Merchant *m, int item_index, bool buying) {
    (void)item_index;
    int base = 10 + m->level * 5;
    float mult = buying ? m->price_modifier : (1.0f / m->price_modifier);
    return (int)(base * mult);
}

int shop_buy(Merchant *m, Entity *buyer, int item_index) {
    if (!m || !buyer || item_index < 0 || item_index >= m->item_count) return -1;
    if (m->items_for_sale[item_index] < 0) return -1;

    int price = shop_get_price(m, item_index, true);
    if (buyer->gold < price) return -1;

    buyer->gold -= price;

    Item *it = item_create("purchased item", '?', ITEM_SCROLL);
    if (!it) return -1;
    it->value = price / 2;

    if (m->type == MT_POTION_SELLER) {
        it->type = ITEM_POTION;
        it->name[0] = '\0';
        strcat(it->name, "health potion");
        it->glyph = '!';
        it->heal_amount = 10 + m->level * 3;
    } else if (m->type == MT_WEAPONSMITH) {
        it->type = ITEM_WEAPON;
        strcpy(it->name, "iron blade");
        it->glyph = '/';
        it->damage_dice = 1 + m->level / 3;
        it->damage_sides = 6 + m->level;
        it->damage_bonus = m->level / 2;
    } else if (m->type == MT_ARMORER) {
        it->type = ITEM_ARMOR;
        strcpy(it->name, "steel plate");
        it->glyph = '[';
        it->armor_value = 2 + m->level;
        it->armor_slot = SLOT_CHEST;
    }

    inventory_add(buyer, it);
    m->items_for_sale[item_index] = -1;
    return 0;
}

int shop_sell(Merchant *m, Entity *seller, int slot, Item *it) {
    if (!m || !seller || !it) return -1;
    if (m->item_count >= MAX_SHOP_ITEMS) return -1;

    int price = it->value / 2;
    if (price < 1) price = 1;

    seller->gold += price;
    inventory_remove(seller, slot);

    m->items_for_sale[m->item_count] = it->id;
    m->item_count++;
    return price;
}

void shop_print_inventory(const Merchant *m) {
    printf("%s's wares:\n", m->name);
    printf("  Price modifier: %.1fx\n", m->price_modifier);
    for (int i = 0; i < m->item_count; i++) {
        if (m->items_for_sale[i] >= 0) {
            int price = shop_get_price(m, i, true);
            printf("  %c) Item %d - %d gold\n", 'a' + i, i, price);
        }
    }
}

int shop_find_merchant_at(MerchantList *ml, int x, int y) {
    for (int i = 0; i < ml->count; i++) {
        if (ml->merchants[i].x == x && ml->merchants[i].y == y) return i;
    }
    return -1;
}

void merchant_list_init(MerchantList *ml) {
    ml->count = 0;
    memset(ml->merchants, 0, sizeof(ml->merchants));
}

int merchant_list_add(MerchantList *ml, MerchantType type, int x, int y, int level) {
    if (ml->count >= MAX_MERCHANTS) return -1;
    shop_init_merchant(&ml->merchants[ml->count], type, x, y, level);
    ml->count++;
    return 0;
}

int shop_trade(Entity *player, Merchant *m) {
    if (!player || !m) return -1;
    printf("\n=== Trading with %s ===\n", m->name);
    printf("Your gold: %d\n", player->gold);
    printf("1. Buy  2. Sell  3. Leave\n");
    return 0;
}
