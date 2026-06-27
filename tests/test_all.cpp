#include "test_helper.h"

#include <iostream>

void run_constructor_tests();
void run_core_allocation_tests();
void run_object_lifecycle_tests();
void run_frame_management_tests();
void run_state_management_tests();
void run_introspection_tests();

int main() {
    std::cout << "\n";
    borderLine();

    run_constructor_tests();
    run_core_allocation_tests();
    run_object_lifecycle_tests();
    run_frame_management_tests();
    run_state_management_tests();
    run_introspection_tests();

    stats();

    borderLine();
    std::cout << "\n";

    return 0;
}


