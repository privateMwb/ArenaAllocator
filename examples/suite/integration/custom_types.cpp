// Constructing non-trivial types with create<T>().
//
// Demonstrates:
// - Forwarding constructor arguments through create<T>()
// - A type with its own owned resources (a std::string member) living
//   inside the arena's buffer
// - Multi-argument construction

#include <support/framework.h>

#include <string>

using namespace ArenaPro;

namespace {

struct Connection {
    std::string host;
    int port;
    bool secure;

    Connection(std::string host, int port, bool secure)
        : host{std::move(host)}, port{port}, secure{secure} {}
};

} // namespace

static void run_examples() {

    setTitle("Constructing A Multi-Member Type");

    Arena<> arena(512);

    Connection* conn = arena.create<Connection>("example.com", 443, true);

    std::cout << "host  : " << conn->host << "\n";
    std::cout << "port  : " << conn->port << "\n";
    std::cout << "secure: " << conn->secure << "\n\n";

    // The Connection object is constructed in place; only the
    // sizeof(Connection) footprint itself comes from the arena — the
    // heap buffer std::string owns internally does not.
    setTitle("Object Footprint vs Arena Usage");

    std::cout << "sizeof(Connection): " << sizeof(Connection) << "\n";
    std::cout << "arena used()      : " << arena.used() << "\n\n";

    setTitle("Cleaning Up");

    // destroy() must still run so Connection's own destructor releases
    // the std::string's heap buffer — the arena only owns the
    // sizeof(Connection) bytes, not what Connection itself allocates.
    arena.destroy(conn);
}

REGISTER_EXAMPLE_SUITE();
