// ============================================================
// fuzz/fuzz_arena.cpp
//
// Differential fuzzer for ArenaPro::Arena, checked against an
// independent shadow model after every single operation (not just
// at the end) so a fuzzer-found failure localizes to the exact
// operation that caused it.
//
// The shadow model deliberately re-derives the bump-pointer math with
// plain modulo arithmetic instead of Arena's shift/mask formulation,
// so the two implementations can only agree if Arena's optimized hot
// path (alignForward()/toShift(), the "validate, then compute" bounds
// check) is actually correct. Every allocation is checked for the
// *exact* address it must land on, not merely "somewhere valid", and
// every returned block is then written to in full so AddressSanitizer
// sees any out-of-bounds handout, and so live blocks can later be
// verified to still hold the pattern they were filled with.
//
// Specifically targets:
//   - allocate()'s success/failure decision at the capacity boundary,
//     including the exact-fit case, zero-size requests, requests near
//     SIZE_MAX (overflow hunting), and alignment padding that alone
//     pushes the cursor past the end of the buffer
//   - alignment correctness across every power of two up to the
//     arena's base alignment, for the raw allocate(), the typed
//     allocate<T>() (including an over-aligned type), and create<T>()
//   - create<T>()/destroy() object lifecycle: perfect forwarding of
//     multiple constructor arguments, nothing constructed on a failed
//     allocation, and a live-object counter that must return to zero
//   - beginFrame()/endFrame() and the RAII ArenaScope: the cursor must
//     rewind to exactly where the frame opened, the storage must be
//     handed out again at the same addresses, and blocks allocated
//     *before* the frame must be untouched by anything that happens
//     inside it
//   - reset(), including that it also clears the statistics
//   - move construction / move assignment / self-move-assignment: the
//     destination must inherit the full state (buffer, cursor, frame
//     stack, statistics) and the source must be left valid and empty
//   - the statistics (total/current/peak/allocation count), checked
//     against the model -- the whole harness runs against both
//     Arena<true> and Arena<false>, so the `EnableStats == false`
//     instantiation (where every stats update compiles away) is
//     exercised identically
//
// The harness only ever calls Arena within its documented
// preconditions (AP_PRE): a violation aborts, which would be a false
// positive. build.sh passes -UNDEBUG so those assertions stay live and
// would surface a *harness* bug immediately.
//
// Deliberately NOT covered yet: precondition-violation behaviour (by
// definition undefined), constructor failure (bad_alloc / size 0), and
// a throwing T constructor inside create() (which would exercise the
// documented "reserved storage is simply never reused" path). That
// last one is the natural next harness to add here, not a replacement
// for this one.
// ============================================================

#include <ArenaPro/Arena.h>
#include <ArenaPro/ArenaScope.h>

#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <utility>
#include <vector>

using ArenaPro::Arena;
using ArenaPro::ArenaScope;

namespace {

// Aborts (rather than throwing/returning) on mismatch so libFuzzer
// captures a minimal, precise reproducer for exactly the operation
// that broke an invariant.
void check(bool ok) {
    if (!ok)
        std::abort();
}

// Consumes the fuzzer's input one byte at a time. Returns 0 once
// exhausted so an operation that runs off the end of the input still
// gets well-defined (if boring) arguments.
class Reader {
  public:
    Reader(const std::uint8_t* data, std::size_t size) : data_{data}, size_{size} {}

    [[nodiscard]] bool empty() const {
        return size_ == 0;
    }

    std::uint8_t byte() {
        if (size_ == 0)
            return 0;
        --size_;
        return *data_++;
    }

  private:
    const std::uint8_t* data_;
    std::size_t size_;
};

// Non-trivial type with a global live-instance counter, used to prove
// create()/destroy() run constructors and destructors exactly once.
struct Tracked {
    static inline std::size_t live = 0;

    std::uint64_t value;
    std::uint64_t complement;

    Tracked(std::uint64_t a, std::uint64_t b) : value{a ^ b}, complement{~(a ^ b)} {
        ++live;
    }
    ~Tracked() {
        --live;
    }
    Tracked(const Tracked&) = delete;
    Tracked& operator=(const Tracked&) = delete;

    [[nodiscard]] bool intact() const {
        return complement == ~value;
    }
};

// Over-aligned type: only allocatable when the arena's base alignment
// is at least 32, which the harness's alignment table sometimes gives.
struct alignas(32) Wide {
    unsigned char bytes[40];
};

// Mirrors Arena::kMaxFrameDepth_ (private). If that constant ever
// shrinks, the harness will trip Arena's own AP_PRE -- a loud signal
// to update this.
constexpr std::size_t kMaxFrames = 8;

struct Frame {
    std::size_t offset;  // Cursor value when the frame was opened.
    std::uint64_t seq;   // Allocation sequence number when it was opened.
};

// Independent model of the allocator's observable state.
struct Model {
    std::size_t cap = 0;
    std::size_t align = 1; // Base alignment of the buffer.
    std::size_t offset = 0;
    std::vector<Frame> frames;

    // Statistics (only compared when the arena tracks them).
    std::size_t total = 0;
    std::size_t current = 0;
    std::size_t peak = 0;
    std::size_t allocations = 0;

    // Round `offset` up to a multiple of `alignment` using modulo,
    // deliberately *not* the shift/mask trick Arena uses.
    [[nodiscard]] std::size_t alignedOffset(std::size_t alignment) const {
        const std::size_t rem = offset % alignment;
        return rem == 0 ? offset : offset + (alignment - rem);
    }

    // Whether a request fits. Written so no sum involving `size` is
    // ever formed, so it cannot itself overflow for size ~ SIZE_MAX.
    [[nodiscard]] bool fits(std::size_t size, std::size_t alignment) const {
        const std::size_t aligned = alignedOffset(alignment);
        return aligned <= cap && size <= cap - aligned;
    }

    void resetStats() {
        total = current = peak = allocations = 0;
    }
};

template <bool EnableStats> class ArenaHarness {
  public:
    ArenaHarness(std::size_t cap, std::size_t align)
        : arena_{cap, align}, base_{arena_.view().data()} {
        model_.cap = cap;
        model_.align = align;
        check(reinterpret_cast<std::uintptr_t>(base_) % align == 0);
        verify(arena_);
    }

    void step(Reader& in) {
        switch (in.byte() % 16) {
        case 0:
        case 1:
        case 2:
        case 3:
            allocRaw(in);
            break;
        case 4:
        case 5:
            allocTyped(in);
            break;
        case 6:
        case 7:
            createTracked(in);
            break;
        case 8:
            destroyTracked(in);
            break;
        case 9:
        case 10:
            beginFrame();
            break;
        case 11:
        case 12:
            endFrame();
            break;
        case 13:
            scoped(in);
            break;
        case 14:
            moveRoundTrip(in);
            break;
        default:
            if (in.byte() % 4 == 0) // reset() discards everything; keep it rare.
                resetAll();
            break;
        }
        verify(arena_);
    }

    // Final checks; leaves no live Tracked objects behind.
    void finish() {
        verifyLive();
        discardSince(0);
        check(Tracked::live == 0);
    }

  private:
    struct Block {
        std::size_t offset;
        std::size_t size;
        std::uint8_t tag;
        std::uint64_t seq;
    };

    struct Object {
        Tracked* ptr;
        std::uint64_t seq;
    };

    Arena<EnableStats> arena_;
    const std::byte* base_;
    Model model_;
    std::vector<Block> blocks_;   // Live raw blocks, in allocation order.
    std::vector<Object> objects_; // Live Tracked objects, in allocation order.
    std::uint64_t seq_ = 0;       // Monotonic allocation counter.
    std::size_t lastOffset_ = 0;  // Offset of the block commit() last accepted.
    std::uint8_t nextTag_ = 1;

    // ---- Argument selection -------------------------------------

    std::size_t pickAlignment(Reader& in) const {
        const unsigned shifts = static_cast<unsigned>(std::countr_zero(model_.align)) + 1;
        return std::size_t{1} << (in.byte() % shifts);
    }

    std::size_t pickSize(Reader& in, std::size_t alignment) const {
        switch (in.byte() % 8) {
        case 0:
            return 0;
        case 1: // Overflow hunting: must fail cleanly, never wrap.
            return std::numeric_limits<std::size_t>::max() - in.byte();
        case 2:
        case 3: { // Straddle the capacity boundary: room-1, room, room+1.
            const std::size_t aligned = model_.alignedOffset(alignment);
            const std::size_t room = aligned <= model_.cap ? model_.cap - aligned : 0;
            const std::size_t delta = in.byte() % 3;
            return room + delta >= 1 ? room + delta - 1 : 0;
        }
        default:
            return in.byte();
        }
    }

    // ---- Model comparison ---------------------------------------

    // Compares what the arena returned for a (size, alignment) request
    // against the model, and commits the request to the model if it
    // was supposed to succeed. Returns true iff it succeeded.
    bool commit(const void* p, std::size_t size, std::size_t alignment) {
        if (!model_.fits(size, alignment)) {
            check(p == nullptr);
            return false;
        }

        const std::size_t aligned = model_.alignedOffset(alignment);
        check(p == base_ + aligned); // The exact address, not just "valid".
        check(reinterpret_cast<std::uintptr_t>(p) % alignment == 0);

        if (size > 0) {
            check(arena_.owns(p));
            check(arena_.owns(static_cast<const std::byte*>(p) + (size - 1)));
        }

        model_.offset = aligned + size;
        model_.total += size;
        model_.current = model_.offset;
        if (model_.offset > model_.peak)
            model_.peak = model_.offset;
        ++model_.allocations;

        lastOffset_ = aligned;
        return true;
    }

    // Writes a fresh pattern over a just-allocated block (so ASan sees
    // every byte) and remembers it for later verification.
    void fill(void* p, std::size_t size) {
        const std::uint8_t tag = nextTag_++;
        if (size > 0)
            std::memset(p, tag, size);
        blocks_.push_back({lastOffset_, size, tag, seq_++});
    }

    void verify(const Arena<EnableStats>& a) const {
        check(a.capacity() == model_.cap);
        check(a.used() == model_.offset);
        check(a.remaining() == model_.cap - model_.offset);
        check(a.frameDepth() == model_.frames.size());

        const auto view = a.view();
        check(view.data() == base_);
        check(view.size() == model_.offset);

        check(a.owns(base_));
        check(a.owns(base_ + (model_.cap - 1)));
        check(!a.owns(base_ + model_.cap)); // One past the end is not owned.
        const int unrelated = 0;
        check(!a.owns(&unrelated));

        if constexpr (EnableStats) {
            const auto& s = a.getStats();
            check(s.totalAllocated_ == model_.total);
            check(s.currentUsed_ == model_.current);
            check(s.peakUsed_ == model_.peak);
            check(s.allocations_ == model_.allocations);
        }
    }

    // Everything that is supposed to still be alive must be intact.
    void verifyLive() const {
        for (const Block& b : blocks_) {
            for (std::size_t i = 0; i < b.size; ++i)
                check(base_[b.offset + i] == static_cast<std::byte>(b.tag));
        }
        for (const Object& o : objects_)
            check(o.ptr->intact());
    }

    // ---- Operations ---------------------------------------------

    void allocRaw(Reader& in) {
        const std::size_t alignment = pickAlignment(in);
        const std::size_t size = pickSize(in, alignment);

        std::byte* p = arena_.allocate(size, alignment);
        if (commit(p, size, alignment))
            fill(p, size);
    }

    void allocTyped(Reader& in) {
        switch (in.byte() % 4) {
        case 0:
            allocTypedImpl<std::uint8_t>();
            break;
        case 1:
            allocTypedImpl<std::uint32_t>();
            break;
        case 2:
            allocTypedImpl<std::uint64_t>();
            break;
        default:
            allocTypedImpl<Wide>();
            break;
        }
    }

    template <typename T> void allocTypedImpl() {
        // allocate()'s precondition: alignment <= the arena's alignment.
        if (alignof(T) > model_.align)
            return;

        T* p = arena_.template allocate<T>();
        if (commit(p, sizeof(T), alignof(T)))
            fill(p, sizeof(T));
    }

    void createTracked(Reader& in) {
        if (alignof(Tracked) > model_.align)
            return;

        const std::uint64_t a = in.byte();
        const std::uint64_t b = in.byte();

        Tracked* p = arena_.template create<Tracked>(a, b);
        if (commit(p, sizeof(Tracked), alignof(Tracked))) {
            check(p->intact() && p->value == (a ^ b));
            objects_.push_back({p, seq_++});
        }
        // Otherwise commit() already verified p == nullptr; the leak
        // check in finish() verifies nothing was constructed.
    }

    void destroyTracked(Reader& in) {
        if (objects_.empty())
            return;

        const std::size_t idx = in.byte() % objects_.size();
        Tracked* p = objects_[idx].ptr;
        check(p->intact());

        const std::size_t usedBefore = arena_.used();
        arena_.destroy(p);
        check(arena_.used() == usedBefore); // destroy() never returns storage.

        objects_.erase(objects_.begin() + static_cast<std::ptrdiff_t>(idx));
    }

    void beginFrame() {
        if (model_.frames.size() >= kMaxFrames)
            return;
        arena_.beginFrame();
        model_.frames.push_back({model_.offset, seq_});
    }

    void endFrame() {
        if (model_.frames.empty())
            return;
        const Frame f = model_.frames.back();
        discardSince(f.seq); // Destructors must run before storage is rewound.
        arena_.endFrame();
        closeFrameInModel(f);
        verifyLive(); // Everything from before the frame must have survived.
    }

    // A few raw allocations inside an RAII ArenaScope.
    void scoped(Reader& in) {
        if (model_.frames.size() >= kMaxFrames)
            return;

        const Frame f{model_.offset, seq_};
        {
            ArenaScope<EnableStats> scope{arena_};
            model_.frames.push_back(f);
            verify(arena_);

            const unsigned count = 1 + in.byte() % 3;
            for (unsigned i = 0; i < count; ++i)
                allocRaw(in);
            verify(arena_);
        } // ~ArenaScope() calls endFrame() here.

        discardSince(f.seq); // Only raw blocks were created inside.
        closeFrameInModel(f);
        verifyLive();
    }

    void resetAll() {
        discardSince(0);
        arena_.reset();

        model_.offset = 0;
        model_.frames.clear();
        model_.resetStats();
    }

    // Move-construct into a temporary, check both sides, then move
    // back by assignment -- and occasionally self-move-assign instead.
    void moveRoundTrip(Reader& in) {
        if (in.byte() % 2 == 0) {
            Arena<EnableStats>& self = arena_; // Alias: avoids -Wself-move.
            arena_ = std::move(self);
            verify(arena_);
            return;
        }

        Arena<EnableStats> moved{std::move(arena_)};
        verify(moved); // The destination inherited everything.
        checkMovedFrom(arena_);

        arena_ = std::move(moved);
        checkMovedFrom(moved);
        verify(arena_);
        verifyLive(); // The buffer's contents travelled with it.
    }

    static void checkMovedFrom(const Arena<EnableStats>& a) {
        check(a.capacity() == 0);
        check(a.used() == 0);
        check(a.remaining() == 0);
        check(a.frameDepth() == 0);
        check(a.view().empty());
        if constexpr (EnableStats) {
            const auto& s = a.getStats();
            check(s.totalAllocated_ == 0 && s.currentUsed_ == 0);
            check(s.peakUsed_ == 0 && s.allocations_ == 0);
        }
    }

    // ---- Bookkeeping --------------------------------------------

    // Destroys every Tracked object, and forgets every raw block,
    // allocated at or after sequence number `seq` -- i.e. everything a
    // frame rollback (or reset) is about to reclaim. Newest first.
    void discardSince(std::uint64_t seq) {
        while (!objects_.empty() && objects_.back().seq >= seq) {
            arena_.destroy(objects_.back().ptr);
            objects_.pop_back();
        }
        while (!blocks_.empty() && blocks_.back().seq >= seq)
            blocks_.pop_back();
    }

    void closeFrameInModel(const Frame& f) {
        model_.offset = f.offset;
        model_.current = f.offset; // peak deliberately untouched.
        model_.frames.pop_back();
    }
};

template <bool EnableStats> void runOnce(const std::uint8_t* data, std::size_t size) {
    Reader in{data, size};

    // Small capacities and alignments dominate so most runs actually
    // hit the out-of-space and alignment-padding paths.
    static constexpr std::size_t kCapacities[] = {1, 17, 64, 256, 1024, 4096};
    static constexpr std::size_t kAlignments[] = {1, 2, 4, 8, 16, 64};

    const std::size_t cap = kCapacities[in.byte() % 6];
    const std::size_t align = kAlignments[in.byte() % 6];

    ArenaHarness<EnableStats> harness{cap, align};
    while (!in.empty())
        harness.step(in);
    harness.finish();
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    if (size < 2)
        return 0;

    // Same input, both instantiations: Arena<true> gets the statistics
    // checked, Arena<false> proves every stats update really compiles
    // away without changing behaviour.
    runOnce<true>(data, size);
    runOnce<false>(data, size);
    return 0;
}
