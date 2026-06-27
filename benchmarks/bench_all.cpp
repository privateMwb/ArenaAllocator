
#include <iostream>

void run_constructor_benchmarks();
void run_core_allocation_benchmarks();
void run_object_lifecycle_benchmarks();
void run_frame_management_benchmarks();
void run_state_management_benchmarks();
void run_introspection_benchmarks();

int main() {
    std::cout << "\n";

    run_constructor_benchmarks();
    run_core_allocation_benchmarks();
    run_object_lifecycle_benchmarks();
    run_frame_management_benchmarks();
    run_state_management_benchmarks();
    run_introspection_benchmarks();

    std::cout << "\n";

    return 0;
}

