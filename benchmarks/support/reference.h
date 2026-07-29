#pragma once

#include <memory_resource>

// The standard implementation benchmarked against ArenaPro — the
// standard library's own linear/bump allocator.
using stdArena = std::pmr::monotonic_buffer_resource;