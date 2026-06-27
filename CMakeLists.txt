cmake_minimum_required(VERSION 3.20)
project(ArenaPro LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Library ---
add_library(ArenaPro
    src/Arena.cpp
)

target_include_directories(ArenaPro PUBLIC include)

# --- Tests ---
add_executable(tests
    tests/test_all.cpp
    tests/test_constructor.cpp
    tests/test_core_allocation.cpp
    tests/test_object_lifecycle.cpp
    tests/test_frame_management.cpp
    tests/test_state_management.cpp
    tests/test_introspection.cpp
)

target_link_libraries(tests PRIVATE ArenaPro)

# --- Benchmarks ---
add_executable(benchmarks
    benchmarks/bench_all.cpp
    benchmarks/bench_constructor.cpp
    benchmarks/bench_core_allocation.cpp
    benchmarks/bench_object_lifecycle.cpp
    benchmarks/bench_frame_management.cpp
    benchmarks/bench_state_management.cpp
    benchmarks/bench_introspection.cpp
)

target_link_libraries(benchmarks PRIVATE ArenaPro)

# --- Examples ---
add_executable(ex1
    examples/example_basic.cpp
)

target_link_libraries(ex1 PRIVATE ArenaPro)

add_executable(ex2
    examples/example_lifecycle.cpp
)

target_link_libraries(ex2 PRIVATE ArenaPro)

add_executable(ex3
    examples/example_scope.cpp
)

target_link_libraries(ex3 PRIVATE ArenaPro)