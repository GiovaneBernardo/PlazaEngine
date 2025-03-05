#pragma once
#include <filesystem>
#include <string>
#include "Engine/Core/Engine.h"
#include "Engine/Core/UUID.h"

namespace Plaza {
	class PLAZA_API Asset {
	  public:
		Asset(uint64_t assetUuid = 0, std::string assetName = "", std::filesystem::path assetPath = "")
			: mAssetUuid(assetUuid), mAssetName(assetName), mAssetPath(assetPath) {
			mAssetUuid == 0 ? Plaza::UUID::NewUUID() : mAssetUuid;
		}
		uint64_t mAssetUuid = 0;
		std::string mAssetName = "";
		std::filesystem::path mAssetPath = std::filesystem::path{};

		~Asset() {}

		std::string GetExtension() { return mAssetPath.extension().string(); }

		template <class Archive> void serialize(Archive& archive) { archive(PL_SER(mAssetUuid), PL_SER(mAssetName)); }
	};
} // namespace Plaza
