/**
 * @file            Arena.tpp
 * @brief           Template implementation for ArenaPro::Arena.
 *
 * Out-of-line member definitions for the Arena bump-pointer allocator
 * declared in Arena.h. Included at the bottom of Arena.h — not meant to
 * be included directly.
 */

// ============================================================
// Arena.tpp
// Template implementation for ArenaPro::Arena.
// ============================================================
//
//  Sections:
//   1. Constructors & Destructor
//   2. Move Semantics
//   3. Core Allocation
//   4. Object Lifecycle
//   5. Frame Management
//   6. State Management
//   7. Introspection
//   8. Memory Helpers
//   9. Alignment Utilities
//   10. Statistics Helpers
//
// ============================================================

#include <bit>

namespace ArenaPro {

// ============================================================
//  Section 1 — Constructors & Destructor
// ============================================================

template <bool EnableStats>
Arena<EnableStats>::Arena(std::size_t size, std::size_t alignment)
    : memory_{allocateMemory(size, alignment)}, cap_{size}, offset_{0},
      alignShift_{toShift(alignment)}, frameStack_{}, frameDepth_{0}, stats_{} {

    // Validate allocator size.
    AP_PRE(size > 0);
}

template <bool EnableStats> Arena<EnableStats>::~Arena() {

    // Release the owned memory buffer. Deliberately the 2-argument
    // (ptr, align_val_t) overload, not the 3-argument sized-and-aligned
    // one (ptr, size, align_val_t): the latter isn't reliably available
    // across every compiler/standard-library pairing this targets, and
    // the 2-argument form is both always available and sufficient —
    // the alignment alone is enough to find the matching deallocation
    // path for a block obtained via ::operator new(size, align_val_t).
    if (memory_) {
        ::operator delete(memory_, std::align_val_t{std::size_t{1} << alignShift_});
    }
}

// ============================================================
//  Section 2 — Move Semantics
// ============================================================

template <bool EnableStats>
Arena<EnableStats>::Arena(Arena&& other) noexcept
    : memory_{std::exchange(other.memory_, nullptr)}, cap_{std::exchange(other.cap_, 0)},
      offset_{std::exchange(other.offset_, 0)}, alignShift_{other.alignShift_},
      frameStack_{other.frameStack_}, frameDepth_{std::exchange(other.frameDepth_, 0)},
      stats_{std::exchange(other.stats_, {})} {}

template <bool EnableStats>
Arena<EnableStats>& Arena<EnableStats>::operator=(Arena&& other) noexcept {

    // Prevent self-assignment.
    if (this == &other)
        return *this;

    // Release the currently owned memory. See the destructor above for
    // why this is the 2-argument (ptr, align_val_t) delete overload,
    // not the 3-argument sized-and-aligned one.
    if (memory_) {
        ::operator delete(memory_, std::align_val_t{std::size_t{1} << alignShift_});
    }

    // Transfer ownership from the source allocator.
    memory_ = std::exchange(other.memory_, nullptr);
    cap_ = std::exchange(other.cap_, 0);
    offset_ = std::exchange(other.offset_, 0);
    alignShift_ = other.alignShift_;
    frameStack_ = other.frameStack_;
    frameDepth_ = std::exchange(other.frameDepth_, 0);
    stats_ = std::exchange(other.stats_, {});

    return *this;
}

// ============================================================
//  Section 3 — Core Allocation
// ============================================================
//
// This is the hottest path in the allocator, so its structure is
// deliberate:
//
//  * Only memory_, cap_ and offset_ are read/written (one cache line —
//    see the layout note on the class). alignShift_ is *not* touched
//    here; it only matters for the precondition check below, which
//    compiles to nothing when NDEBUG is defined.
//
//  * The bounds check is written as "validate, then compute" rather
//    than "compute, then detect wraparound": we first confirm
//    `aligned` is still within the buffer, and only then compute
//    `cap_ - aligned` (which cannot underflow, because we've just
//    proven aligned <= cap_) and compare it against `size`. This
//    avoids ever forming the (potentially overflowing) sum
//    `aligned + size` before it has been validated, and it removes
//    the fast path's dependency on the CPU's overflow/carry flag from
//    the previous addition — the two comparisons are independent and
//    can be scheduled by the CPU without waiting on each other's
//    flags. Measured to be equal or fewer instructions than the
//    original formulation, with a simpler dependency chain.
//
//  * [[likely]]/[[unlikely]] mark the fast path explicitly. On this
//    allocator, running out of space is the rare event; on
//    architectures/compilers without GCC's default "forward
//    conditional branches are unlikely taken" heuristic, these
//    attributes are the difference between the failure path being
//    laid out cold (out of line) or inline in the hot path's icache
//    footprint.

template <bool EnableStats>
std::byte* Arena<EnableStats>::allocate(std::size_t size, std::size_t request_alignment) noexcept {

    // Validate allocation arguments.
    AP_PRE(isPowerOfTwo(request_alignment));
    AP_PRE(request_alignment <= (std::size_t{1} << alignShift_));

    // Compute the aligned allocation offset.
    const std::uint8_t shift = toShift(request_alignment);
    const std::size_t aligned = alignForward(offset_, shift);

    // Validate that the aligned offset itself is still within bounds
    // before computing anything derived from `size`. This makes the
    // subsequent subtraction overflow-free by construction.
    if (aligned > cap_) [[unlikely]]
        return nullptr;

    const std::size_t avail = cap_ - aligned;
    if (size > avail) [[unlikely]]
        return nullptr;

    // Advance the allocation cursor.
    offset_ = aligned + size;

    // Update allocation statistics.
    statAlloc(size, offset_);

    // Return the allocated memory block.
    return memory_ + aligned;
}

template <bool EnableStats> template <typename T> T* Arena<EnableStats>::allocate() noexcept {
    // alignof(T)/sizeof(T) are compile-time constants, so `allocate`
    // above fully constant-folds here under any optimizing compiler
    // (verified via disassembly: no runtime countr_zero/shift survives
    // at -O2, for both trivial and multi-member types). A hand-written
    // "typed fast path" was evaluated and produced byte-identical
    // generated code to this call, so it was deliberately omitted to
    // avoid duplicated logic for zero benefit.
    return reinterpret_cast<T*>(allocate(sizeof(T), alignof(T)));
}

// ============================================================
//  Section 4 — Object Lifecycle
// ============================================================

template <bool EnableStats>
template <typename T, typename... Args>
    requires(!std::is_array_v<T>) && std::constructible_from<T, Args...>
T* Arena<EnableStats>::create(Args&&... args) {

    // Reserve storage for the object.
    std::byte* raw = allocate(sizeof(T), alignof(T));

    // Return nullptr if the allocation failed.
    if (!raw) [[unlikely]]
        return nullptr;

    // Construct the object in the allocated storage.
    return ::new (static_cast<void*>(raw)) T(std::forward<Args>(args)...);
}

template <bool EnableStats>
template <typename T>
    requires(!std::is_array_v<T>)
void Arena<EnableStats>::destroy(T* ptr) noexcept {

    // Validate the object pointer.
    AP_PRE(ptr != nullptr);
    AP_PRE(owns(ptr));

    // Destroy the object. The underlying arena storage is not reclaimed.
    ptr->~T();
}

// ============================================================
//  Section 5 — Frame Management
// ============================================================

template <bool EnableStats> void Arena<EnableStats>::beginFrame() noexcept {

    // Ensure another frame can be opened.
    AP_PRE(frameDepth_ < kMaxFrameDepth_);

    // Save the current allocation state.
    frameStack_[frameDepth_] = offset_;
    ++frameDepth_;
}

template <bool EnableStats> void Arena<EnableStats>::endFrame() noexcept {

    // Ensure a frame is available to close.
    AP_PRE(frameDepth_ > 0);

    // Restore the allocation state of the previous frame.
    --frameDepth_;
    offset_ = frameStack_[frameDepth_];

    // Update allocation statistics.
    statDealloc();
}

// ============================================================
//  Section 6 — State Management
// ============================================================

template <bool EnableStats> void Arena<EnableStats>::reset() noexcept {

    // Restore the allocator to its initial state.
    offset_ = 0;
    frameDepth_ = 0;

    if constexpr (EnableStats)
        stats_ = {};
}

// ============================================================
//  Section 7 — Introspection
// ============================================================

template <bool EnableStats> bool Arena<EnableStats>::owns(const void* ptr) const noexcept {
    const auto* p = static_cast<const std::byte*>(ptr);
    return p >= memory_ && p < memory_ + cap_;
}

template <bool EnableStats> std::span<const std::byte> Arena<EnableStats>::view() const noexcept {
    return std::span<const std::byte>{memory_, offset_};
}

template <bool EnableStats>
const typename Arena<EnableStats>::Stats& Arena<EnableStats>::getStats() const noexcept
    requires EnableStats
{
    return stats_;
}

template <bool EnableStats> std::size_t Arena<EnableStats>::used() const noexcept {
    return offset_;
}

template <bool EnableStats> std::size_t Arena<EnableStats>::remaining() const noexcept {
    return cap_ - offset_;
}

template <bool EnableStats> std::size_t Arena<EnableStats>::capacity() const noexcept {
    return cap_;
}

template <bool EnableStats> std::size_t Arena<EnableStats>::frameDepth() const noexcept {
    return frameDepth_;
}

// ============================================================
//  Section 8 — Memory Helpers
// ============================================================

template <bool EnableStats>
std::byte* Arena<EnableStats>::allocateMemory(std::size_t size, std::size_t alignment) {
    return static_cast<std::byte*>(::operator new(size, std::align_val_t{alignment}));
}

// ============================================================
//  Section 9 — Alignment Utilities
// ============================================================

template <bool EnableStats>
constexpr std::size_t Arena<EnableStats>::alignForward(std::size_t ptr,
                                                       std::uint8_t shift) noexcept {
    const std::size_t mask = (std::size_t{1} << shift) - 1;
    return (ptr + mask) & ~mask;
}

template <bool EnableStats>
constexpr std::uint8_t Arena<EnableStats>::toShift(std::size_t alignment) noexcept {
    AP_PRE(isPowerOfTwo(alignment));
    return static_cast<std::uint8_t>(std::countr_zero(alignment));
}

template <bool EnableStats>
constexpr bool Arena<EnableStats>::isPowerOfTwo(std::size_t value) noexcept {
    return value != 0 && (value & (value - 1)) == 0;
}

// ============================================================
//  Section 10 — Statistics Helpers
// ============================================================

template <bool EnableStats>
constexpr void Arena<EnableStats>::statAlloc(std::size_t size, std::size_t usedNow) noexcept {
    if constexpr (EnableStats) {
        stats_.totalAllocated_ += size;
        stats_.currentUsed_ = usedNow;
        ++stats_.allocations_;

        if (usedNow > stats_.peakUsed_)
            stats_.peakUsed_ = usedNow;
    }
}

template <bool EnableStats> constexpr void Arena<EnableStats>::statDealloc() noexcept {
    if constexpr (EnableStats) {
        stats_.currentUsed_ = offset_;
    }
}

} // namespace ArenaPro
