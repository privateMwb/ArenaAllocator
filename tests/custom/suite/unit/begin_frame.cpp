// Arena beginFrame() test suite.
//
// Coverage:
// - Opening a frame increments the frame depth
// - Nested frames are tracked correctly

#include <support/framework.h>

using namespace ArenaPro;

// Verifies beginFrame() increments frameDepth() by one.
static void opens_frame_increments_depth() {
    Arena<> arena(64);
    CHK(arena.frameDepth() == 0);

    arena.beginFrame();
    CHK(arena.frameDepth() == 1);

    arena.endFrame();
}

// Verifies several nested beginFrame() calls each increment the depth.
static void nested_frames_track_depth() {
    Arena<> arena(64);
    arena.beginFrame();
    arena.beginFrame();
    arena.beginFrame();
    CHK(arena.frameDepth() == 3);

    arena.endFrame();
    arena.endFrame();
    arena.endFrame();
}

// Executes all beginFrame() test cases.
static void run_tests() {
    RUN(opens_frame_increments_depth);
    RUN(nested_frames_track_depth);
}

REGISTER_TEST_SUITE();
