// Arena object lifecycle benchmarks.
//
// Measures the cost of arena create/destroy against equivalent
// heap new/delete for trivial and non-trivial types.
//
// Coverage:
// - Trivial type construction vs heap new/delete
// - Non-trivial type construction vs heap new/delete

#include "bench_helper.h"

using namespace AllocatorPro;

namespace {

struct Vec3 {
    float x_, y_, z_;
    Vec3(float x, float y, float z) : x_(x), y_(y), z_(z) {}
};

} // namespace

// Measures arena create/destroy for a trivial type
// against heap new/delete.
static void bench_create_trivial() {
    Arena<> a{1024 * 1024};

    auto arena_create = [&] {
        a.beginFrame();
        int* p = a.create<int>(42);
        doNotOptimize(p);
        a.destroy(p);
        a.endFrame();
        doNotOptimize();
    };
    BENCH("arena_create_trivial", LARGE, arena_create);

    auto heap_create = [&] {
        int* p = new int(42);
        doNotOptimize(p);
        delete p;
        doNotOptimize();
    };
    BENCH("heap_create_trivial", LARGE, heap_create);
}

// Measures arena create/destroy for a non-trivial type
// against heap new/delete.
static void bench_create_non_trivial() {
    Arena<> a{1024 * 1024};

    auto arena_create = [&] {
        a.beginFrame();
        Vec3* p = a.create<Vec3>(1.0f, 2.0f, 3.0f);
        doNotOptimize(p);
        a.destroy(p);
        a.endFrame();
        doNotOptimize();
    };
    BENCH("arena_create_non_trivial", LARGE, arena_create);

    auto heap_create = [&] {
        Vec3* p = new Vec3(1.0f, 2.0f, 3.0f);
        doNotOptimize(p);
        delete p;
        doNotOptimize();
    };
    BENCH("heap_create_non_trivial", LARGE, heap_create);
}

// Runs all lifecycle benchmarks.
void run_lifecycle_benchmarks() {
    setHeader("Object Lifecycle Benchmarks");

    bench_create_trivial();
    std::cout << "\n";

    bench_create_non_trivial();
    borderLine();
    std::cout << "\n";
}