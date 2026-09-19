// Arena alignment test suite.
//
// Coverage:
// - Default request_alignment matches alignof(std::max_align_t)
// - A custom power-of-two alignment is honored
// - Aligning a later allocation consumes padding bytes as needed

#include <ArenaPro/Arena.h>
#include <ArenaPro/ArenaScope.h>
#include <cstddef>
#include <cstdint>
#include <gtest/gtest.h>

using namespace ArenaPro;

namespace {

// True if `p` satisfies `alignment` (must be a power of two).
bool isAligned(const std::byte* p, std::size_t alignment) {
    return reinterpret_cast<std::uintptr_t>(p) % alignment == 0;
}

} // namespace

// Verifies allocate() with no explicit alignment uses alignof(max_align_t).
TEST(Alignment, DefaultAlignmentIsMaxAlign) {
    Arena<> arena(64);
    std::byte* p = arena.allocate(1);
    EXPECT_TRUE(isAligned(p, alignof(std::max_align_t)));
}

// Verifies allocate() honors an explicitly requested alignment.
TEST(Alignment, CustomAlignmentIsHonored) {
    Arena<> arena(128, 64);
    std::byte* p16 = arena.allocate(1, 16);
    EXPECT_TRUE(isAligned(p16, 16));

    std::byte* p32 = arena.allocate(1, 32);
    EXPECT_TRUE(isAligned(p32, 32));
}

// Verifies aligning a subsequent allocation skips the padding bytes
// needed to satisfy that alignment, advancing the cursor accordingly.
TEST(Alignment, PaddingConsumesCapacity) {
    Arena<> arena(64, 16);
    (void)arena.allocate(1, 16);
    const std::size_t usedAfterFirst = arena.used();

    std::byte* second = arena.allocate(1, 16);
    EXPECT_TRUE(isAligned(second, 16));
    EXPECT_GT(arena.used(), usedAfterFirst + 1);
}
