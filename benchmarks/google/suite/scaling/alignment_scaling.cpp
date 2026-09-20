// Arena Scaling Benchmark Suite — Alignment Scaling
// Measures how Arena allocate() cost changes as the requested
// alignment grows, against stdArena.
//
// The Arena side wraps every call in beginFrame()/endFrame() for the
// same reason as capacity_growth.cpp: a page-aligned request can burn
// close to a full 4 KiB of padding per call, and across kIterations
// (1,000,000 calls) that adds up to gigabytes — the wrapper rolls the
// cursor back after every measured call so no buffer size has to be
// sized for that, and the added bookkeeping is the same constant at
// every alignment, so the comparison across them still holds.
// stdArena needs no wrapper; its default upstream resource
// transparently grows on demand.
//
// Every case runs a fixed kIterations, on both sides, so the two are
// compared over identical work and stdArena's upstream growth stays
// bounded. Google Benchmark's auto-tuned count would keep growing it
// for as long as the timer ran.
//
// Covers:
// - allocate() at the natural alignment of an int (4 bytes)
// - allocate() at cache-line alignment (64 bytes)
// - allocate() at page alignment (4096 bytes)

#include <support/framework.h>

#include <benchmark/benchmark.h>

#include <cstddef>

using namespace ArenaPro;

namespace {
constexpr std::size_t kCapacityBytes = 1024 * 1024;
constexpr std::size_t kAllocSize = sizeof(int);
constexpr std::size_t kNaturalAlignment = alignof(int);
constexpr std::size_t kCacheLineAlignment = 64;
constexpr std::size_t kPageAlignment = 4096;
constexpr std::size_t kIterations = 1'000'000;
} // namespace

// Measures Arena allocate() at the natural alignment of an int.
static void alignment_natural_arena(benchmark::State& state) {
    Arena<false> cSrc(kCapacityBytes, kNaturalAlignment);

    for (auto _ : state) {
        cSrc.beginFrame();
        benchmark::DoNotOptimize(cSrc.allocate(kAllocSize, kNaturalAlignment));
        cSrc.endFrame();
    }
}
BENCHMARK(alignment_natural_arena)->Iterations(kIterations);

// Measures stdArena allocate() at the natural alignment of an int.
static void alignment_natural_std(benchmark::State& state) {
    stdArena sSrc(kCapacityBytes);

    for (auto _ : state)
        benchmark::DoNotOptimize(sSrc.allocate(kAllocSize, kNaturalAlignment));
}
BENCHMARK(alignment_natural_std)->Iterations(kIterations);

// Measures Arena allocate() at cache-line alignment.
static void alignment_cache_line_arena(benchmark::State& state) {
    Arena<false> cSrc(kCapacityBytes, kCacheLineAlignment);

    for (auto _ : state) {
        cSrc.beginFrame();
        benchmark::DoNotOptimize(cSrc.allocate(kAllocSize, kCacheLineAlignment));
        cSrc.endFrame();
    }
}
BENCHMARK(alignment_cache_line_arena)->Iterations(kIterations);

// Measures stdArena allocate() at cache-line alignment.
static void alignment_cache_line_std(benchmark::State& state) {
    stdArena sSrc(kCapacityBytes);

    for (auto _ : state)
        benchmark::DoNotOptimize(sSrc.allocate(kAllocSize, kCacheLineAlignment));
}
BENCHMARK(alignment_cache_line_std)->Iterations(kIterations);

// Measures Arena allocate() at page alignment.
static void alignment_page_arena(benchmark::State& state) {
    Arena<false> cSrc(kCapacityBytes, kPageAlignment);

    for (auto _ : state) {
        cSrc.beginFrame();
        benchmark::DoNotOptimize(cSrc.allocate(kAllocSize, kPageAlignment));
        cSrc.endFrame();
    }
}
BENCHMARK(alignment_page_arena)->Iterations(kIterations);

// Measures stdArena allocate() at page alignment.
static void alignment_page_std(benchmark::State& state) {
    stdArena sSrc(kCapacityBytes);

    for (auto _ : state)
        benchmark::DoNotOptimize(sSrc.allocate(kAllocSize, kPageAlignment));
}
BENCHMARK(alignment_page_std)->Iterations(kIterations);
