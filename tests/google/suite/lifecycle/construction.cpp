// Arena construction test suite.
//
// Coverage:
// - A valid size/alignment constructs a ready-to-use arena
// - A custom alignment is honored by the first allocation
// - An allocation failure during construction propagates std::bad_alloc

#include <ArenaPro/Arena.h>
#include <ArenaPro/ArenaScope.h>
#include <cstddef>
#include <cstdint>
#include <gtest/gtest.h>
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
TEST(Construction, ValidSizeConstructsCorrectly) {
    Arena<> arena(64);
    EXPECT_EQ(arena.capacity(), 64u);
    EXPECT_EQ(arena.used(), 0u);
    EXPECT_EQ(arena.frameDepth(), 0u);
}

// Verifies the alignment passed to the constructor is honored by allocate().
TEST(Construction, CustomAlignmentIsStored) {
    Arena<> arena(128, 32);
    std::byte* p = arena.allocate(1, 32);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(p) % 32, 0u);
}

#if !ARENA_TESTS_UNDER_ASAN
// Verifies a buffer size the system cannot satisfy throws std::bad_alloc.
TEST(Construction, HugeSizeThrowsBadAlloc) {
    EXPECT_THROW(Arena<>(static_cast<std::size_t>(-1)), std::bad_alloc);
}
#endif
