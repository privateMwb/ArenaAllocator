// Arena Introspection Benchmark Suite
// Measures the cost of ownership queries, memory view, and stat accessors
// against equivalent heap-based checks.
//
// Covers:
// - owns vs manual bounds check
// - view vs heap pointer arithmetic
// - getStats vs manual stat tracking
// - capacity/used/remaining vs heap size tracking

#include "bench_helper.h"

using namespace AllocatorPro;

// Owns
// measures owns() vs equivalent manual bounds check on heap pointer
static void bench_owns() {
	Arena arena{1024};
	void* ptr = arena.allocate(64, alignof(std::max_align_t));

	BENCH("arena_owns", LARGE, {
		bool result = arena.owns(ptr);
		doNotOptimize(result);
	});

	BENCH("heap_owns", LARGE, {
		void* base = ::operator new(1024);
		bool result = ptr >= base &&
		ptr < static_cast<std::byte*>(base) + 1024;
		doNotOptimize(result);
		::operator delete(base);
	});
}

// View
// measures view() vs equivalent heap span construction
static void bench_view() {
	Arena arena{1024};
	(void)arena.allocate(256, alignof(std::max_align_t));

	BENCH("arena_view", LARGE, {
		auto v = arena.view();
		doNotOptimize(v);
	});

	void* base = ::operator new(1024);

	BENCH("heap_view", LARGE, {
		const std::byte* p   = static_cast<const std::byte*>(base);
		std::size_t      len = 256;
		doNotOptimize(p);
		doNotOptimize(len);
	});

	::operator delete(base);
}

// Get Stats
// measures getStats() vs manual stat struct access
static void bench_get_stats() {
	struct HeapStats {
		std::size_t totalAllocated_;
		std::size_t currentUsed_;
		std::size_t peakUsed_;
		std::size_t allocations_;
	};
	
	Arena arena{1024};
	(void)arena.allocate(128, alignof(std::max_align_t));

	BENCH("arena_get_stats", LARGE, {
		const auto& s = arena.getStats();
		doNotOptimize(s);
	});

	BENCH("heap_get_stats", LARGE, {
		HeapStats s;
		s.totalAllocated_ = 128;
		s.currentUsed_    = 128;
		s.peakUsed_       = 128;
		s.allocations_    = 1;
		doNotOptimize(s);
	});
}

// Capacity Used Remaining
// measures capacity/used/remaining vs equivalent heap size tracking
static void bench_capacity_used_remaining() {
	Arena arena{1024};
	(void)arena.allocate(256, alignof(std::max_align_t));

	BENCH("arena_capacity_used_remaining", LARGE, {
		std::size_t cap  = arena.capacity();
		std::size_t used = arena.used();
		std::size_t rem  = arena.remaining();
		doNotOptimize(cap);
		doNotOptimize(used);
		doNotOptimize(rem);
	});

	BENCH("heap_capacity_used_remaining", LARGE, {
		std::size_t cap  = 1024;
		std::size_t used = 256;
		std::size_t rem  = cap - used;
		doNotOptimize(cap);
		doNotOptimize(used);
		doNotOptimize(rem);
	});
}

// Benchmark Runner
// Executes all introspection benchmark cases.
void run_introspection_benchmarks() {
	setHeader("Introspection Benchmarks");

	bench_owns();
	std::cout << "\n";

	bench_view();
	std::cout << "\n";

	bench_get_stats();
	std::cout << "\n";

	bench_capacity_used_remaining();
	borderLine();
	std::cout << "\n";
}
