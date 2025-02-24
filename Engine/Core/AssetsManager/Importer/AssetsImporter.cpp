#include "Engine/Core/PreCompiledHeaders.h"
#include "AssetsImporter.h"
#include "Engine/Core/AssetsManager/Serializer/AssetsSerializer.h"
#include "Editor/GUI/FileExplorer/FileExplorer.h"
#include "Editor/GUI/Utils/Filesystem.h"
#include "Engine/Core/AssetsManager/Loader/AssetsLoader.h"
#include "Engine/Core/AssetsManager/Metadata/Metadata.h"
#include "Engine/Components/Core/Prefab.h"
#include "Engine/Core/Renderer/Model.h"
#include "Engine/Core/Scene.h"

namespace Plaza {
	std::string AssetsImporter::ImportAsset(std::string path, uint64_t uuid, AssetsImporterSettings settings) {
		std::filesystem::path filePath = std::filesystem::path{ path };
		if (!std::filesystem::exists(filePath))
			return "";

		std::string extension = filePath.extension().string();

		if (AssetsImporter::mExtensionMapping.find(extension) == AssetsImporter::mExtensionMapping.end())
			return "";

		AssetImported asset = AssetImported({ extension, path });

		std::string outDirectory = settings.outDirectory.empty() ? Editor::Gui::FileExplorer::currentDirectory : settings.outDirectory;
		std::string outPath = outDirectory + "/" + Editor::Utils::Filesystem::GetUnrepeatedName(std::filesystem::path{ path }.stem().string());

		Entity* mainEntity;
		switch (AssetsImporter::mExtensionMapping.at(extension)) {
		case AssetExtension::OBJ:
			AssetsImporter::ImportModel(asset, outPath, outDirectory, settings);
			break;
		case AssetExtension::FBX:
			AssetsImporter::ImportModel(asset, outPath, outDirectory, settings);
			break;
		case AssetExtension::PNG:
			return AssetsImporter::ImportTexture(asset, uuid);
			break;
		case AssetExtension::JPEG:
			return AssetsImporter::ImportTexture(asset, uuid);
			break;
		case AssetExtension::JPG:
			return AssetsImporter::ImportTexture(asset, uuid);
			break;
		case AssetExtension::DDS:
			return AssetsImporter::ImportTexture(asset, uuid);
			break;
		case AssetExtension::TGA:
			return AssetsImporter::ImportTexture(asset, uuid);
			break;
		case AssetExtension::PSD:
			return AssetsImporter::ImportTexture(asset, uuid);
			break;
		case AssetExtension::MP3:
			return AssetsImporter::ImportTexture(asset, uuid);
			break;
		}

		return outPath;
	}

	void AssetsImporter::ImportModel(const AssetImported& asset, const std::string& outPath, const std::string& outDirectory, const AssetsImporterSettings& settings) {
		std::string extension = std::filesystem::path(asset.mPath).extension().string();
		std::shared_ptr<Model> model = std::make_shared<Model>();
		std::shared_ptr<Scene> modelScene;

		switch (AssetsImporter::mExtensionMapping.at(extension)) {
		case AssetExtension::OBJ:
			if (settings.mImportModel) {
				modelScene = AssetsImporter::ImportOBJ(asset, std::filesystem::path{}, *model.get());
			}
			break;
		case AssetExtension::FBX:
			if (settings.mImportModel) {
				modelScene = AssetsImporter::ImportFBX(asset, std::filesystem::path(outPath), *model.get(), settings);
			}
			//AssetsLoader::LoadPrefab(AssetsManager::NewAsset(AssetType::MODEL, outPath + Standards::modelExtName));
			break;
		}
		if (settings.mImportAnimations)
			ImportAnimation(asset.mPath, outDirectory);

		model->mAssetPath = std::filesystem::path{ outPath + Standards::modelExtName };
		model->mAssetName = model->mAssetPath.stem().string();
		std::shared_ptr<Prefab> prefab = std::make_shared<Prefab>(modelScene.get(), modelScene->mainSceneEntity);
		prefab->mAssetPath = std::filesystem::path{ outPath + Standards::prefabExtName };
		prefab->mAssetName = prefab->mAssetPath.stem().string();

		AssetsSerializer::SerializeFile<Model>(*model.get(), model->mAssetPath.string(), Application::Get()->mSettings.mModelSerializationMode);
		AssetsSerializer::SerializeFile<Prefab>(*prefab.get(), prefab->mAssetPath.string(), Application::Get()->mSettings.mPrefabSerializationMode);

		std::shared_ptr<Model> modelAsset = AssetsSerializer::DeSerializeFile<Model>(model->mAssetPath.string(), Application::Get()->mSettings.mModelSerializationMode);
		AssetsLoader::LoadAsset(modelAsset.get());
		std::shared_ptr<Prefab> prefabAsset = AssetsSerializer::DeSerializeFile<Prefab>(prefab->mAssetPath.string(), Application::Get()->mSettings.mPrefabSerializationMode);
		AssetsLoader::LoadAsset(prefabAsset.get());

		prefabAsset->LoadToScene(Scene::GetActiveScene());
		Scene::GetActiveScene()->RecalculateAddedComponents();

	}

	void AssetsImporter::ImportAnimation(std::filesystem::path filePath, std::filesystem::path outFolder, AssetsImporterSettings settings) {
		if (!std::filesystem::exists(filePath))
			return;

		std::string extension = filePath.extension().string();
		if (AssetsImporter::mExtensionMapping.find(extension) == AssetsImporter::mExtensionMapping.end())
			return;

		AssetImported asset = AssetImported({ extension, filePath.string() });

		std::vector<Animation> loadedAnimations = std::vector<Animation>();

		switch (AssetsImporter::mExtensionMapping.at(extension)) {
		case AssetExtension::FBX:
			loadedAnimations = AssetsImporter::ImportAnimationFBX(asset.mPath);
			break;
		}

		for (Animation& animation : loadedAnimations) {
			std::string animationOutPath = Editor::Utils::Filesystem::GetUnrepeatedPath(outFolder.string() + "/" + animation.mAssetName + Standards::animationExtName);
			AssetsSerializer::SerializeAnimation(animation, animationOutPath, Application::Get()->mSettings.mAnimationSerializationMode);
			AssetsManager::NewAsset<Animation>(AssetType::ANIMATION, animationOutPath);
			AssetsManager::AddAnimation(animation);
		}
	}

	std::string AssetsImporter::ImportTexture(AssetImported importedAsset, uint64_t uuid) {
		std::string outPath = Editor::Gui::FileExplorer::currentDirectory + "/" + std::filesystem::path{ importedAsset.mPath }.filename().string();
		outPath = Editor::Utils::Filesystem::GetUnrepeatedPath(outPath);

		std::filesystem::copy(importedAsset.mPath, outPath);

		Asset asset{ uuid <= 0 ? Plaza::UUID::NewUUID() : uuid, std::filesystem::path{ importedAsset.mPath }.filename().string() , outPath };
		Metadata::CreateMetadataFile(&asset);
		AssetsManager::AddAsset(new Asset(asset));

		return outPath;
	}
}