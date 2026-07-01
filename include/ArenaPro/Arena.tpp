// ============================================================
// Arena.tpp
// Template implementation for AllocatorPro::Arena.
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

namespace AllocatorPro {


// ============================================================
//  Section 1 — Constructors & Destructor
// ============================================================

template<bool EnableStats>
Arena<EnableStats>::Arena(std::size_t size, std::size_t alignment)
	: memory_      { allocateMemory(size, alignment) }
	, cap_         { size }
	, offset_      { 0 }
	, alignShift_  { toShift(alignment) }
	, frameStack_  {}
	, frameDepth_  { 0 } 
	, stats_       {} {
   
	// Validate allocator size.
	AP_PRE(size > 0);
}

template<bool EnableStats>
Arena<EnableStats>::~Arena() {

	// Release the owned memory buffer.
	if (memory_) {
		::operator delete(
		    memory_,
		    cap_,
		    std::align_val_t{std::size_t{1} << alignShift_}
		);
	}
}


// ============================================================
//  Section 2 — Move Semantics
// ============================================================

template<bool EnableStats>
Arena<EnableStats>::Arena(Arena&& other) noexcept
	: memory_      {
	std::exchange(other.memory_, nullptr)
}
, cap_         { std::exchange(other.cap_, 0) }
, offset_      { std::exchange(other.offset_, 0) }
, alignShift_  { other.alignShift_ }
, frameStack_  { other.frameStack_ }
, frameDepth_  { std::exchange(other.frameDepth_, 0) }
, stats_       { std::exchange(other.stats_, {}) } {}

template<bool EnableStats>
Arena<EnableStats>& Arena<EnableStats>::operator=(Arena&& other) noexcept {

	// Prevent self-assignment.
	if (this == &other)
		return *this;

	// Release the currently owned memory.
	if (memory_) {
		::operator delete(
		    memory_,
		    cap_,
		    std::align_val_t{std::size_t{1} << alignShift_}
		);
	}

	// Transfer ownership from the source allocator.
	memory_     = std::exchange(other.memory_, nullptr);
	cap_        = std::exchange(other.cap_, 0);
	offset_     = std::exchange(other.offset_, 0);
	alignShift_ = other.alignShift_;
	frameStack_ = other.frameStack_;
	frameDepth_ = std::exchange(other.frameDepth_, 0);
	stats_ = std::exchange(other.stats_, {});

	return *this;
}


// ============================================================
//  Section 3 — Core Allocation
// ============================================================

template<bool EnableStats>
std::byte* Arena<EnableStats>::allocate(std::size_t size,
                                        std::size_t request_alignment) noexcept {

	// Validate allocation arguments.
	AP_PRE(isPowerOfTwo(request_alignment));
	AP_PRE(request_alignment <= (std::size_t{1} << alignShift_));

	// Compute the aligned allocation offset.
	const std::uint8_t shift     = toShift(request_alignment);
	const std::size_t  aligned   = alignForward(offset_, shift);
	const std::size_t  newOffset = aligned + size;

	// Return nullptr if the arena cannot satisfy the request.
	if (newOffset > cap_ || newOffset < aligned)
		return nullptr;

	// Advance the allocation cursor.
	offset_ = newOffset;

	// Update allocation statistics.
	statAlloc(size, newOffset);

	// Return the allocated memory block.
	return memory_ + aligned;
}

template<bool EnableStats>
template<typename T>
T* Arena<EnableStats>::allocate() noexcept {
	return reinterpret_cast<T*>(allocate(sizeof(T), alignof(T)));
}


// ============================================================
//  Section 4 — Object Lifecycle
// ============================================================

template<bool EnableStats>
template<typename T, typename... Args>
requires (!std::is_array_v<T>) && std::constructible_from<T, Args...>
T* Arena<EnableStats>::create(Args&&... args) {

	// Reserve storage for the object.
	std::byte* raw = allocate(sizeof(T), alignof(T));

	// Return nullptr if the allocation failed.
	if (!raw)
		return nullptr;

	// Construct the object in the allocated storage.
	return ::new (static_cast<void*>(raw))
	       T(std::forward<Args>(args)...);
}

template<bool EnableStats>
template<typename T>
requires (!std::is_array_v<T>)
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

template<bool EnableStats>
void Arena<EnableStats>::beginFrame() noexcept {

	// Ensure another frame can be opened.
	AP_PRE(frameDepth_ < kMaxFrameDepth_);

	// Save the current allocation state.
	frameStack_[frameDepth_] = offset_;
	++frameDepth_;
}

template<bool EnableStats>
void Arena<EnableStats>::endFrame() noexcept {

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

template<bool EnableStats>
void Arena<EnableStats>::reset() noexcept {

	// Restore the allocator to its initial state.
	offset_     = 0;
	frameDepth_ = 0;
  
  if constexpr (EnableStats) 
  stats_ = {};
}


// ============================================================
//  Section 7 — Introspection
// ============================================================

template<bool EnableStats>
bool Arena<EnableStats>::owns(const void* ptr) const noexcept {
	const auto* p = static_cast<const std::byte*>(ptr);
	return p >= memory_ && p < memory_ + cap_;
}

template<bool EnableStats>
std::span<const std::byte> Arena<EnableStats>::view() const noexcept {
	return std::span<const std::byte>{memory_, offset_};
}

template<bool EnableStats>
const typename Arena<EnableStats>::Stats&
Arena<EnableStats>::getStats() const noexcept
requires EnableStats {
	return stats_;
}

template<bool EnableStats>
std::size_t Arena<EnableStats>::used() const noexcept {
	return offset_;
}

template<bool EnableStats>
std::size_t Arena<EnableStats>::remaining() const noexcept {
	return cap_ - offset_;
}

template<bool EnableStats>
std::size_t Arena<EnableStats>::capacity() const noexcept {
	return cap_;
}

template<bool EnableStats>
std::size_t Arena<EnableStats>::frameDepth() const noexcept {
	return frameDepth_;
}


// ============================================================
//  Section 8 — Memory Helpers
// ============================================================

template<bool EnableStats>
std::byte* Arena<EnableStats>::allocateMemory(std::size_t size,
                                              std::size_t alignment) {
	return static_cast<std::byte*>(
	    ::operator new(size, std::align_val_t{alignment})
	);
}


// ============================================================
//  Section 9 — Alignment Utilities
// ============================================================

template<bool EnableStats>
constexpr std::size_t
Arena<EnableStats>::alignForward(std::size_t ptr, std::uint8_t shift) noexcept {
	const std::size_t mask = (std::size_t{1} << shift) - 1;
	return (ptr + mask) & ~mask;
}

template<bool EnableStats>
constexpr std::uint8_t
Arena<EnableStats>::toShift(std::size_t alignment) noexcept {
	AP_PRE(isPowerOfTwo(alignment));
	return static_cast<std::uint8_t>(std::countr_zero(alignment));
}

template<bool EnableStats>
constexpr bool Arena<EnableStats>::isPowerOfTwo(std::size_t value) noexcept {
	return value != 0 && (value & (value - 1)) == 0;
}


// ============================================================
//  Section 10 — Statistics Helpers
// ============================================================

template<bool EnableStats>
constexpr void Arena<EnableStats>::statAlloc(std::size_t size, std::size_t usedNow) noexcept {
	if constexpr (EnableStats) {
		stats_.totalAllocated_ += size;
		stats_.currentUsed_     = usedNow;
		++stats_.allocations_;

		if (usedNow > stats_.peakUsed_)
			stats_.peakUsed_ = usedNow;
	}
}

template<bool EnableStats>
constexpr void Arena<EnableStats>::statDealloc() noexcept {
	if constexpr (EnableStats) {
	    stats_.currentUsed_ = offset_;
	}
}

} // namespace AllocatorPro