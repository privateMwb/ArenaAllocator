// Arena Core Benchmark Suite — Typed Allocate
// Measures Arena allocate<T>() performance against stdArena.
//
// Each Arena/Std pair runs a fixed iteration count, sized so the buffer
// can never be exhausted mid-benchmark — only the steady-state
// bump-allocation path is measured.
//
// Covers:
// - allocate<T>() for a small, trivially-typed T
// - allocate<T>() for a larger, multi-member T

#include <support/framework.h>

#include <benchmark/benchmark.h>

#include <cstddef>

using namespace ArenaPro;

namespace {
constexpr std::size_t kCapacityBytes = 64 * 1024 * 1024;
constexpr std::size_t kIterations = 1'000'000;

struct Small {
    int value;
};

struct Large {
    double a, b, c, d;
    int e, f;
};

static_assert(kIterations * sizeof(Large) <= kCapacityBytes,
              "kIterations must fit within kCapacityBytes");
} // namespace

// Measures Arena allocate<T>() for a small, single-member type.
static void allocate_small_type_arena(benchmark::State& state) {
    Arena<false> cSrc(kCapacityBytes);

    for (auto _ : state) {
        Small* p = cSrc.allocate<Small>();
        benchmark::DoNotOptimize(p);
    }
}
BENCHMARK(allocate_small_type_arena)->Iterations(kIterations);

// Measures the stdArena equivalent for a small, single-member type.
static void allocate_small_type_std(benchmark::State& state) {
    stdArena sSrc(kCapacityBytes);

    for (auto _ : state) {
        void* p = sSrc.allocate(sizeof(Small), alignof(Small));
        benchmark::DoNotOptimize(p);
    }
}
BENCHMARK(allocate_small_type_std)->Iterations(kIterations);

// Measures Arena allocate<T>() for a larger, multi-member type.
static void allocate_large_type_arena(benchmark::State& state) {
    Arena<false> cSrc(kCapacityBytes);

    for (auto _ : state) {
        Large* p = cSrc.allocate<Large>();
        benchmark::DoNotOptimize(p);
    }
}
BENCHMARK(allocate_large_type_arena)->Iterations(kIterations);

// Measures the stdArena equivalent for a larger, multi-member type.
static void allocate_large_type_std(benchmark::State& state) {
    stdArena sSrc(kCapacityBytes);

    for (auto _ : state) {
        void* p = sSrc.allocate(sizeof(Large), alignof(Large));
        benchmark::DoNotOptimize(p);
    }
}
BENCHMARK(allocate_large_type_std)->Iterations(kIterations);
