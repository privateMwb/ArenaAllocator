#!/bin/bash -eu
# ============================================================
# .clusterfuzzlite/build.sh
#
# ArenaPro is header-only, so unlike a harness that needs to compile
# separate .cpp translation units first, this just compiles the fuzz
# target directly against the headers under include/.
#
# -UNDEBUG keeps the AP_PRE/AP_POST/AP_INVARIANT/AP_ASSERT contract
# macros (Contract.h) live regardless of what $CXXFLAGS contains, so a
# violated precondition aborts and is reported as a crash instead of
# silently compiling away.
#
# Add more `${SRC}/ArenaAllocator/fuzz/fuzz_*.cpp` harnesses here as
# they're added; each becomes its own $OUT binary.
# ============================================================

cd "${SRC}/ArenaAllocator"

$CXX $CXXFLAGS -std=c++20 -UNDEBUG \
  -I"${SRC}/ArenaAllocator/include" \
  fuzz/fuzz_arena.cpp \
  $LIB_FUZZING_ENGINE \
  -o "${OUT}/fuzz_arena"
