// Arena Core Benchmark Suite — Construct
// Measures Arena create<T>() performance against stdArena.
//
// stdArena (a memory_resource) has no construct() of its own, so the
// comparison is built the same way Arena::create() itself is: raw
// storage from allocate(), then placement-new T(args...) directly on
// top of it — an apples-to-apples measure of allocation plus
// construction, not just allocation alone.
//
// Each Arena/Std pair runs a fixed iteration count, sized so the buffer
// can never be exhausted mid-benchmark.
//
// Covers:
// - construction of a type with a trivial, argument-less constructor
// - construction of a type with a real constructor body and multiple
//   arguments

#include <support/framework.h>

#include <benchmark/benchmark.h>

#include <cstddef>
#include <new>

using namespace ArenaPro;

namespace {
constexpr std::size_t kCapacityBytes = 64 * 1024 * 1024;
constexpr std::size_t kIterations = 1'000'000;

struct Trivial {
    int value = 0;
};

struct NonTrivial {
    double x, y, z;
    int tag;
    NonTrivial(double x_, double y_, double z_, int tag_) : x(x_), y(y_), z(z_), tag(tag_) {}
};

static_assert(kIterations * sizeof(NonTrivial) <= kCapacityBytes,
              "kIterations must fit within kCapacityBytes");
} // namespace

// Measures Arena create<T>() for a type with a trivial, argument-less
// constructor.
static void ConstructTrivialArena(benchmark::State& state) {
    Arena<false> cSrc(kCapacityBytes);

    for (auto _ : state) {
        Trivial* p = cSrc.create<Trivial>();
        benchmark::DoNotOptimize(p);
    }
}
BENCHMARK(ConstructTrivialArena)->Iterations(kIterations);

// Measures the stdArena equivalent (allocate + placement-new) for a type
// with a trivial, argument-less constructor.
static void ConstructTrivialStd(benchmark::State& state) {
    stdArena sSrc(kCapacityBytes);

    for (auto _ : state) {
        void* raw = sSrc.allocate(sizeof(Trivial), alignof(Trivial));
        Trivial* p = ::new (raw) Trivial();
        benchmark::DoNotOptimize(p);
    }
}
BENCHMARK(ConstructTrivialStd)->Iterations(kIterations);

// Measures Arena create<T>() for a type with a real constructor body and
// multiple forwarded arguments.
static void ConstructNonTrivialArena(benchmark::State& state) {
    Arena<false> cSrc(kCapacityBytes);

    for (auto _ : state) {
        NonTrivial* p = cSrc.create<NonTrivial>(1.0, 2.0, 3.0, 7);
        benchmark::DoNotOptimize(p);
    }
}
BENCHMARK(ConstructNonTrivialArena)->Iterations(kIterations);

// Measures the stdArena equivalent (allocate + placement-new) for a type
// with a real constructor body and multiple arguments.
static void ConstructNonTrivialStd(benchmark::State& state) {
    stdArena sSrc(kCapacityBytes);

    for (auto _ : state) {
        void* raw = sSrc.allocate(sizeof(NonTrivial), alignof(NonTrivial));
        NonTrivial* p = ::new (raw) NonTrivial(1.0, 2.0, 3.0, 7);
        benchmark::DoNotOptimize(p);
    }
}
BENCHMARK(ConstructNonTrivialStd)->Iterations(kIterations);
