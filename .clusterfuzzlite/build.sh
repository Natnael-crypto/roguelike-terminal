#!/bin/bash -eu

# Build all C source files (excluding main.c) as objects with sanitizers
SOURCES=$(find "$SRC/src" -name '*.c' ! -name 'main.c' | sort)

OBJECTS=""
for src in $SOURCES; do
    obj="$OUT/$(echo "$src" | sed "s|$SRC/src/||; s|/|_|g; s|\.c$||").o"
    $CC $CFLAGS -fsanitize=address,undefined -fno-omit-frame-pointer -g -O1 -std=c99 \
        -I"$SRC/include" -I"$SRC/src" \
        -c "$src" -o "$obj"
    OBJECTS="$OBJECTS $obj"
done

# Build the fuzz harness
$CXX $CXXFLAGS -fsanitize=fuzzer,address,undefined -g -O1 -std=c++17 \
    -I"$SRC/include" -I"$SRC/src" \
    "$SRC/fuzz/roguelike_fuzzer.c" \
    $OBJECTS \
    -o "$OUT/roguelike_fuzzer" \
    -lm

# Package seed corpus
if [ -d "$SRC/fuzz/corpus/roguelike_fuzzer" ]; then
    zip -j "$OUT/roguelike_fuzzer_seed_corpus.zip" "$SRC/fuzz/corpus/roguelike_fuzzer/"*
fi
