#pragma once
#include "Engine/Core/Engine.h"
#include <cstdint>

namespace Plaza {
	class PLAZA_API UUID {
	public:
		static uint64_t NewUUID();
	};
}