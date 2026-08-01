/**
 * @file            Arena.h
 *
 * @date            2026-07-28
 *
 * @version         1.0.0
 *
 * @copyright       Copyright (c) 2026 MWB
 *                  All rights reserved.
 *                  https://github.com/privateMwb/ArenaAllocator
 *
 * @attention       This source is released under the MIT license
 *                  SPDX-License-Identifier: MIT
 *                  <http://opensource.org/licenses/MIT>
 */

#pragma once

// clang-format off
#include <array>       // std::array (frame rollback stack)
#include <concepts>    // std::constructible_from
#include <cstddef>     // std::size_t, std::byte
#include <cstdint>     // std::uint8_t
#include <new>         // ::operator new/delete, std::align_val_t
#include <span>        // std::span (view())
#include <type_traits> // std::conditional_t, std::is_array_v
#include <utility>     // std::exchange, std::forward
// clang-format on

#include <ArenaPro/Contract.h>

namespace ArenaPro {

/// @brief Forward declaration of the RAII arena frame scope helper. See ArenaScope.h.
template <bool EnableStats> class ArenaScope;

/**
 * @brief A fixed-capacity linear (bump-pointer) allocator with nested frame support.
 * @tparam EnableStats When `true`, the allocator tracks allocation statistics
 * (total/current/peak usage, allocation count). Costs nothing at runtime
 * when `false` — the statistics member is an empty type and every update
 * site compiles away via `if constexpr`.
 * @details Allocations are carved sequentially out of a single buffer
 * allocated once at construction — there is no growth, no chunking, and no
 * further heap traffic for the lifetime of the arena. Memory is only ever
 * reclaimed in bulk: either all at once via reset(), or back to a
 * previously opened checkpoint via beginFrame()/endFrame(). Freeing
 * individual allocations is not supported by design; destroy() only runs a
 * `T`'s destructor and never returns storage to the arena. See ArenaScope
 * for an RAII wrapper around frame lifetime.
 *
 * @note memory_, cap_ and offset_ are declared first and are exactly 24
 * bytes (fitting a single cache line together with alignShift_). These
 * three fields are the *only* state touched by the allocate() hot path in
 * a release build (frameStack_/frameDepth_/stats_ are read on other,
 * colder paths). Do not reorder them without re-checking that the hot
 * path still fits in one cache line.
 */
template <bool EnableStats = false> class Arena {
  public:
    /**
     * @brief Runtime allocation statistics, present only when `EnableStats` is `true`.
     * @details Every field is a running total maintained by allocate() and
     * reset() at no cost when `EnableStats` is `false` (the whole struct
     * degenerates to `Empty` in that case).
     */
    struct Stats {
        std::size_t totalAllocated_ = 0; ///< Total bytes allocated over the allocator's lifetime.
        std::size_t currentUsed_ = 0;    ///< Current bytes in use.
        std::size_t peakUsed_ = 0;       ///< Maximum bytes ever in use.
        std::size_t allocations_ = 0;    ///< Number of successful allocations.
    };

  private:
    /// @brief Zero-size placeholder stored in place of `Stats` when `EnableStats` is `false`.
    struct Empty {};

    // Core allocator state (hot path — keep first / cache-line-local).
    std::byte* memory_;  ///< Owned buffer, allocated once at construction.
    std::size_t cap_;    ///< Total buffer size, in bytes.
    std::size_t offset_; ///< Current bump-pointer cursor, in bytes from `memory_`.
    std::uint8_t
        alignShift_; ///< `log2` of the buffer's base alignment; bounds `request_alignment`.

    /// @brief Maximum number of nested beginFrame()/endFrame() checkpoints supported.
    static constexpr std::size_t kMaxFrameDepth_ = 8;

    std::array<std::size_t, kMaxFrameDepth_>
        frameStack_;         ///< Saved `offset_` values, one per open frame.
    std::size_t frameDepth_; ///< Number of currently open frames.

    /// @brief Statistics storage; degenerates to `Empty` (zero size) when `EnableStats` is `false`.
    [[no_unique_address]]
    std::conditional_t<EnableStats, Stats, Empty> stats_;

  public:
    /**
     * @brief Constructs an arena that owns a freshly allocated buffer of `size` bytes.
     * @param size Total buffer size, in bytes. Must be greater than 0.
     * @param alignment Base alignment of the buffer, and therefore the
     * maximum alignment any single allocate() call may request. Defaults
     * to `alignof(std::max_align_t)`.
     * @throws Whatever `::operator new` throws (typically `std::bad_alloc`)
     * if the underlying allocation fails.
     * @details The buffer is allocated exactly once, here — no further
     * calls to `::operator new`/`::operator delete` occur for the
     * lifetime of the arena, other than in the destructor.
     */
    explicit Arena(std::size_t size, std::size_t alignment = alignof(std::max_align_t));

    /// @brief Releases the owned buffer in one call to sized, aligned `::operator delete`.
    ~Arena();

    Arena(const Arena&) = delete;
    Arena& operator=(const Arena&) = delete;

    /**
     * @brief Move-constructs an arena, taking ownership of `other`'s buffer and frame state.
     * @param other Arena to move from. Left in a valid, empty (zero-capacity) state.
     */
    Arena(Arena&& other) noexcept;

    /**
     * @brief Move-assigns from `other`, releasing this arena's current buffer first.
     * @param other Arena to move from. Left in a valid, empty (zero-capacity) state.
     * @return Reference to `*this`.
     */
    Arena& operator=(Arena&& other) noexcept;

    /**
     * @brief Allocates `size` bytes aligned to `request_alignment`.
     * @param size Number of bytes to allocate. May be 0.
     * @param request_alignment Required alignment of the returned pointer.
     * Must be a power of two and no greater than the alignment the arena
     * was constructed with. Defaults to `alignof(std::max_align_t)`.
     * @return Pointer to the allocated block, or `nullptr` if the
     * remaining capacity cannot satisfy the request.
     * @details The hot path of the allocator: bumps `offset_` forward to
     * satisfy alignment, checks the result against `cap_`, and advances
     * the cursor. Never fails except when the arena is out of space —
     * there is no other failure mode.
     */
    [[nodiscard]] std::byte*
    allocate(std::size_t size, std::size_t request_alignment = alignof(std::max_align_t)) noexcept;

    /**
     * @brief Allocates storage for a single object of type `T`, sized and aligned for `T`.
     * @tparam T Type to reserve storage for. Storage only — `T` is not constructed.
     * @return Pointer to `sizeof(T)` bytes aligned for `T`, or `nullptr` if
     * the remaining capacity cannot satisfy the request.
     */
    template <typename T> [[nodiscard]] T* allocate() noexcept;

    /**
     * @brief Allocates storage for a `T` and constructs it in place from `args`.
     * @tparam T Type to construct. Must not be an array type and must be
     * constructible from `Args...`.
     * @tparam Args Deduced types of the constructor arguments.
     * @param args Arguments forwarded into `T`'s constructor.
     * @return Pointer to the newly constructed `T`, or `nullptr` if the
     * allocation itself failed (in which case nothing is constructed).
     * @details If `T`'s constructor throws, the exception propagates and
     * the reserved storage is simply never reused — the arena is
     * unaffected, since deallocation is bulk-only by design.
     */
    template <typename T, typename... Args>
        requires(!std::is_array_v<T>) && std::constructible_from<T, Args...>
    [[nodiscard]] T* create(Args&&... args);

    /**
     * @brief Runs `ptr`'s destructor. Does not return its storage to the arena.
     * @tparam T Type of the object being destroyed. Must not be an array type.
     * @param ptr Pointer to a live object previously returned by create()
     * or otherwise placement-constructed into this arena. Must not be
     * `nullptr` and must be owned by this arena.
     * @details The arena has no concept of freeing individual
     * allocations — the underlying bytes remain reserved until the
     * enclosing frame ends or the arena is reset(). This exists purely so
     * `T`'s destructor runs at a well-defined point.
     */
    template <typename T>
        requires(!std::is_array_v<T>)
    void destroy(T* ptr) noexcept;

    /**
     * @brief Opens a new rollback checkpoint at the current allocation cursor.
     * @details Must be paired with a later endFrame(). Up to
     * `kMaxFrameDepth_` frames may be open at once. Prefer ArenaScope for
     * exception-safe, RAII-managed frames.
     */
    void beginFrame() noexcept;

    /**
     * @brief Closes the most recently opened frame, rewinding the
     * allocation cursor back to where beginFrame() opened it.
     * @details Anything allocated since the matching beginFrame() is
     * invalidated; their storage becomes available for reuse by future
     * allocations.
     */
    void endFrame() noexcept;

    /**
     * @brief Rewinds the arena to its initial, empty state in O(1).
     * @details Resets the allocation cursor and frame depth to zero and,
     * if `EnableStats` is `true`, clears the statistics. Does not touch
     * or zero the underlying buffer — only bookkeeping is reset.
     */
    void reset() noexcept;

    /**
     * @brief Checks whether `ptr` falls within this arena's buffer.
     * @param ptr Pointer to test. May be any pointer value, including one
     * from an unrelated allocation.
     * @return `true` if `ptr` lies within `[memory_, memory_ + cap_)`.
     */
    [[nodiscard]] AP_PURE bool owns(const void* ptr) const noexcept;

    /**
     * @brief Returns a read-only view of the bytes allocated so far.
     * @return A span covering `[memory_, memory_ + used())`.
     */
    [[nodiscard]] AP_PURE std::span<const std::byte> view() const noexcept;

    /**
     * @brief Returns the allocator's running statistics.
     * @return Reference to the current Stats. Only callable when
     * `EnableStats` is `true`.
     */
    [[nodiscard]] const Stats& getStats() const noexcept
        requires EnableStats;

    /// @brief Returns the number of bytes currently allocated (the bump-pointer cursor).
    [[nodiscard]] AP_PURE std::size_t used() const noexcept;
    /// @brief Returns the number of bytes still available (`capacity() - used()`).
    [[nodiscard]] AP_PURE std::size_t remaining() const noexcept;
    /// @brief Returns the total buffer size passed to the constructor.
    [[nodiscard]] AP_PURE std::size_t capacity() const noexcept;

    /// @brief Returns the current frame stack depth (number of open beginFrame() calls).
    [[nodiscard]] AP_PURE std::size_t frameDepth() const noexcept;

  private:
    /**
     * @brief Allocates the arena's owned buffer via sized, aligned `::operator new`.
     * @param size Buffer size, in bytes.
     * @param alignment Buffer alignment. Must be a power of two.
     * @return Pointer to a freshly allocated buffer of `size` bytes.
     * @throws Whatever `::operator new` throws on failure.
     */
    [[nodiscard]] static std::byte* allocateMemory(std::size_t size, std::size_t alignment);

    /**
     * @brief Validates the constructor's `size` argument and passes it through.
     * @param size Requested buffer size, in bytes. Must be greater than 0.
     * @return `size`, unchanged.
     * @details Exists so the precondition can run *before* allocateMemory()
     * is invoked in the constructor's member-initializer list — a plain
     * `AP_PRE` in the constructor body would fire only after the buffer
     * had already been allocated.
     */
    [[nodiscard]] static constexpr std::size_t validateSize(std::size_t size) noexcept;

    /**
     * @brief Rounds `ptr` up to the next multiple of `2^shift`.
     * @param ptr Byte offset to align.
     * @param shift `log2` of the target alignment, as produced by toShift().
     * @return `ptr` rounded up to the nearest multiple of `1 << shift`.
     */
    [[nodiscard]] static constexpr std::size_t alignForward(std::size_t ptr,
                                                            std::uint8_t shift) noexcept;

    /**
     * @brief Converts a power-of-two alignment to its `log2` shift amount.
     * @param alignment Alignment value. Must be a power of two.
     * @return `std::countr_zero(alignment)`.
     */
    [[nodiscard]] static constexpr std::uint8_t toShift(std::size_t alignment) noexcept;

    /**
     * @brief Checks whether `value` is a power of two.
     * @param value Value to test.
     * @return `true` if `value` is nonzero and has exactly one set bit.
     */
    [[nodiscard]] static constexpr bool isPowerOfTwo(std::size_t value) noexcept;

    /**
     * @brief Records a successful allocation in `stats_`. No-op when `EnableStats` is `false`.
     * @param size Number of bytes just allocated.
     * @param usedNow Allocation cursor immediately after the allocation.
     */
    constexpr void statAlloc(std::size_t size, std::size_t usedNow) noexcept;

    /// @brief Refreshes `stats_.currentUsed_` after a frame rollback. No-op when `EnableStats` is
    /// `false`.
    constexpr void statDealloc() noexcept;
};

} // namespace ArenaPro

/// @brief Umbrella alias so this library's types are reachable as
/// `rain::Arena`, alongside every other project library, while its true
/// namespace (and all internal diagnostics) remains `ArenaPro`. Reopens
/// `rain` rather than aliasing it, since multiple libraries each contribute
/// their own names into the same `rain` namespace -- an alias
/// (`namespace rain = ArenaPro;`) can only ever bind to one target and
/// collides the moment a second library declares its own `rain` alias to
/// something else. Declared here only; ArenaScope.h includes this header
/// directly, so `rain::ArenaScope` is already reachable through it and
/// doesn't need its own declaration.
namespace rain {
using namespace ArenaPro;
}

#include "Arena.tpp"
