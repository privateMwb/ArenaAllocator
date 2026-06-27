#include "Arena.h"

namespace AllocatorPro {

// Constructors & Destructor
Arena::Arena(std::size_t size)
	: memory_(static_cast<std::byte*>(
	              ::operator new(size, std::align_val_t{alignof(std::max_align_t)})))
	, cap_(size)
	, offset_(0)
	, frameDepth_(0)
	, stats_{} {}

Arena::~Arena() {
	::operator delete(memory_, std::align_val_t{alignof(std::max_align_t)});
}

Arena::Arena(Arena&& other) noexcept
	: memory_(other.memory_)
	, cap_(other.cap_)
	, offset_(other.offset_)
	, frameDepth_(other.frameDepth_)
	, frameStack_(other.frameStack_)
	, stats_(other.stats_)
{
	other.memory_ = nullptr;
	other.cap_ = 0;
	other.offset_ = 0;
	other.frameDepth_ = 0;
	other.stats_ = {};
}

Arena& Arena::operator=(Arena&& other) noexcept {
	if (this != &other)
	{
		::operator delete(memory_,
		                  std::align_val_t{alignof(std::max_align_t)});

		memory_ = other.memory_;
		cap_ = other.cap_;
		offset_ = other.offset_;
		frameDepth_ = other.frameDepth_;
		frameStack_ = other.frameStack_;
		stats_ = other.stats_;

		other.memory_ = nullptr;
		other.cap_ = 0;
		other.offset_ = 0;
		other.frameDepth_ = 0;
		other.stats_ = {};
	}

	return *this;
}

// Core Allocation
void* Arena::allocate(std::size_t size, std::size_t alignment) noexcept {
	assert(size > 0);
	assert(isPowerOfTwo(alignment));

	const std::size_t aligned = alignForward(offset_, alignment);

	if (aligned + size > cap_)
		return nullptr;

	void* ptr = memory_ + aligned;
	offset_ = aligned + size;

	++stats_.allocations_;
	stats_.totalAllocated_ += size;
	stats_.currentUsed_ = offset_;

	if (stats_.currentUsed_ > stats_.peakUsed_)
		stats_.peakUsed_ = stats_.currentUsed_;

	return ptr;
}

// Frame Management
void Arena::beginFrame() noexcept {
	assert(frameDepth_ < kMaxFrameDepth_);

	frameStack_[frameDepth_++] = offset_;
}

void Arena::endFrame() noexcept {
	assert(frameDepth_ > 0);

	offset_ = frameStack_[--frameDepth_];
	stats_.currentUsed_ = offset_;
}

// State Management
void Arena::reset() noexcept {
	offset_ = 0;
	frameDepth_ = 0;
	stats_.currentUsed_ = 0;
	stats_.allocations_ = 0;
}

// Introspection
bool Arena::owns(const void* ptr) const noexcept {
	const auto* p = static_cast<const std::byte*>(ptr);
	return p >= memory_ && p < memory_ + cap_;
}

std::span<const std::byte> Arena::view() const noexcept {
	return { memory_, offset_ };
}

const Arena::Stats& Arena::getStats() const noexcept {
	return stats_;
}

std::size_t Arena::capacity() const noexcept {
	return cap_;
}

std::size_t Arena::used() const noexcept {
	return offset_;
}

std::size_t Arena::remaining() const noexcept {
	return cap_ - offset_;
}

} // namespace AllocatorPro