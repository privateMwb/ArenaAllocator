// Arena Core Benchmark Suite — Destroy
// Measures Arena destroy<T>() performance against stdArena.
//
// destroy() only runs T's destructor once per live object — calling it
// twice on the same pointer is undefined behavior, so this file cannot
// just repeat one destroy() call in a loop. Instead, each case
// pre-constructs a pool of distinct objects up front (untimed), then
// times destroying a fresh one from the pool on every iteration.
//
// Every case runs exactly kIterations iterations, and the pool holds
// exactly kIterations objects, so the pool can never run out
// mid-benchmark. Do not remove ->Iterations(): with auto-tuned
// iteration counts the loop would index past the end of the pool.
//
// Neither Arena nor a bare memory_resource reclaims storage on
// destroy — both sides do nothing but run the destructor; stdArena has
// no destroy() of its own, so its side calls the destructor directly.
//
// Covers:
// - destroying a type with a trivial (no-op) destructor
// - destroying a type with a non-trivial destructor that does real work

#include <support/framework.h>

#include <benchmark/benchmark.h>

#include <cstddef>
#include <new>
#include <vector>

using namespace ArenaPro;

namespace {
constexpr std::size_t kIterations = 1'000'000;
constexpr std::size_t kCapacityBytes = 256 * 1024 * 1024;

struct Trivial {
    int value = 0;
};

struct NonTrivial {
    int data[8]{};
    ~NonTrivial() {
        for (auto& d : data)
            d = 0;
    }
};

static_assert(kIterations * sizeof(NonTrivial) <= kCapacityBytes,
              "kIterations must fit within kCapacityBytes");
} // namespace

// Measures Arena destroy<T>() for a type with a trivial destructor.
static void destroy_trivial_arena(benchmark::State& state) {
    Arena<false> cSrc(kCapacityBytes);

    std::vector<Trivial*> pool(kIterations);
    for (std::size_t i = 0; i < kIterations; ++i)
        pool[i] = cSrc.create<Trivial>();

    std::size_t idx = 0;
    for (auto _ : state)
        cSrc.destroy(pool[idx++]);
}
BENCHMARK(destroy_trivial_arena)->Iterations(kIterations);

// Measures the stdArena equivalent (direct destructor call) for a type
// with a trivial destructor.
static void destroy_trivial_std(benchmark::State& state) {
    stdArena sSrc(kCapacityBytes);

    std::vector<Trivial*> pool(kIterations);
    for (std::size_t i = 0; i < kIterations; ++i) {
        void* raw = sSrc.allocate(sizeof(Trivial), alignof(Trivial));
        pool[i] = ::new (raw) Trivial();
    }

    std::size_t idx = 0;
    for (auto _ : state)
        pool[idx++]->~Trivial();
}
BENCHMARK(destroy_trivial_std)->Iterations(kIterations);

// Measures Arena destroy<T>() for a type with a non-trivial destructor.
static void destroy_non_trivial_arena(benchmark::State& state) {
    Arena<false> cSrc(kCapacityBytes);

    std::vector<NonTrivial*> pool(kIterations);
    for (std::size_t i = 0; i < kIterations; ++i)
        pool[i] = cSrc.create<NonTrivial>();

    std::size_t idx = 0;
    for (auto _ : state)
        cSrc.destroy(pool[idx++]);
}
BENCHMARK(destroy_non_trivial_arena)->Iterations(kIterations);

// Measures the stdArena equivalent (direct destructor call) for a type
// with a non-trivial destructor.
static void destroy_non_trivial_std(benchmark::State& state) {
    stdArena sSrc(kCapacityBytes);

    std::vector<NonTrivial*> pool(kIterations);
    for (std::size_t i = 0; i < kIterations; ++i) {
        void* raw = sSrc.allocate(sizeof(NonTrivial), alignof(NonTrivial));
        pool[i] = ::new (raw) NonTrivial();
    }

    std::size_t idx = 0;
    for (auto _ : state)
        pool[idx++]->~NonTrivial();
}
BENCHMARK(destroy_non_trivial_std)->Iterations(kIterations);
