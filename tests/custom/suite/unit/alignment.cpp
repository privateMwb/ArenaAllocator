// Arena alignment test suite.
//
// Coverage:
// - Default request_alignment matches alignof(std::max_align_t)
// - A custom power-of-two alignment is honored
// - Aligning a later allocation consumes padding bytes as needed

#include <support/framework.h>

#include <cstdint>

using namespace ArenaPro;

namespace {

// True if `p` satisfies `alignment` (must be a power of two).
bool isAligned(const std::byte* p, std::size_t alignment) {
    return reinterpret_cast<std::uintptr_t>(p) % alignment == 0;
}

} // namespace

// Verifies allocate() with no explicit alignment uses alignof(max_align_t).
static void default_alignment_is_max_align() {
    Arena<> arena(64);
    std::byte* p = arena.allocate(1);
    CHK(isAligned(p, alignof(std::max_align_t)));
}

// Verifies allocate() honors an explicitly requested alignment.
static void custom_alignment_is_honored() {
    Arena<> arena(128, 64);
    std::byte* p16 = arena.allocate(1, 16);
    CHK(isAligned(p16, 16));

    std::byte* p32 = arena.allocate(1, 32);
    CHK(isAligned(p32, 32));
}

// Verifies aligning a subsequent allocation skips the padding bytes
// needed to satisfy that alignment, advancing the cursor accordingly.
static void padding_consumes_capacity() {
    Arena<> arena(64, 16);
    (void)arena.allocate(1, 16);
    const std::size_t usedAfterFirst = arena.used();

    std::byte* second = arena.allocate(1, 16);
    CHK(isAligned(second, 16));
    CHK(arena.used() > usedAfterFirst + 1);
}

// Executes all alignment test cases.
static void run_tests() {
    RUN(default_alignment_is_max_align);
    RUN(custom_alignment_is_honored);
    RUN(padding_consumes_capacity);
}

REGISTER_TEST_SUITE();
