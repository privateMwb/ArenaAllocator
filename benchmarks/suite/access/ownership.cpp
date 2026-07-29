// Arena Ownership Benchmark Suite
// Measures Arena owns() performance.
//
// Each case builds its source arena once, outside the timed lambda —
// only repeated ownership checks against an already-populated arena are
// measured. std::pmr::monotonic_buffer_resource exposes no ownership
// query, so both cases run solo.
//
// Covers:
// - owns() on a pointer that belongs to the arena (hit path)
// - owns() on a pointer that does not belong to the arena (miss path)

#include <support/framework.h>

using namespace ArenaPro;

namespace {
constexpr std::size_t kSize = 4096;
} // namespace

// Measures owns() on a pointer allocated from the arena.
static void bench_owns_hit() {
    Arena<false> aSrc(kSize);
    std::byte* ptr = aSrc.allocate(sizeof(int));

    auto a = [&] {
        bool v = aSrc.owns(ptr);
        doNotOptimize(v);
    };

    BENCH_SOLO("owns() hit", a);
}

// Measures owns() on a pointer that does not belong to the arena.
static void bench_owns_miss() {
    Arena<false> aSrc(kSize);
    (void)aSrc.allocate(sizeof(int));

    int outside = 0;
    const void* ptr = &outside;

    auto a = [&] {
        bool v = aSrc.owns(ptr);
        doNotOptimize(v);
    };

    BENCH_SOLO("owns() miss", a);
}

// Executes all ownership benchmark cases.
static void run_benchmarks() {
    bench_owns_hit();
    std::cout << "\n";

    bench_owns_miss();
}

REGISTER_BENCH_SUITE();
