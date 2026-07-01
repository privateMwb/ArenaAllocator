#pragma once

#include <ArenaPro/Contract.h>

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <new>
#include <span>
#include <type_traits>
#include <utility>

namespace AllocatorPro {

// Forward declaration of the RAII arena scope helper.
template<bool EnableStats>
class ArenaScope;

// A linear (bump-pointer) allocator with nested frame support.
// Allocations are performed sequentially and are reclaimed by
// resetting the allocator or rolling back to a previously opened frame.
template<bool EnableStats = false>
class Arena {
public:

        // Runtime allocation statistics.
        // Present only when EnableStats is true.
        struct Stats {
                std::size_t totalAllocated_  = 0;  // Total bytes allocated over the allocator's lifetime.
                std::size_t currentUsed_     = 0;  // Current bytes in use.
                std::size_t peakUsed_        = 0;  // Maximum bytes ever in use.
                std::size_t allocations_     = 0;  // Number of successful allocations.
        };

private:

        // Zero-size placeholder used when statistics are disabled.
        struct Empty {};

        // Core allocator state.
        std::byte*    memory_;
        std::size_t   cap_;
        std::size_t   offset_;
        std::uint8_t  alignShift_;

        // Stack of rollback frames.
        static constexpr std::size_t kMaxFrameDepth_ = 8;
        std::array<std::size_t, kMaxFrameDepth_> frameStack_;
        std::size_t frameDepth_;

        // Optional statistics storage with zero runtime overhead when disabled.
        [[no_unique_address]]
        std::conditional_t<EnableStats, Stats, Empty> stats_;

public:

        // Constructors and destructor.
        explicit Arena(std::size_t size,
                       std::size_t alignment = alignof(std::max_align_t));
        ~Arena();

        Arena(const Arena&)             = delete;
        Arena& operator=(const Arena&)  = delete;

        Arena(Arena&& other)             noexcept;
        Arena& operator=(Arena&& other)  noexcept;

        // Allocates a block of memory.
        [[nodiscard]] std::byte* allocate(
            std::size_t size,
            std::size_t request_alignment = alignof(std::max_align_t)) noexcept;

        // Allocates storage for a single object of type T.
        template<typename T>
        [[nodiscard]] T* allocate() noexcept;

        // Object construction and destruction utilities.
        template<typename T, typename... Args>
        requires (!std::is_array_v<T>) && std::constructible_from<T, Args...>
        [[nodiscard]] T* create(Args&&... args);

        template<typename T>
        requires (!std::is_array_v<T>)
        void destroy(T* ptr) noexcept;

        // Begins a new rollback frame.
        void beginFrame() noexcept;

        // Ends the current rollback frame.
        void endFrame() noexcept;

        // Resets the allocator to its initial state.
        void reset() noexcept;

        // Returns whether the pointer belongs to this allocator.
        [[nodiscard]] AP_PURE bool owns(const void* ptr) const noexcept;

        // Returns a read-only view of the allocated memory.
        [[nodiscard]] AP_PURE std::span<const std::byte> view() const noexcept;

        // Returns runtime allocation statistics.
        [[nodiscard]] const Stats& getStats() const noexcept
        requires EnableStats;

        // Returns allocator usage information.
        [[nodiscard]] AP_PURE std::size_t used()       const noexcept;
        [[nodiscard]] AP_PURE std::size_t remaining()  const noexcept;
        [[nodiscard]] AP_PURE std::size_t capacity()   const noexcept;

       // Returns current frame stack depth (number of active frames).
       [[nodiscard]] AP_PURE std::size_t frameDepth() const noexcept;
private:

        // Memory allocation helper.
        [[nodiscard]] static std::byte* allocateMemory(std::size_t size, std::size_t alignment);

        // Alignment helper utilities.
        [[nodiscard]] static constexpr std::size_t   alignForward(std::size_t ptr, std::uint8_t shift) noexcept;
        [[nodiscard]] static constexpr std::uint8_t  toShift(std::size_t alignment) noexcept;
        [[nodiscard]] static constexpr bool          isPowerOfTwo(std::size_t value) noexcept;

        // Internal statistics helpers.
        constexpr void statAlloc(std::size_t size, std::size_t usedNow) noexcept;
        constexpr void statDealloc() noexcept;
};

} // namespace AllocatorPro

#include "Arena.tpp"
