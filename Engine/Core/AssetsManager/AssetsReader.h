#pragma once
#include "Asset.h"
#include <filesystem>

namespace Plaza {
	class AssetsReader {
	  public:
		static Asset* ReadAssetAtPath(std::filesystem::path path);
	};
} // namespace Plaza
