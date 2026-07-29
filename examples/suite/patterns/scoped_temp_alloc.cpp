// Scoped temporary allocation with ArenaScope.
//
// Demonstrates:
// - Using ArenaScope to carve out short-lived working memory inside a
//   function, without any manual cleanup
// - The pattern composing naturally with early returns and exceptions
// - The arena being exactly as full after the call as before it

#include <support/framework.h>

using namespace ArenaPro;

namespace {

// Sums the first n squares using scratch space from the arena, without
// leaving any trace behind once it returns.
int sumOfSquares(Arena<>& arena, int n) {
    ArenaScope<false> scope(arena);

    int total = 0;
    for (int i = 1; i <= n; ++i) {
        int* square = arena.create<int>(i * i);
        total += *square;
    }

    return total; // scope's destructor rolls back everything above
}

} // namespace

static void run_examples() {

    setTitle("Before Calling");

    Arena<> arena(1024);
    std::cout << "used before call: " << arena.used() << "\n\n";

    setTitle("During The Call");

    int result = sumOfSquares(arena, 5);
    std::cout << "sum of squares 1..5: " << result << "\n\n";

    setTitle("After Calling");

    // Every allocation sumOfSquares() made was rolled back when its
    // ArenaScope destructed, so the arena looks untouched from here.
    std::cout << "used after call: " << arena.used() << "\n";
}

REGISTER_EXAMPLE_SUITE();
