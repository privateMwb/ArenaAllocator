/**
 * @file            Contract.h
 * @brief           Contract-checking macros and compiler attributes shared
 *                  across ArenaPro.
 *
 * Centralizes the assert()-based precondition/postcondition/invariant
 * macros used throughout the library, plus a couple of portable compiler
 * attributes, so both can be swapped out globally in one place without
 * touching call sites.
 */

#pragma once

#include <cassert>

/**
 * @brief Documents a function's preconditions. Callers must satisfy these.
 * @details Expands to `assert(condition)`; compiles to nothing when
 * `NDEBUG` is defined. Callers remain responsible for satisfying
 * preconditions unconditionally, even in release builds.
 */
#define AP_PRE(condition) assert(condition)

/// @brief Documents a function's postconditions — guarantees it makes on return.
#define AP_POST(condition) assert(condition)

/// @brief Documents a condition that must always hold for an object's state.
#define AP_INVARIANT(condition) assert(condition)

/// @brief Internal implementation sanity check, not part of any public contract.
#define AP_ASSERT(condition) assert(condition)

/**
 * @brief Marks a function as free of observable side effects, with a
 * return value depending only on its arguments and/or object state.
 * @details Expands to `__attribute__((pure))` on GCC/Clang, enabling
 * additional optimizations (e.g. common subexpression elimination across
 * repeated calls); expands to nothing on compilers that don't support it.
 */
#if defined(__GNUC__) || defined(__clang__)
#define AP_PURE __attribute__((pure))
#else
#define AP_PURE
#endif
