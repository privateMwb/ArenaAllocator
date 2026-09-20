// Arena Lifecycle Benchmark Suite — Construction
// Measures Arena's constructor performance against stdArena.
//
// Deliberately a single case: Arena allocates its buffer eagerly, in
// the constructor, while monotonic_buffer_resource(initial_size,
// upstream) defers its first upstream allocation until the first
// allocate() call. There's no natural size sweep to add here — the
// interesting result is that single strategy difference, not how it
// scales with size.
//
// Covers:
// - constructing an empty allocator sized for kCapacityBytes, without
//   performing any allocation from it

#include <support/framework.h>

#include <benchmark/benchmark.h>

#include <cstddef>

using namespace ArenaPro;

namespace {
constexpr std::size_t kCapacityBytes = 4096;
} // namespace

// Measures constructing an empty, unused Arena.
static void construction_arena(benchmark::State& state) {
    for (auto _ : state) {
        Arena<false> a(kCapacityBytes);
        benchmark::DoNotOptimize(&a);
    }
}
BENCHMARK(construction_arena);

// Measures constructing an empty, unused stdArena.
static void construction_std(benchmark::State& state) {
    for (auto _ : state) {
        stdArena a(kCapacityBytes);
        benchmark::DoNotOptimize(&a);
    }
}
BENCHMARK(construction_std);
