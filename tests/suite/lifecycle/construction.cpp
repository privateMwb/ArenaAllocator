// Arena construction test suite.
//
// Coverage:
// - A valid size/alignment constructs a ready-to-use arena
// - A custom alignment is honored by the first allocation
// - An allocation failure during construction propagates std::bad_alloc

#include <support/framework.h>

#include <cstdint>
#include <new>

using namespace AllocatorPro;

// Verifies a freshly constructed arena starts empty with the requested capacity.
static void valid_size_constructs_correctly() {
    Arena<> arena(64);
    CHK(arena.capacity() == 64);
    CHK(arena.used() == 0);
    CHK(arena.frameDepth() == 0);
}

// Verifies the alignment passed to the constructor is honored by allocate().
static void custom_alignment_is_stored() {
    Arena<> arena(128, 32);
    std::byte* p = arena.allocate(1, 32);
    CHK(reinterpret_cast<std::uintptr_t>(p) % 32 == 0);
}

// Verifies a buffer size the system cannot satisfy throws std::bad_alloc.
static void huge_size_throws_bad_alloc() {
    CHK_THROWS(Arena<>(static_cast<std::size_t>(-1)), std::bad_alloc);
}

// Executes all construction test cases.
static void run_tests() {
    RUN(valid_size_constructs_correctly);
    RUN(custom_alignment_is_stored);
    RUN(huge_size_throws_bad_alloc);
}

REGISTER_TEST_SUITE();
