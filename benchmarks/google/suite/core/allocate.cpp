// Arena Core Benchmark Suite — Allocate
// Measures Arena allocate() performance against stdArena,
// the standard library's own linear/bump allocator.
//
// Each Arena/Std pair runs a fixed iteration count, sized so the buffer
// can never be exhausted mid-benchmark — only the steady-state
// bump-allocation path is measured, with no branching to the failure
// path. The count is derived from the largest block size, so it is safe
// for every case in this file.
//
// Covers:
// - allocate() of a small, word-sized block at the default alignment
// - allocate() of a larger, cache-line-sized block at the default alignment
// - allocate() of a small block at an over-aligned boundary (64 bytes)

#include <support/framework.h>

#include <benchmark/benchmark.h>

#include <cstddef>
#include <memory_resource>

using namespace ArenaPro;

namespace {
constexpr std::size_t kCapacityBytes = 64 * 1024 * 1024;
constexpr std::size_t kSmallSize = sizeof(int);
constexpr std::size_t kLargeSize = 256;
constexpr std::size_t kOverAlignment = 64;

// Largest per-call footprint is kLargeSize, so this many calls always fit.
constexpr std::size_t kIterations = kCapacityBytes / kLargeSize;
} // namespace

// Measures Arena allocate() of a small, word-sized block at the default
// alignment — the cheapest possible call through the hot path.
static void allocate_small_arena(benchmark::State& state) {
    Arena<false> cSrc(kCapacityBytes);

    for (auto _ : state) {
        std::byte* p = cSrc.allocate(kSmallSize);
        benchmark::DoNotOptimize(p);
    }
}
BENCHMARK(allocate_small_arena)->Iterations(kIterations);

// Measures stdArena allocate() of a small, word-sized block at the
// default alignment.
static void allocate_small_std(benchmark::State& state) {
    stdArena sSrc(kCapacityBytes);

    for (auto _ : state) {
        void* p = sSrc.allocate(kSmallSize);
        benchmark::DoNotOptimize(p);
    }
}
BENCHMARK(allocate_small_std)->Iterations(kIterations);

// Measures Arena allocate() of a larger, cache-line-sized block at the
// default alignment.
static void allocate_large_arena(benchmark::State& state) {
    Arena<false> cSrc(kCapacityBytes);

    for (auto _ : state) {
        std::byte* p = cSrc.allocate(kLargeSize);
        benchmark::DoNotOptimize(p);
    }
}
BENCHMARK(allocate_large_arena)->Iterations(kIterations);

// Measures stdArena allocate() of a larger, cache-line-sized block at
// the default alignment.
static void allocate_large_std(benchmark::State& state) {
    stdArena sSrc(kCapacityBytes);

    for (auto _ : state) {
        void* p = sSrc.allocate(kLargeSize);
        benchmark::DoNotOptimize(p);
    }
}
BENCHMARK(allocate_large_std)->Iterations(kIterations);

// Measures Arena allocate() of a small block at an over-aligned (64-byte)
// boundary — exercises the alignment padding path on every call.
static void allocate_aligned_arena(benchmark::State& state) {
    Arena<false> cSrc(kCapacityBytes, kOverAlignment);

    for (auto _ : state) {
        std::byte* p = cSrc.allocate(kSmallSize, kOverAlignment);
        benchmark::DoNotOptimize(p);
    }
}
BENCHMARK(allocate_aligned_arena)->Iterations(kIterations);

// Measures stdArena allocate() of a small block at an over-aligned
// (64-byte) boundary.
static void allocate_aligned_std(benchmark::State& state) {
    stdArena sSrc(kCapacityBytes);

    for (auto _ : state) {
        void* p = sSrc.allocate(kSmallSize, kOverAlignment);
        benchmark::DoNotOptimize(p);
    }
}
BENCHMARK(allocate_aligned_std)->Iterations(kIterations);
