cmake_minimum_required(VERSION 3.20)

project(
    ArenaPro
    VERSION 1.0.0
    LANGUAGES CXX
)


# ============================================================================
# C++ Configuration
# ============================================================================

# Configure the C++ language standard.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# ============================================================================
# ArenaPro (Header-only Library)
# ============================================================================

# Define the header-only ArenaPro library.
add_library(ArenaPro INTERFACE)

# Expose the public include directory to consumers.
target_include_directories(
    ArenaPro
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Require C++23 for all targets that consume ArenaPro.
target_compile_features(
    ArenaPro
    INTERFACE
    cxx_std_23
)

# ============================================================================
# Tests
# ============================================================================

# Build the test executable.
add_executable(
    tests
    tests/test_all.cpp
    tests/test_constructor.cpp
    tests/test_move.cpp
    tests/test_memory.cpp
    tests/test_lifecycle.cpp
    tests/test_frame.cpp
    tests/test_state.cpp
    tests/test_introspection.cpp
    tests/test_stats.cpp
    tests/test_scope.cpp
)

# Link ArenaPro to the test executable.
target_link_libraries(
    tests
    PRIVATE
    ArenaPro
)


# ============================================================================
# Benchmarks
# ============================================================================

# Build the benchmark executable.
add_executable(
    benchmarks
    benchmarks/bench_all.cpp
    benchmarks/bench_allocation.cpp
    benchmarks/bench_lifecycle.cpp
    benchmarks/bench_reset.cpp
    benchmarks/bench_scope.cpp
)

# Link ArenaPro to the benchmark executable.
target_link_libraries(
    benchmarks
    PRIVATE
    ArenaPro
)


# ============================================================================
# Examples
# ============================================================================

# Build the basic usage example.
add_executable(
    example_basic
    examples/example_basic.cpp
)

# Link ArenaPro to the basic usage example.
target_link_libraries(
    example_basic
    PRIVATE
    ArenaPro
)

# Build the object lifecycle example.
add_executable(
    example_lifecycle
    examples/example_lifecycle.cpp
)

# Link ArenaPro to the object lifecycle example.
target_link_libraries(
    example_lifecycle
    PRIVATE
    ArenaPro
)

# Build the ArenaScope example.
add_executable(
    example_scope
    examples/example_scope.cpp
)

# Link ArenaPro to the StackScope example.
target_link_libraries(
    example_scope
    PRIVATE
    ArenaPro
)

# Build the statistics example.
add_executable(
    example_stats
    examples/example_stats.cpp
)

# Link ArenaPro to the statistics example.
target_link_libraries(
    example_stats
    PRIVATE
    ArenaPro
)


# ============================================================================
# Installation
# ============================================================================

# Install the ArenaPro target and export it for downstream projects.
install(
    TARGETS ArenaPro
    EXPORT ArenaProTargets
)

# Install the exported CMake targets.
install(
    EXPORT ArenaProTargets
    FILE ArenaProTargets.cmake
    NAMESPACE ArenaPro::
    DESTINATION lib/cmake/ArenaPro
)

# Install the public header files.
install(
    DIRECTORY include/
    DESTINATION include
)


# ============================================================================
# Packaging
# ============================================================================

include(CMakePackageConfigHelpers)

# Generate the package configuration file.
configure_package_config_file(
    cmake/ArenaProConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/ArenaProConfig.cmake
    INSTALL_DESTINATION lib/cmake/ArenaPro
)

# Generate the package version file.
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/ArenaProConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Install the package configuration files.
install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/ArenaProConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/ArenaProConfigVersion.cmake
    DESTINATION lib/cmake/ArenaPro
)