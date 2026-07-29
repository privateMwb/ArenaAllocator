// Minimal smoke test: confirms the installed package's headers are
// reachable and the library links, by constructing the core type.
//
// This deliberately doesn't exercise parsing or other behavior — that
// would need matching the library's actual API surface. Consider
// expanding this to parse a small literal and check the result once
// this is filled in for a real project.
#include <ArenaPro/Arena.h>
#include <ArenaPro/ArenaScope.h>

#include <iostream>

int main() {
    rain::Arena<> arena(1024);
    std::cout << "ArenaPro linked and constructed successfully.\n";
    return 0;
}