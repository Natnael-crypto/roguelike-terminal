#include "engine/dialogue.h"
#include "engine/inventory.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void dialogue_init(Dialogue *d, const char *npc_name) {
    memset(d, 0, sizeof(Dialogue));
    strncpy(d->npc_name, npc_name, MAX_NAME_LEN - 1);
    d->node_count = 0;
    d->current_node = 0;
    d->active = false;
}

int dialogue_add_node(Dialogue *d, int node_id, const char *text) {
    if (d->node_count >= MAX_DIALOGUE_NODES) return -1;
    DialogueNode *n = &d->nodes[d->node_count];
    n->node_id = node_id;
    strncpy(n->text, text, MAX_DIALOGUE_TEXT - 1);
    n->response_count = 0;
    n->quest_id = -1;
    n->shop_id = -1;
    n->is_shop = false;
    n->is_quest_giver = false;
    n->is_trade = false;
    d->node_count++;
    return d->node_count - 1;
}

int dialogue_add_response(Dialogue *d, int node_id, const char *response, int next_node) {
    for (int i = 0; i < d->node_count; i++) {
        if (d->nodes[i].node_id == node_id) {
            DialogueNode *n = &d->nodes[i];
            if (n->response_count >= MAX_RESPONSES) return -1;
            strncpy(n->responses[n->response_count], response, MAX_RESPONSE_TEXT - 1);
            n->next_nodes[n->response_count] = next_node;
            n->response_count++;
            return n->response_count - 1;
        }
    }
    return -1;
}

void dialogue_start(Dialogue *d, int start_node) {
    d->current_node = start_node;
    d->active = true;
}

int dialogue_choose(Dialogue *d, int choice) {
    if (!d || !d->active) return -1;
    for (int i = 0; i < d->node_count; i++) {
        if (d->nodes[i].node_id == d->current_node) {
            DialogueNode *n = &d->nodes[i];
            if (choice < 0 || choice >= n->response_count) return -1;
            d->current_node = n->next_nodes[choice];
            if (d->current_node < 0) d->active = false;
            return d->current_node;
        }
    }
    return -1;
}

const char *dialogue_get_text(const Dialogue *d) {
    if (!d || !d->active) return "";
    for (int i = 0; i < d->node_count; i++) {
        if (d->nodes[i].node_id == d->current_node)
            return d->nodes[i].text;
    }
    return "";
}

int dialogue_get_response_count(const Dialogue *d) {
    if (!d || !d->active) return 0;
    for (int i = 0; i < d->node_count; i++) {
        if (d->nodes[i].node_id == d->current_node)
            return d->nodes[i].response_count;
    }
    return 0;
}

const char *dialogue_get_response(const Dialogue *d, int index) {
    if (!d || !d->active) return "";
    for (int i = 0; i < d->node_count; i++) {
        if (d->nodes[i].node_id == d->current_node) {
            if (index >= 0 && index < d->nodes[i].response_count)
                return d->nodes[i].responses[index];
        }
    }
    return "";
}

bool dialogue_is_active(const Dialogue *d) {
    return d && d->active;
}

void quest_init(Quest *q, uint32_t id, QuestType type, const char *name, const char *desc, int target, int count) {
    memset(q, 0, sizeof(Quest));
    q->id = id;
    q->type = type;
    strncpy(q->name, name, MAX_QUEST_NAME - 1);
    strncpy(q->description, desc, MAX_QUEST_DESC - 1);
    q->target_id = target;
    q->target_count = count;
    q->current_count = 0;
    q->active = true;
    q->completed = false;
    q->turned_in = false;
    q->turn_deadline = 0;
}

int quest_progress(Quest *q, int amount) {
    if (!q || !q->active || q->completed) return -1;
    q->current_count += amount;
    if (q->current_count >= q->target_count) {
        q->completed = true;
        return 1;
    }
    return 0;
}

bool quest_is_complete(const Quest *q) {
    return q && q->completed && !q->turned_in;
}

int quest_reward(Quest *q, Entity *player) {
    if (!q || !player || !q->completed) return -1;
    player->gold += q->reward_gold;
    player->experience += q->reward_exp;
    q->turned_in = true;
    if (q->reward_item.type != 0) {
        Item *it = item_create(q->reward_item.name, q->reward_item.glyph, q->reward_item.type);
        if (it) {
            it->damage_dice = q->reward_item.damage_dice;
            it->damage_sides = q->reward_item.damage_sides;
            it->armor_value = q->reward_item.armor_value;
            it->heal_amount = q->reward_item.heal_amount;
            inventory_add(player, it);
        }
    }
    return q->reward_gold;
}

void quest_update_from_kill(Quest *q, int monster_id) {
    if (q->active && !q->completed && q->type == QT_KILL_MONSTERS) {
        if (monster_id == q->target_id || q->target_id == 0)
            quest_progress(q, 1);
    }
}

void quest_update_from_item(Quest *q, int item_id) {
    if (q->active && !q->completed && q->type == QT_COLLECT_ITEMS) {
        if (item_id == q->target_id || q->target_id == 0)
            quest_progress(q, 1);
    }
}

int quest_check_completion(Quest *q, const World *w) {
    (void)w;
    if (!q || !q->active) return 0;
    return q->completed ? 1 : 0;
}

const char *quest_type_name(QuestType type) {
    switch (type) {
        case QT_KILL_MONSTERS: return "Slay";
        case QT_COLLECT_ITEMS: return "Collect";
        case QT_REACH_DEPTH: return "Explore";
        case QT_CLEAR_ROOM: return "Cleanse";
        case QT_DELIVER_ITEM: return "Deliver";
        case QT_ESCORT_NPC: return "Escort";
        default: return "Quest";
    }
}

int quest_generate_random(Quest *q, uint32_t id, int player_level, RNGState *rng) {
    QuestType types[] = {QT_KILL_MONSTERS, QT_COLLECT_ITEMS, QT_REACH_DEPTH};
    QuestType type = types[rng_range(rng, 0, 3)];
    char name[64], desc[256];
    int target = 0, count = 1;

    switch (type) {
        case QT_KILL_MONSTERS:
            count = 3 + player_level;
            snprintf(name, sizeof(name), "Slay %d monsters", count);
            snprintf(desc, sizeof(desc), "The local town needs you to eliminate %d foul creatures infesting the dungeon.", count);
            target = 0;
            break;
        case QT_COLLECT_ITEMS:
            count = 1 + player_level / 2;
            snprintf(name, sizeof(name), "Collect %d items", count);
            snprintf(desc, sizeof(desc), "A merchant requires %d rare components from the depths below.", count);
            target = 0;
            break;
        case QT_REACH_DEPTH:
            target = player_level + 2;
            count = 1;
            snprintf(name, sizeof(name), "Reach depth %d", target);
            snprintf(desc, sizeof(desc), "Explore deeper into the dungeon, reaching level %d and beyond.", target);
            break;
        default:
            snprintf(name, sizeof(name), "Help the town");
            snprintf(desc, sizeof(desc), "Complete a task for the town.");
            break;
    }

    quest_init(q, id, type, name, desc, target, count);
    q->reward_gold = (player_level * 10) + rng_range(rng, 5, 20);
    q->reward_exp = (player_level * 15) + rng_range(rng, 10, 30);
    return 0;
}

void dialogue_generate_greeting(Dialogue *d, const char *npc_name, bool has_quest, bool is_merchant) {
    dialogue_init(d, npc_name);

    char greeting[MAX_DIALOGUE_TEXT];
    snprintf(greeting, sizeof(greeting), "Hello, adventurer. I am %s.", npc_name);
    dialogue_add_node(d, 0, greeting);

    char farewell[MAX_DIALOGUE_TEXT];
    snprintf(farewell, sizeof(farewell), "Safe travels, %s.", npc_name);
    dialogue_add_node(d, 99, farewell);

    dialogue_add_response(d, 0, "Goodbye.", 99);

    if (has_quest) {
        dialogue_add_node(d, 10, "I have a task that needs doing. Are you interested?");
        dialogue_add_response(d, 0, "Tell me about the task.", 10);
        dialogue_add_response(d, 10, "I'll take it!", 99);
        dialogue_add_response(d, 10, "Not right now.", 0);
    }

    if (is_merchant) {
        dialogue_add_node(d, 20, "Take a look at my wares. I have the finest goods in the realm.");
        dialogue_add_response(d, 0, "Show me your wares.", 20);
        dialogue_add_response(d, 20, "I'll buy something.", 99);
        dialogue_add_response(d, 20, "Maybe later.", 0);
    }
}

void dialogue_generate_merchant(Dialogue *d, const char *npc_name) {
    dialogue_generate_greeting(d, npc_name, false, true);
    char offer[MAX_DIALOGUE_TEXT];
    snprintf(offer, sizeof(offer), "%s nods and spreads their wares on the counter.", npc_name);
    dialogue_add_node(d, 21, offer);
    dialogue_add_response(d, 0, "Browse wares.", 21);
    dialogue_add_response(d, 21, "Make a purchase.", 99);
    dialogue_add_response(d, 21, "I'm just looking.", 99);
}

void dialogue_generate_quest_giver(Dialogue *d, const char *npc_name, const Quest *q) {
    dialogue_generate_greeting(d, npc_name, true, false);
    dialogue_add_node(d, 11, q->description);
    dialogue_add_response(d, 0, "What's the quest?", 11);
    dialogue_add_response(d, 11, "I accept!", 99);
    dialogue_add_response(d, 11, "Maybe another time.", 0);
}

/* === NPC-Specific Dialogue Trees === */

void dialogue_generate_blacksmith(Dialogue *d, const char *npc_name) {
    dialogue_init(d, npc_name);
    dialogue_add_node(d, 0, "Welcome to my smithy! I forge the finest weapons and armor in the realm.");
    dialogue_add_node(d, 1, "I can repair your equipment for a fee, or craft something new.");
    dialogue_add_node(d, 2, "Here are my current wares. Quality steel, each piece.");
    dialogue_add_node(d, 3, "Bring me rare ores and I can forge legendary items for you.");
    dialogue_add_node(d, 99, "Stay safe out there, adventurer. Come back if you need an edge.");
    
    dialogue_add_response(d, 0, "Show me your weapons.", 2);
    dialogue_add_response(d, 0, "Can you repair my gear?", 1);
    dialogue_add_response(d, 0, "What about legendary crafting?", 3);
    dialogue_add_response(d, 0, "Goodbye.", 99);
    dialogue_add_response(d, 1, "Yes please, repair everything.", 99);
    dialogue_add_response(d, 1, "Maybe later.", 0);
    dialogue_add_response(d, 2, "I'll take a look.", 99);
    dialogue_add_response(d, 2, "Not today, thanks.", 0);
    dialogue_add_response(d, 3, "What ores do you need?", 99);
    dialogue_add_response(d, 3, "I'll keep an eye out.", 0);
}

void dialogue_generate_wizard(Dialogue *d, const char *npc_name) {
    dialogue_init(d, npc_name);
    { char buf[MAX_DIALOGUE_TEXT]; snprintf(buf, sizeof(buf), "Ah, a seeker of arcane knowledge! I am %s, keeper of the crystal archives.", npc_name); dialogue_add_node(d, 0, buf); }
    dialogue_add_node(d, 1, "I can identify magical items for you, or teach you a cantrip or two.");
    dialogue_add_node(d, 2, "The ancient tomes speak of a hidden vault beneath the dungeon. Few have returned.");
    dialogue_add_node(d, 3, "Let me channel my energy to reveal the secrets of your belongings.");
    dialogue_add_node(d, 4, "In the deep halls, there lies the Crystal of Eternity. That would be a worthy quest.");
    dialogue_add_node(d, 99, "May the stars guide your path, friend.");
    
    dialogue_add_response(d, 0, "Identify my items, please.", 3);
    dialogue_add_response(d, 0, "Teach me a spell.", 1);
    dialogue_add_response(d, 0, "Tell me about the dungeon.", 2);
    dialogue_add_response(d, 0, "Goodbye.", 99);
    dialogue_add_response(d, 1, "I'm ready to learn!", 99);
    dialogue_add_response(d, 1, "Not now.", 0);
    dialogue_add_response(d, 2, "Tell me more.", 4);
    dialogue_add_response(d, 2, "I'll see for myself.", 0);
    dialogue_add_response(d, 3, "Here are my items.", 99);
    dialogue_add_response(d, 3, "On second thought, not yet.", 0);
    dialogue_add_response(d, 4, "I accept this quest!", 99);
    dialogue_add_response(d, 4, "Sounds too dangerous.", 0);
}

void dialogue_generate_priest(Dialogue *d, const char *npc_name) {
    dialogue_init(d, npc_name);
    { char buf[MAX_DIALOGUE_TEXT]; snprintf(buf, sizeof(buf), "Blessings of the gods upon you, traveler. I am %s, servant of the divine light.", npc_name); dialogue_add_node(d, 0, buf); }
    dialogue_add_node(d, 1, "Let me offer a prayer for your recovery. The light heals all wounds.");
    dialogue_add_node(d, 2, "The darkness grows in the depths. Ancient evils stir once more.");
    dialogue_add_node(d, 3, "I can bless your equipment to help against the undead hordes.");
    dialogue_add_node(d, 4, "A sacred relic was stolen from our temple. Retrieve it, and you shall be rewarded.");
    dialogue_add_node(d, 99, "Go with the gods' protection, child.");
    
    dialogue_add_response(d, 0, "Heal me, please.", 1);
    dialogue_add_response(d, 0, "Bless my equipment.", 3);
    dialogue_add_response(d, 0, "Tell me of the darkness.", 2);
    dialogue_add_response(d, 0, "Goodbye.", 99);
    dialogue_add_response(d, 1, "I accept your healing.", 99);
    dialogue_add_response(d, 1, "I'm fine for now.", 0);
    dialogue_add_response(d, 2, "Can I help?", 4);
    dialogue_add_response(d, 2, "I'll be careful.", 0);
    dialogue_add_response(d, 3, "Bless me!", 99);
    dialogue_add_response(d, 3, "Not needed.", 0);
    dialogue_add_response(d, 4, "I'll retrieve the relic!", 99);
    dialogue_add_response(d, 4, "I'm not ready for that.", 0);
}

void dialogue_generate_town_guard(Dialogue *d, const char *npc_name) {
    dialogue_init(d, npc_name);
    { char buf[MAX_DIALOGUE_TEXT]; snprintf(buf, sizeof(buf), "Halt! ... Oh, it's just you. %s, town watch. Keep your wits about you down there.", npc_name); dialogue_add_node(d, 0, buf); }
    dialogue_add_node(d, 1, "We've lost three good men to those caves. The goblins are getting bolder.");
    dialogue_add_node(d, 2, "If you clear out the goblin nest on level 3, I'll put in a good word with the quartermaster.");
    dialogue_add_node(d, 3, "Watch out for traps. The old tunnels are riddled with them.");
    dialogue_add_node(d, 4, "Heard rumors of a hidden treasure room on level 7. Could be worth investigating.");
    dialogue_add_node(d, 99, "Stay sharp, and don't die down there.");
    
    dialogue_add_response(d, 0, "What can you tell me?", 1);
    dialogue_add_response(d, 0, "Any quests?", 2);
    dialogue_add_response(d, 0, "Advice?", 3);
    dialogue_add_response(d, 0, "Goodbye, guard.", 99);
    dialogue_add_response(d, 1, "I'll handle the goblins.", 2);
    dialogue_add_response(d, 1, "Good to know.", 0);
    dialogue_add_response(d, 2, "I'll clear them out.", 99);
    dialogue_add_response(d, 2, "Not my problem.", 0);
    dialogue_add_response(d, 3, "Thanks for the warning.", 0);
    dialogue_add_response(d, 4, "I'll check it out.", 99);
    dialogue_add_response(d, 4, "Sounds like a rumor.", 0);
}

void dialogue_generate_farmer(Dialogue *d, const char *npc_name) {
    dialogue_init(d, npc_name);
    { char buf[MAX_DIALOGUE_TEXT]; snprintf(buf, sizeof(buf), "Oi there, adventurer! %s's the name. Monsters keep trampling my crops at night.", npc_name); dialogue_add_node(d, 0, buf); }
    dialogue_add_node(d, 1, "If you could thin out the monster population, I'd be much obliged.");
    dialogue_add_node(d, 2, "I've got some supplies that might help you on your journey.");
    dialogue_add_node(d, 3, "Those giant rats eat my grain. Kill a dozen and I'll reward you handsomely.");
    dialogue_add_node(d, 4, "My cellar leads to an old tunnel. Haven't dared explore it myself.");
    dialogue_add_node(d, 99, "Good luck, friend! Mind the crops on your way out.");
    
    dialogue_add_response(d, 0, "I can help with the monsters.", 1);
    dialogue_add_response(d, 0, "Got any supplies?", 2);
    dialogue_add_response(d, 0, "The giant rats?", 3);
    dialogue_add_response(d, 0, "Farewell.", 99);
    dialogue_add_response(d, 1, "Where are they?", 3);
    dialogue_add_response(d, 1, "Maybe another time.", 0);
    dialogue_add_response(d, 2, "Let's see what you have.", 99);
    dialogue_add_response(d, 2, "Nothing I need.", 0);
    dialogue_add_response(d, 3, "I'll hunt them down.", 99);
    dialogue_add_response(d, 3, "Not interested.", 0);
    dialogue_add_response(d, 4, "Tell me about the tunnel.", 99);
    dialogue_add_response(d, 4, "I'll leave it alone.", 0);
}

/* === Predefined Quest Templates === */

typedef struct {
    QuestType type;
    const char *name;
    const char *description;
    int target_count;
    int reward_gold_mult;
    int reward_exp_mult;
    int min_level;
} QuestTemplate;

static const QuestTemplate quest_templates[] = {
    {QT_KILL_MONSTERS, "Goblin Extermination", "The town guard needs you to cull the goblin population infesting the upper levels.", 8, 5, 10, 1},
    {QT_KILL_MONSTERS, "Rat Infestation", "The innkeeper's cellar is overrun with giant rats. Thin their numbers.", 10, 3, 5, 1},
    {QT_KILL_MONSTERS, "Skeleton Uprising", "Ancient skeletons have risen from their graves. Send them back.", 6, 6, 15, 3},
    {QT_KILL_MONSTERS, "Dragon Threat", "A young dragon has been terrorizing the lower levels. Slay it before it grows.", 1, 20, 50, 10},
    {QT_KILL_MONSTERS, "Spider Nest", "A giant spider has established a nest in the tunnels. Clear it out.", 5, 4, 8, 2},
    {QT_KILL_MONSTERS, "Troll Hunt", "Trolls have been attacking caravans on the mountain pass. Eliminate them.", 3, 8, 20, 5},
    {QT_KILL_MONSTERS, "Wraith Banishing", "Wraiths haunt the ancient crypt. Banish their unholy spirits.", 4, 7, 18, 4},
    
    {QT_COLLECT_ITEMS, "Mushroom Harvest", "The alchemist needs rare glowcap mushrooms from the deep caves.", 5, 4, 8, 1},
    {QT_COLLECT_ITEMS, "Crystal Collection", "A wizard requires crystal formations from the volcanic caves.", 3, 6, 15, 3},
    {QT_COLLECT_ITEMS, "Herb Gathering", "The healer needs medicinal herbs that only grow in the dark.", 8, 3, 6, 1},
    {QT_COLLECT_ITEMS, "Dragon Scales", "The blacksmith wants dragon scales to forge a legendary shield.", 3, 10, 25, 8},
    {QT_COLLECT_ITEMS, "Artifact Retrieval", "A scholar seeks ancient artifacts scattered throughout the dungeon.", 4, 8, 20, 5},
    
    {QT_REACH_DEPTH, "Explore the Depths", "Cartographers need someone to map the unknown lower levels of the dungeon.", 10, 10, 20, 1},
    {QT_REACH_DEPTH, "The Dark Below", "Legends speak of a city at the bottom of the dungeon. Prove it exists.", 15, 15, 30, 5},
    {QT_REACH_DEPTH, "Mountain's Heart", "Reach the deepest magma chambers and report what lies there.", 20, 20, 40, 10},
    
    {QT_CLEAR_ROOM, "Purge the Crypt", "The priests need a specific burial chamber cleansed of undead.", 1, 6, 12, 2},
    {QT_CLEAR_ROOM, "Clear the Fortress", "A goblin fortress blocks passage to the lower mines. Capture it.", 1, 8, 16, 3},
    {QT_CLEAR_ROOM, "Silence the Temple", "A corrupted temple spreads evil magic. Destroy the heart of the corruption.", 1, 12, 30, 6},
    
    {QT_DELIVER_ITEM, "Lost Package", "A merchant lost an important package somewhere in the dungeon. Find and return it.", 1, 5, 10, 1},
    {QT_DELIVER_ITEM, "Message to the Depths", "Carry an urgent message to the outpost on level 8.", 1, 7, 18, 4},
    {QT_DELIVER_ITEM, "Stolen Relic", "A sacred relic was stolen by a dark cult. Retrieve it from their hideout.", 1, 10, 25, 6},
};

static const int quest_template_count = sizeof(quest_templates) / sizeof(quest_templates[0]);

int quest_get_template_count(void) {
    return quest_template_count;
}

const char *quest_get_template_name(int idx) {
    if (idx < 0 || idx >= quest_template_count) return NULL;
    return quest_templates[idx].name;
}

int quest_generate_from_template(Quest *q, uint32_t id, int template_idx, int player_level, RNGState *rng) {
    if (template_idx < 0 || template_idx >= quest_template_count) return -1;
    const QuestTemplate *t = &quest_templates[template_idx];
    
    (void)player_level;
    
    int adjusted_count = t->target_count + (player_level / 3);
    int target_id = 0;
    if (t->type == QT_KILL_MONSTERS) {
        target_id = (player_level % 5) + 1;
    }
    
    quest_init(q, id, t->type, t->name, t->description, target_id, adjusted_count);
    q->reward_gold = adjusted_count * t->reward_gold_mult + rng_range(rng, 5, 20);
    q->reward_exp = adjusted_count * t->reward_exp_mult + rng_range(rng, 10, 30);
    q->turn_deadline = 200 + player_level * 50;
    return 0;
}

int quest_generate_random_quest(Quest *q, uint32_t id, int player_level, RNGState *rng) {
    int eligible[32];
    int eligible_count = 0;
    for (int i = 0; i < quest_template_count && eligible_count < 32; i++) {
        if (player_level >= quest_templates[i].min_level) {
            eligible[eligible_count++] = i;
        }
    }
    if (eligible_count == 0) {
        return quest_generate_random(q, id, player_level, rng);
    }
    int idx = eligible[rng_range(rng, 0, eligible_count)];
    return quest_generate_from_template(q, id, idx, player_level, rng);
}

int quest_count_active(const Quest *quests, int total) {
    int count = 0;
    for (int i = 0; i < total; i++) {
        if (quests[i].active && !quests[i].completed) count++;
    }
    return count;
}

int quest_count_completed(const Quest *quests, int total) {
    int count = 0;
    for (int i = 0; i < total; i++) {
        if (quests[i].completed && quests[i].turned_in) count++;
    }
    return count;
}

int quest_get_next_id(const Quest *quests, int total) {
    uint32_t max_id = 0;
    for (int i = 0; i < total; i++) {
        if (quests[i].id > max_id) max_id = quests[i].id;
    }
    return max_id + 1;
}
