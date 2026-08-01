/**
 * @file            ArenaScope.h
 * @brief           RAII helper that manages an Arena frame's lifetime.
 *
 * A small scope guard used to pair every beginFrame() with an endFrame(),
 * even when the guarded code throws or returns early. See Arena.h for the
 * frame mechanism itself.
 */

#pragma once

#include "Arena.h"

namespace ArenaPro {

/**
 * @brief RAII helper that manages an Arena frame.
 * @tparam EnableStats Must match the `EnableStats` of the Arena being scoped.
 * @details Opens a frame on construction (Arena::beginFrame()) and closes
 * it on destruction (Arena::endFrame()), so a scope's allocations are
 * always rolled back when control leaves it — including via an
 * exception. Neither copyable nor movable: a scope is tied to exactly one
 * point in the call stack.
 */
template <bool EnableStats> class [[nodiscard]] ArenaScope {
  private:
    Arena<EnableStats>& arena_; ///< Arena this scope opens/closes a frame on.

  public:
    /**
     * @brief Opens a new frame on `arena`.
     * @param arena Arena to manage. Must outlive this ArenaScope.
     */
    explicit ArenaScope(Arena<EnableStats>& arena) noexcept : arena_{arena} {
        arena_.beginFrame();
    }

    /// @brief Closes the frame opened by the constructor, rolling back its allocations.
    ~ArenaScope() noexcept {
        arena_.endFrame();
    }

    ArenaScope(const ArenaScope&) = delete;
    ArenaScope& operator=(const ArenaScope&) = delete;

    ArenaScope(ArenaScope&&) = delete;
    ArenaScope& operator=(ArenaScope&&) = delete;
};

} // namespace ArenaPro
