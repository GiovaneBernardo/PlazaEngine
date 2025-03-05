#include "Engine/Core/PreCompiledHeaders.h"
#include "RenamedFile.h"
#include "Engine/Core/AssetsManager/AssetsReader.h"
#include "Engine/Components/Rendering/Material.h"
namespace Plaza::Editor {
	void RenamedFileManager::Run(std::string oldPath, std::string newPath) {
		std::string newExtension = std::filesystem::path{newPath}.extension().string();

		/* Material */
		if (newExtension == Standards::materialExtName) {
			Asset* asset = AssetsManager::GetAsset(newPath);
			asset->mAssetPath = newPath;
			asset->mAssetName = std::filesystem::path{newPath}.stem().string();
			AssetsManager::GetMaterial(asset->mAssetUuid)->mAssetPath = newPath;
			AssetsManager::GetMaterial(asset->mAssetUuid)->mAssetName = asset->mAssetName;
			// if (AssetsManager::mMaterialsNames.find(oldPath) != AssetsManager::mMaterialsNames.end()) {
			//	uint64_t materialUuid = AssetsManager::mMaterialsNames.at(oldPath);
			//	AssetsManager::mMaterials.at(materialUuid)->mAssetName = newPath;
			//	AssetsManager::mMaterialsNames.emplace(newPath, materialUuid);
			//	AssetsManager::mMaterialsNames.erase(oldPath);
			//	AssetsManager::ChangeAssetPath(AssetsManager::mMaterials.at(materialUuid)->mAssetUuid, newPath);
			// }
		}
		/* C# Script */
		else if (newExtension == ".cs") {
			auto it = Application::Get()->activeProject->scripts.find(oldPath);
			if (it != Application::Get()->activeProject->scripts.end())
				Application::Get()->activeProject->scripts.emplace(newPath, it->second);
		}
		else if (AssetsManager::GetExtensionType(newExtension) == AssetType::SHADERS) {
			if (AssetsManager::mShaders.find(oldPath) != AssetsManager::mShaders.end()) {
				AssetsManager::mShaders.emplace(newPath, AssetsManager::mShaders.at(oldPath));
				AssetsManager::mShaders.erase(AssetsManager::mShaders.find(oldPath));
			}
			else {
				AssetsManager::mShaders.emplace(std::filesystem::path(newPath).filename().string(), newPath);
			}
		}
	}
} // namespace Plaza::Editor
