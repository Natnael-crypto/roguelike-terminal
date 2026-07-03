CC      ?= gcc
CFLAGS  ?= -Wall -Wextra -Werror -std=c99 -g -O0 -Wno-unused-parameter
CFLAGS  += -Iinclude -Isrc
LDFLAGS ?= -lm

SRC_DIR    = src
CORE_DIR   = $(SRC_DIR)/core
ENGINE_DIR = $(SRC_DIR)/engine
IO_DIR     = $(SRC_DIR)/io
SYS_DIR    = $(SRC_DIR)/systems

SRCS = $(CORE_DIR)/game.c \
       $(CORE_DIR)/world.c \
       $(CORE_DIR)/state.c \
       $(CORE_DIR)/config.c \
       $(ENGINE_DIR)/map.c \
       $(ENGINE_DIR)/dungeon.c \
       $(ENGINE_DIR)/entity.c \
       $(ENGINE_DIR)/combat.c \
       $(ENGINE_DIR)/item.c \
       $(ENGINE_DIR)/inventory.c \
       $(ENGINE_DIR)/ai.c \
       $(ENGINE_DIR)/pathfinding.c \
       $(ENGINE_DIR)/magic.c \
       $(ENGINE_DIR)/trap.c \
       $(ENGINE_DIR)/shop.c \
       $(ENGINE_DIR)/ranged.c \
       $(ENGINE_DIR)/crafting.c \
       $(ENGINE_DIR)/dialogue.c \
       $(ENGINE_DIR)/equipment.c \
       $(IO_DIR)/cli.c \
       $(IO_DIR)/renderer.c \
       $(IO_DIR)/input.c \
       $(IO_DIR)/serializer.c \
       $(IO_DIR)/logger.c \
       $(SYS_DIR)/memory.c \
       $(SYS_DIR)/rng.c \
       $(SYS_DIR)/event.c \
       $(SYS_DIR)/scheduler.c \
       $(SRC_DIR)/main.c

OBJS = $(SRCS:.c=.o)
TARGET = roguelike

TEST_DIR = tests
FUZZ_DIR = fuzz
TOOLS_DIR = tools

.PHONY: all clean test fuzz asan debug tools

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)
	rm -f $(TEST_DIR)/*.o $(TEST_DIR)/test_*
	rm -f $(TOOLS_DIR)/*.o $(TOOLS_DIR)/stress_sim $(TOOLS_DIR)/replay
	rm -f $(FUZZ_DIR)/*.o $(FUZZ_DIR)/roguelike_fuzzer
	rm -f *.gcno *.gcda *.gcov

TEST_SRCS = $(wildcard $(TEST_DIR)/test_*.c)
TEST_BINS = $(TEST_SRCS:.c=)

test: $(TEST_BINS)
	@for t in $(TEST_BINS); do \
		echo "Running $$t..."; \
		./$$t || exit 1; \
	done
	@echo "All tests passed."

$(TEST_DIR)/test_%.o: $(TEST_DIR)/test_%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(TEST_DIR)/test_%: $(TEST_DIR)/test_%.o $(filter-out $(SRC_DIR)/main.o, $(OBJS))
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

asan: CFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer
asan: LDFLAGS += -fsanitize=address,undefined
asan: clean $(TARGET)

debug: CFLAGS += -DDEBUG -g3
debug: clean $(TARGET)

tools: $(TOOLS_DIR)/stress_sim $(TOOLS_DIR)/replay

$(TOOLS_DIR)/stress_sim: $(TOOLS_DIR)/stress_runner.c $(filter-out $(SRC_DIR)/main.o, $(OBJS))
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(TOOLS_DIR)/replay: $(TOOLS_DIR)/replay_analyzer.c $(filter-out $(SRC_DIR)/main.o, $(OBJS))
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

FUZZ_CC ?= clang
FUZZ_CFLAGS = -fsanitize=address,undefined -fno-omit-frame-pointer -g -O1 -std=c99 -Iinclude -Isrc -Wno-unused-parameter
FUZZ_CXXFLAGS = -fsanitize=fuzzer,address,undefined -g -O1 -std=c++17 -Iinclude -Isrc
FUZZ_CXX ?= clang++
FUZZ_OBJS = $(FUZZ_DIR)/fuzz_%.o

# Recompile every .o with ASan/UBSan for the fuzz target
FUZZ_OBJ_LIST = $(patsubst %.c,$(FUZZ_DIR)/%.o,$(filter-out $(SRC_DIR)/main.c, $(SRCS)))

$(FUZZ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(FUZZ_CC) $(FUZZ_CFLAGS) -c -o $@ $<

fuzz: $(FUZZ_DIR)/roguelike_fuzzer
	@echo "Fuzz target built: $(FUZZ_DIR)/roguelike_fuzzer"

$(FUZZ_DIR)/roguelike_fuzzer: $(FUZZ_DIR)/roguelike_fuzzer.c $(FUZZ_OBJ_LIST)
	$(FUZZ_CXX) $(FUZZ_CXXFLAGS) $< $(FUZZ_OBJ_LIST) -o $@ $(LDFLAGS)
