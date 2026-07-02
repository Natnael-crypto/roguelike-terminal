# Roguelike Terminal RPG

A terminal-based roguelike dungeon crawler written in C with no external dependencies.

## Features

- Procedurally generated dungeons with rooms and corridors
- Field of view (FOV) with raycasting
- Turn-based combat with melee and ranged attacks
- Monster AI with multiple behavior types (wander, aggressive, coward, guard)
- Inventory system with items, equipment, and potions
- Character progression with leveling and stat growth
- Multiple dungeon levels with increasing difficulty
- Save/Load game state (binary format)
- Deterministic replay mode
- Debug runtime inspector
- ANSI color terminal output

## Build

Requirements: gcc or clang, make

```bash
make
```

Debug build with AddressSanitizer:
```bash
make asan
```

## Run

```bash
./roguelike
```

With options:
```bash
./roguelike --seed 12345 --depth 10 --debug
```

## Controls

| Key | Action |
|-----|--------|
| WASD | Move (up/down/left/right) |
| q/e/z/x | Diagonal movement |
| . | Wait (skip turn) |
| g | Pick up item |
| i | Open inventory |
| c | Character sheet |
| ? | Help screen |
| > | Descend stairs |
| < | Ascend stairs |
| Q | Quit game |
| ! | Debug inspector |

## Run Tests

```bash
make test
```

## Fuzz

```bash
make fuzz
./fuzz/roguelike_fuzzer fuzz/corpus/roguelike_fuzzer/
```

## Tools

```bash
make tools
./tools/stress_sim [games] [steps] [seed]
./tools/replay [replay_file]
```

## Known Limitations

- Map size is fixed at 80x25 tiles
- Maximum 128 monsters and 256 items on a single level
- AI pathfinding uses simple greedy approach, not full A* for monster movement
