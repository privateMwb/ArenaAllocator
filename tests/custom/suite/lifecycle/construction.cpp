// Arena construction test suite.
//
// Coverage:
// - A valid size/alignment constructs a ready-to-use arena
// - A custom alignment is honored by the first allocation
// - An allocation failure during construction propagates std::bad_alloc

#include <support/framework.h>

#include <cstdint>
#include <new>

// AddressSanitizer's allocator does not reliably honor
// allocator_may_return_null for a SIZE_MAX-sized request across all
// versions/configurations — it can hard-abort on the OOM path instead
// of letting ::operator new throw std::bad_alloc, which this test
// relies on. That's a sanitizer edge case, not Arena's own behavior
// (its constructor just lets whatever ::operator new throws
// propagate), so the case is skipped under ASan rather than chasing
// sanitizer-version-specific behavior.
#if defined(__SANITIZE_ADDRESS__)
#define ARENA_TESTS_UNDER_ASAN 1
#elif defined(__has_feature)
#if __has_feature(address_sanitizer)
#define ARENA_TESTS_UNDER_ASAN 1
#endif
#endif
#ifndef ARENA_TESTS_UNDER_ASAN
#define ARENA_TESTS_UNDER_ASAN 0
#endif

using namespace ArenaPro;

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

#if !ARENA_TESTS_UNDER_ASAN
// Verifies a buffer size the system cannot satisfy throws std::bad_alloc.
static void huge_size_throws_bad_alloc() {
    CHK_THROWS(Arena<>(static_cast<std::size_t>(-1)), std::bad_alloc);
}
#endif

// Executes all construction test cases.
static void run_tests() {
    RUN(valid_size_constructs_correctly);
    RUN(custom_alignment_is_stored);
#if !ARENA_TESTS_UNDER_ASAN
    RUN(huge_size_throws_bad_alloc);
#endif
}

REGISTER_TEST_SUITE();
