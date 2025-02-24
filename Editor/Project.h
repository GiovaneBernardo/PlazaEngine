#pragma once
#include "Engine/Core/AssetsManager/Asset.h"
#include "Engine/Core/Scripting/Script.h"
namespace Plaza {
	namespace Editor {
		class PLAZA_API Project : public Asset {
		public:
			uint64_t mLastSceneUuid = 0;
			std::string creationDate = "";
			std::string lastModifyDate = "";

			static void Load(const std::string filePath);

			std::map<std::string, Script> scripts;

			static void PasteCmakeFile(const std::filesystem::path& directory);
			static void PasteGitIgnore(const std::filesystem::path& directory);
			static void PasteAllProjectFiles(const std::filesystem::path& directory);

			template <class Archive>
			void serialize(Archive& archive) {
				archive(mAssetUuid, mAssetName, creationDate, lastModifyDate, mLastSceneUuid);
			}
		};
	}
}