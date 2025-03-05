#pragma once
#include <unordered_set>
#include "Engine/Core/AssetsManager/Asset.h"
namespace Plaza {
	class PLAZA_API Script : public Asset {
	  public:
		Script() { this->lastModifiedDate = std::chrono::system_clock::now(); }
		std::chrono::system_clock::time_point lastModifiedDate;
		std::unordered_set<uint64_t> entitiesUsingThisScript;
	};
} // namespace Plaza
