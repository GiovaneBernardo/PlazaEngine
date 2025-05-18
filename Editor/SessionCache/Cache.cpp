#include "Engine/Core/PreCompiledHeaders.h"
#include "Cache.h"
#include "Engine/Core/Standards.h"
#include <ThirdParty/cereal/cereal/archives/json.hpp>
namespace Plaza::Editor {
	void Cache::Load() {
		Cache cache =
			Cache::DeSerialize(FilesManager::sEngineSettingsFolder.string() + "/cache" + Standards::editorCacheExtName);
		Project::Load(cache.mLastOpenProjectPath);
	}

	void Cache::Serialize(const std::string filePath) {
		Cache cache;
		cache.mLastOpenProjectPath = Application::Get()->projectPath + "/" +
									 Application::Get()->activeProject->mAssetName + Standards::projectExtName;
		cache.mLastOpenProjectName = Application::Get()->activeProject->mAssetName;
		std::ofstream os(filePath);
		{
			cereal::JSONOutputArchive archive(os);
			archive(cache);
		}
		os.close();
	}

	Cache Cache::DeSerialize(const std::string filePath) {
		Cache cache;
		if (!std::filesystem::exists(filePath)) {
			return cache;
		}
		std::ifstream os(filePath);
		{
			cereal::JSONInputArchive archive(os);
			archive(cache);
		}
		os.close();
		return cache;
	}
} // namespace Plaza::Editor
