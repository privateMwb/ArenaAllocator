// Arena Lifecycle Benchmark Suite — Scope RAII
// Measures ArenaScope construction/destruction performance.
//
// ArenaScope is an Arena-only RAII wrapper around beginFrame()/
// endFrame() — std::pmr has no equivalent nested-checkpoint concept to
// wrap, so this runs solo.
//
// Covers:
// - constructing and immediately destroying an ArenaScope around an
//   arena that already has other, permanent allocations in it

#include <support/framework.h>

using namespace ArenaPro;

namespace {
constexpr std::size_t kSize = 4096;
} // namespace

// Measures a full ArenaScope construct/destruct pair.
static void bench_scope_raii() {
    Arena<false> aSrc(kSize);
    doNotOptimize(aSrc.allocate(64));

    auto a = [&] {
        ArenaScope<false> scope(aSrc);
        doNotOptimize(scope);
    };

    BENCH_SOLO("ArenaScope construct/destruct", a);
}

// Executes all scope RAII benchmark cases.
static void run_benchmarks() {
    bench_scope_raii();
}

REGISTER_BENCH_SUITE();
