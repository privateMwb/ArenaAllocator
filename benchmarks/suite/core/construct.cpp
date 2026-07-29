// Arena Core Benchmark Suite — Construct
// Measures Arena create<T>() performance against stdArena.
//
// stdArena (a memory_resource) has no construct() of its own, so the
// comparison is built the same way Arena::create() itself is: raw
// storage from allocate(), then placement-new T(args...) directly on
// top of it — an apples-to-apples measure of allocation plus
// construction, not just allocation alone.
//
// Covers:
// - construction of a type with a trivial, argument-less constructor
// - construction of a type with a real constructor body and multiple
//   arguments

#include <support/framework.h>

using namespace ArenaPro;

namespace {
constexpr std::size_t kCapacityBytes = 64 * 1024 * 1024;

struct Trivial {
    int value = 0;
};

struct NonTrivial {
    double x, y, z;
    int tag;
    NonTrivial(double x_, double y_, double z_, int tag_) : x(x_), y(y_), z(z_), tag(tag_) {}
};
} // namespace

// Measures constructing a type with a trivial, argument-less constructor.
static void bench_construct_trivial() {
    Arena<false> cSrc(kCapacityBytes);
    stdArena sSrc(kCapacityBytes);

    auto c = [&] {
        Trivial* p = cSrc.create<Trivial>();
        doNotOptimize(p);
    };

    auto s = [&] {
        void* raw = sSrc.allocate(sizeof(Trivial), alignof(Trivial));
        Trivial* p = ::new (raw) Trivial();
        doNotOptimize(p);
    };

    BENCH("create<T>() trivial", c, s);
}

// Measures constructing a type with a real constructor body and
// multiple forwarded arguments.
static void bench_construct_nontrivial() {
    Arena<false> cSrc(kCapacityBytes);
    stdArena sSrc(kCapacityBytes);

    auto c = [&] {
        NonTrivial* p = cSrc.create<NonTrivial>(1.0, 2.0, 3.0, 7);
        doNotOptimize(p);
    };

    auto s = [&] {
        void* raw = sSrc.allocate(sizeof(NonTrivial), alignof(NonTrivial));
        NonTrivial* p = ::new (raw) NonTrivial(1.0, 2.0, 3.0, 7);
        doNotOptimize(p);
    };

    BENCH("create<T>() non-trivial", c, s);
}

// Executes all construct benchmark cases.
static void run_benchmarks() {
    bench_construct_trivial();
    std::cout << "\n";

    bench_construct_nontrivial();
}

REGISTER_BENCH_SUITE();
