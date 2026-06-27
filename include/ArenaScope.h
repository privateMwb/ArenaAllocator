#pragma once

#include "Arena.h"

namespace AllocatorPro {

class [[nodiscard]] ArenaScope {
private:
	// Reference
	Arena& arena_;

public:
	// Constructor & Destructor
	explicit ArenaScope(Arena& arena) :
		arena_(arena) {
		arena_.beginFrame();
	}

	~ArenaScope() {
		arena_.endFrame();
	}

	ArenaScope(const ArenaScope&)             = delete;
	ArenaScope& operator=(const ArenaScope&)  = delete;

	ArenaScope(ArenaScope&&)             = delete;
	ArenaScope& operator=(ArenaScope&&)  = delete;
};

} // namespace AllocatorPro