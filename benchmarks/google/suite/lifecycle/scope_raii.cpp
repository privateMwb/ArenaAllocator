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

#include <benchmark/benchmark.h>

#include <cstddef>

using namespace ArenaPro;

namespace {
constexpr std::size_t kSize = 4096;
} // namespace

// Measures a full ArenaScope construct/destruct pair.
static void ScopeRaii(benchmark::State& state) {
    Arena<false> aSrc(kSize);
    benchmark::DoNotOptimize(aSrc.allocate(64));

    for (auto _ : state) {
        ArenaScope<false> scope(aSrc);
        benchmark::DoNotOptimize(scope);
    }
}
BENCHMARK(ScopeRaii);
