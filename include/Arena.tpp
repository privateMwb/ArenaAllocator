#pragma once

#include <cassert>

namespace AllocatorPro {

// Alignment helpers
constexpr bool Arena::isPowerOfTwo(std::size_t alignment) noexcept {
    return alignment != 0 &&
           (alignment & (alignment - 1)) == 0;
}

constexpr std::size_t Arena::alignForward(
    std::size_t value,
    std::size_t alignment) noexcept {
    return (value + alignment - 1) & ~(alignment - 1);
}

// Core Allocation
template<typename T>
T* Arena::allocate() noexcept {
    return static_cast<T*>(allocate(sizeof(T), alignof(T)));
}

// Object Lifecycle
template<typename T, typename... Args>
T* Arena::create(Args&&... args) {
    void* raw = allocate(sizeof(T), alignof(T));

    if (!raw)
        return nullptr;

    return ::new (raw) T(std::forward<Args>(args)...);
}

template<typename T>
void Arena::destroy(T* ptr) noexcept {
    assert(ptr != nullptr);
    assert(owns(ptr));

    ptr->~T();
}

} // namespace AllocatorPro