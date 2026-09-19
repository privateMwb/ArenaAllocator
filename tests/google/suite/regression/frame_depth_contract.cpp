// Regression: exceeding the frame stack's capacity, or closing a
// frame when none is open, is an AP_PRE (debug-only) contract
// violation by design — not a runtime-checked error. Both sides of
// that contract are documented in Arena.h's beginFrame()/endFrame().
//
// Note: the violation itself can't be exercised here, since AP_PRE is
// a plain assert() and assert failure calls abort(), which isn't
// catchable via try/catch. This test instead pins the last *valid*
// boundary — opening exactly kMaxFrameDepth_ frames — to guard
// against an off-by-one turning that boundary into a false failure
// (e.g. `<` accidentally becoming `<=`, or vice versa).
//
// Coverage:
// - Opening the documented maximum number of frames (8, per Arena.h's
//   kMaxFrameDepth_) succeeds and closes back down to zero cleanly

#include <ArenaPro/Arena.h>
#include <ArenaPro/ArenaScope.h>
#include <cstddef>
#include <gtest/gtest.h>

using namespace ArenaPro;

// This mirrors Arena.h's private kMaxFrameDepth_ constant. Update
// alongside it if it ever changes.
namespace {
constexpr std::size_t kMaxFrameDepth = 8;
} // namespace

// Verifies opening the maximum documented number of frames succeeds,
// and that they all close back down to an empty frame stack.
TEST(FrameDepthContract, OpeningMaxFramesSucceeds) {
    Arena<> arena(64);

    for (std::size_t i = 0; i < kMaxFrameDepth; ++i)
        arena.beginFrame();
    EXPECT_EQ(arena.frameDepth(), kMaxFrameDepth);

    for (std::size_t i = 0; i < kMaxFrameDepth; ++i)
        arena.endFrame();
    EXPECT_EQ(arena.frameDepth(), 0u);
}
