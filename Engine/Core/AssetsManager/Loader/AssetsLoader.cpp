#include "Engine/Core/PreCompiledHeaders.h"
#include "AssetsLoader.h"
#include "Engine/Core/AssetsManager/Serializer/AssetsSerializer.h"
#include "Editor/DefaultAssets/Models/DefaultModels.h"
#include <ThirdParty/cereal/cereal/archives/binary.hpp>
#include <future>
#include "Engine/Core/Renderer/Model.h"
#include "Engine/Core/AssetsManager/Asset.h"
#include "Engine/Components/Core/Prefab.h"
#include "Engine/Core/Renderer/Vulkan/Renderer.h"
#include "Engine/Core/Scene.h"

namespace Plaza {
	void AssetsLoader::LoadAsset(Asset* asset) {
		if (asset->GetExtension() == Standards::metadataExtName)
			AssetsLoader::LoadMetadata(asset);
		else
			AssetsManager::NewAsset<Asset>(asset->mAssetUuid, asset->mAssetPath.string());

		if (asset->GetExtension() == Standards::modelExtName)
			AssetsLoader::LoadModel(asset);
		if (asset->GetExtension() == Standards::prefabExtName)
			AssetsLoader::LoadPrefab(asset);
		else if (asset->GetExtension() == Standards::materialExtName)
			AssetsLoader::LoadMaterial(asset, Application::Get()->mSettings.mCommonSerializationMode);
		else if (asset->GetExtension() == Standards::animationExtName)
			AssetsLoader::LoadAnimation(asset, Application::Get()->mSettings.mAnimationSerializationMode);
	}

	void AssetsLoader::LoadMetadata(Asset* asset) {
		Metadata::MetadataStructure metadata =
			*AssetsSerializer::DeSerializeFile<Metadata::MetadataStructure>(
				 asset->mAssetPath.string(), Application::Get()->mSettings.mMetaDataSerializationMode)
				 .get();
		std::string metadataContentExtension =
			std::filesystem::path{asset->mAssetPath.parent_path().string() + "/" + metadata.mContentName}
				.extension()
				.string();
		bool metadataContentExtensionIsSupported =
			AssetsManager::mAssetTypeByExtension.find(metadataContentExtension) !=
			AssetsManager::mAssetTypeByExtension.end();
		if (!metadataContentExtensionIsSupported)
			return;
		AssetType type = AssetsManager::mAssetTypeByExtension.at(metadataContentExtension);

		switch (type) {
			case AssetType::TEXTURE:
				AssetsLoader::LoadTexture(AssetsManager::NewAsset<Asset>(
					std::make_shared<Asset>(Metadata::ConvertMetadataToAsset(metadata))));
				break;
			case AssetType::SCRIPT:
				AssetsLoader::LoadScript(AssetsManager::NewAsset<Asset>(
					std::make_shared<Asset>(Metadata::ConvertMetadataToAsset(metadata))));
				break;
			case AssetType::SHADERS:
				AssetsManager::AddShaders(AssetsManager::NewAsset<Asset>(
					std::make_shared<Asset>(Metadata::ConvertMetadataToAsset(metadata))));
				break;
			default:
				AssetsManager::NewAsset<Asset>(std::make_shared<Asset>(Metadata::ConvertMetadataToAsset(metadata)));
				break;
		}
	}

	std::shared_ptr<Model> AssetsLoader::LoadModel(Asset* asset) {
		std::shared_ptr<Model> model = AssetsSerializer::DeSerializeFile<Model>(
			asset->mAssetPath.string(), Application::Get()->mSettings.mModelSerializationMode);
		AssetsManager::AddModel(model);

		for (auto& [key, mesh] : model->mMeshes) {
			Mesh* newMesh = VulkanRenderer::GetRenderer()->CreateNewMesh(
				mesh->vertices, mesh->normals, mesh->uvs, mesh->tangent, mesh->indices, mesh->materialsIndices,
				mesh->usingNormal, mesh->bonesHolder, {});
			newMesh->uuid = key;
			AssetsManager::AddMesh(newMesh);
		}

		return model;
	}

	std::shared_ptr<Scene> AssetsLoader::LoadScene(Asset* asset, SerializationMode serializationMode) {
		std::shared_ptr<Scene> scene =
			AssetsSerializer::DeSerializeFile<Scene>(asset->mAssetPath.string(), serializationMode);
		Scene::GetEditorScene()->Copy(scene.get());
		return scene;
	}

	static std::vector<std::future<void>> futures;
	void AssetsLoader::LoadPrefab(Asset* asset) {
		if (!AssetsManager::GetPrefab(asset->mAssetUuid)) {
			std::shared_ptr<Prefab> prefab = AssetsSerializer::DeSerializeFile<Prefab>(
				asset->mAssetPath.string(), Application::Get()->mSettings.mPrefabSerializationMode);
			AssetsManager::AddPrefab(prefab);
		}
	}

	void AssetsLoader::LoadScript(Asset* asset) { AssetsManager::AddScript(static_cast<Script*>(asset)); };
	Texture* AssetsLoader::LoadTexture(Asset* asset) {
		if (!asset)
			return new Texture();
		if (AssetsManager::GetTexture(asset->mAssetUuid) != nullptr)
			return AssetsManager::GetTexture(asset->mAssetUuid);

		Texture* texture = Application::Get()->mRenderer->LoadTexture(asset->mAssetPath.string());
		texture->mAssetUuid = asset->mAssetUuid;
		AssetsManager::mTextures.emplace(asset->mAssetUuid, texture);
		return texture;
	}
} // namespace Plaza
