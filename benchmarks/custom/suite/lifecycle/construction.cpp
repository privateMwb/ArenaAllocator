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

using namespace ArenaPro;

namespace {
constexpr std::size_t kCapacityBytes = 4096;
} // namespace

// Measures constructing an empty allocator, unused.
static void bench_construction() {
    auto c = [&] {
        Arena<false> a(kCapacityBytes);
        doNotOptimize(a);
    };

    auto s = [&] {
        stdArena a(kCapacityBytes);
        doNotOptimize(a);
    };

    BENCH("construction", c, s);
}

// Executes all construction benchmark cases.
static void run_benchmarks() {
    bench_construction();
}

REGISTER_BENCH_SUITE();
