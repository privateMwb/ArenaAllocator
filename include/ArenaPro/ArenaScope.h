#pragma once

#include "Arena.h"

namespace AllocatorPro {

// RAII helper that manages an Arena frame.
// Opens a frame on construction and closes it on destruction.
template<bool EnableStats>
class [[nodiscard]] ArenaScope {
private:
    Arena<EnableStats>& arena_;

public:
    explicit ArenaScope(Arena<EnableStats>& arena) noexcept
        : arena_{arena} {
        arena_.beginFrame();
    }

    ~ArenaScope() noexcept {
        arena_.endFrame();
    }

    ArenaScope(const ArenaScope&)            = delete;
    ArenaScope& operator=(const ArenaScope&) = delete;

    ArenaScope(ArenaScope&&)                 = delete;
    ArenaScope& operator=(ArenaScope&&)      = delete;
};

} // namespace AllocatorPro