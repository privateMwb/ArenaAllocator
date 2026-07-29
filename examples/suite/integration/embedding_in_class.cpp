// Embedding Arena inside a domain-specific class.
//
// Demonstrates:
// - Wrapping Arena as a private implementation detail
// - Exposing a narrow, purpose-built API instead of the raw allocator
// - A frame used internally to reset per-request state

#include <support/framework.h>

using namespace ArenaPro;

namespace {

// A minimal stand-in for a per-connection scratch allocator, the kind of
// thing an HTTP server might use to allocate short-lived, per-request
// objects without touching the heap.
class RequestScratchpad {
  public:
    explicit RequestScratchpad(std::size_t size) : arena_{size} {}

    template <typename T, typename... Args> T* create(Args&&... args) {
        return arena_.create<T>(std::forward<Args>(args)...);
    }

    // Called once a request has been fully handled, rolling back every
    // allocation made while serving it.
    void endRequest() {
        arena_.reset();
    }

    std::size_t bytesInUse() const {
        return arena_.used();
    }

  private:
    Arena<> arena_;
};

struct Header {
    const char* name;
    const char* value;
};

} // namespace

static void run_examples() {

    setTitle("Using The Wrapper");

    RequestScratchpad scratch(512);

    Header* h = scratch.create<Header>("Content-Type", "application/json");
    std::cout << h->name << ": " << h->value << "\n";
    std::cout << "bytes in use: " << scratch.bytesInUse() << "\n\n";

    setTitle("Ending A Request");

    // The caller never sees Arena's frame/reset vocabulary directly —
    // just the domain operation that happens to reuse it internally.
    scratch.endRequest();

    std::cout << "bytes in use after endRequest(): " << scratch.bytesInUse() << "\n";
}

REGISTER_EXAMPLE_SUITE();
