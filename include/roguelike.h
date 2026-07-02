#ifndef ROGUELIKE_H
#define ROGUELIKE_H

#include "core/config.h"
#include "core/game.h"
#include "core/world.h"
#include "core/state.h"

#include "engine/map.h"
#include "engine/dungeon.h"
#include "engine/entity.h"
#include "engine/combat.h"
#include "engine/item.h"
#include "engine/inventory.h"
#include "engine/ai.h"
#include "engine/pathfinding.h"

#include "io/cli.h"
#include "io/renderer.h"
#include "io/input.h"
#include "io/serializer.h"
#include "io/logger.h"

#include "systems/memory.h"
#include "systems/rng.h"
#include "systems/event.h"
#include "systems/scheduler.h"

#endif
