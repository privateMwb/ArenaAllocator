// Arena Core Benchmark Suite — Destroy
// Measures Arena destroy<T>() performance against stdArena.
//
// destroy() only runs T's destructor once per live object — calling it
// twice on the same pointer is undefined behavior, so this file cannot
// just repeat one destroy() call in a loop. Instead, each case
// pre-constructs a pool of distinct objects up front (untimed), then
// times destroying a fresh one from the pool on every call.
//
// Neither Arena nor a bare memory_resource reclaims storage on
// destroy — both sides do nothing but run the destructor; stdArena has
// no destroy() of its own, so its side calls the destructor directly.
//
// Covers:
// - destroying a type with a trivial (no-op) destructor
// - destroying a type with a non-trivial destructor that does real work

#include <support/framework.h>

#include <vector>

using namespace ArenaPro;

namespace {
// Sized well above the largest iteration tier (1M) plus headroom for
// any warm-up repetitions the harness performs, so the pool never runs
// out mid-benchmark. Increase this if the harness's real per-lambda
// call budget (see macros.h) turns out to exceed it.
constexpr std::size_t kPoolSize = 2'000'000;
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
} // namespace

// Measures destroy<T>() for a type with a trivial destructor.
static void bench_destroy_trivial() {
    Arena<false> cSrc(kCapacityBytes);
    stdArena sSrc(kCapacityBytes);

    std::vector<Trivial*> cPool(kPoolSize);
    std::vector<Trivial*> sPool(kPoolSize);
    for (std::size_t i = 0; i < kPoolSize; ++i) {
        cPool[i] = cSrc.create<Trivial>();
        void* raw = sSrc.allocate(sizeof(Trivial), alignof(Trivial));
        sPool[i] = ::new (raw) Trivial();
    }

    std::size_t cIdx = 0;
    std::size_t sIdx = 0;
    auto c = [&] { cSrc.destroy(cPool[cIdx++]); };
    auto s = [&] { sPool[sIdx++]->~Trivial(); };

    BENCH("destroy<T>() trivial", c, s);
}

// Measures destroy<T>() for a type with a non-trivial destructor.
static void bench_destroy_nontrivial() {
    Arena<false> cSrc(kCapacityBytes);
    stdArena sSrc(kCapacityBytes);

    std::vector<NonTrivial*> cPool(kPoolSize);
    std::vector<NonTrivial*> sPool(kPoolSize);
    for (std::size_t i = 0; i < kPoolSize; ++i) {
        cPool[i] = cSrc.create<NonTrivial>();
        void* raw = sSrc.allocate(sizeof(NonTrivial), alignof(NonTrivial));
        sPool[i] = ::new (raw) NonTrivial();
    }

    std::size_t cIdx = 0;
    std::size_t sIdx = 0;
    auto c = [&] { cSrc.destroy(cPool[cIdx++]); };
    auto s = [&] { sPool[sIdx++]->~NonTrivial(); };

    BENCH("destroy<T>() non-trivial", c, s);
}

// Executes all destroy benchmark cases.
static void run_benchmarks() {
    bench_destroy_trivial();
    std::cout << "\n";

    bench_destroy_nontrivial();
}

REGISTER_BENCH_SUITE();
