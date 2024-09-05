#include "Engine/Core/PreCompiledHeaders.h"
#include "AssetsManager.h"

#include "Engine/Core/AssetsManager/Importer/AssetsImporter.h"
#include "Engine/Core/AssetsManager/Loader/AssetsLoader.h"

namespace Plaza {
	Asset* AssetsManager::GetAssetOrImport(std::string path, uint64_t uuid, std::string outDirectory) {
		Asset* asset = AssetsManager::GetAsset(path);
		if (!asset) {
			std::string importedAssetPath = AssetsImporter::ImportAsset(path, uuid, AssetsImporterSettings{ outDirectory });
			asset = AssetsManager::GetAsset(importedAssetPath);
			if (asset)
				AssetsLoader::LoadAsset(asset);
		}
		return asset;
	}
}