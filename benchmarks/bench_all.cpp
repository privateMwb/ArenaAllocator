
#include <iostream>

void run_allocation_benchmarks();
void run_lifecycle_benchmarks();
void run_reset_benchmarks();
void run_scope_benchmarks();

int main() {
    std::cout << "\n";

    run_allocation_benchmarks();
    run_lifecycle_benchmarks();
    run_reset_benchmarks();
    run_scope_benchmarks();

    std::cout << "\n";
    return 0;
}

