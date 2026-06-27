#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <new>
#include <span>
#include <utility>

namespace AllocatorPro {

class Arena {
public:
    // Debug Statistics
    struct Stats {
        std::size_t totalAllocated_ = 0;
        std::size_t currentUsed_    = 0;
        std::size_t peakUsed_       = 0;
        std::size_t allocations_    = 0;
    };

private:

    // Core Memory
    std::byte*   memory_;
    std::size_t  cap_;
    std::size_t  offset_;

    // Frame Stack
    static constexpr std::size_t              kMaxFrameDepth_ = 8;
    std::array<std::size_t, kMaxFrameDepth_>  frameStack_{};
    std::size_t                               frameDepth_ = 0;

    // Statistics
    Stats stats_{};

public:
    
    // Constructors & Destructor
    explicit Arena(std::size_t size);
    ~Arena();

    Arena(const Arena&)             = delete;
    Arena& operator=(const Arena&)  = delete;

    Arena(Arena&& other)             noexcept;
    Arena& operator=(Arena&& other)  noexcept;
    
    // Core Allocation
    [[nodiscard]] void* allocate(std::size_t size, std::size_t alignment = alignof(std::max_align_t)) noexcept;

    template<typename T>
    [[nodiscard]] T* allocate() noexcept;
    
    // Object Lifecycle
    template<typename T, typename... Args>
    [[nodiscard]] T* create(Args&&... args);

    template<typename T>
    void destroy(T* ptr) noexcept;
    
    // Frame Management
    void beginFrame()  noexcept;
    void endFrame()    noexcept;
    
    // State Management 
    void reset() noexcept;
    
    // Introspection 
    [[nodiscard]] bool owns(const void* ptr)         const noexcept;
    [[nodiscard]] std::span<const std::byte> view()  const noexcept;
    [[nodiscard]] const Stats& getStats()            const noexcept;

    [[nodiscard]] std::size_t capacity()   const noexcept;
    [[nodiscard]] std::size_t used()       const noexcept;
    [[nodiscard]] std::size_t remaining()  const noexcept;

private:

    // Align Helpers
    [[nodiscard]] static constexpr std::size_t alignForward(std::size_t value, std::size_t alignment) noexcept;
    [[nodiscard]] static constexpr bool isPowerOfTwo(std::size_t alignment) noexcept;
};

} // namespace AllocatorPro

#include "Arena.tpp"