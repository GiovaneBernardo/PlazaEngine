#include "Engine/Core/PreCompiledHeaders.h"
#include "AssetsManager.h"

#include "Engine/Core/AssetsManager/Importer/AssetsImporter.h"
#include "Engine/Core/AssetsManager/Loader/AssetsLoader.h"
#include "Engine/Core/AssetsManager/AssetsReader.h"
#include "Engine/Core/Physics.h"
#include "Engine/Core/Renderer/Model.h"
#include "Engine/Components/Core/Prefab.h"
#include "Engine/Components/Physics/Collider.h"
#include "Engine/Components/Rendering/MeshRenderer.h"
#include "Engine/Components/Rendering/Material.h"
#include "Engine/Components/Rendering/AnimationComponent.h"
#include "Engine/Core/AssetsManager/Serializer/AssetsSerializer.h"
#include "Engine/Components/Physics/PhysicsMaterial.h"
#include "Engine/Core/Scripting/Script.h"

namespace Plaza {
	void AssetsManager::Init() {
		PL_CORE_INFO("Initializing Assets Manager");

		mTypeMap = []() {
			std::unordered_map<AssetType, std::unordered_set<uint64_t>> map;
			for (int i = 0; i <= UNKNOWN; ++i) {
				map.emplace(static_cast<AssetType>(i), std::unordered_set<uint64_t>());
			}
			return map;
		}();

		AssetsManager::mAssetTypeByExtension.emplace(Standards::metadataExtName, AssetType::METADATA);
		AssetsManager::mAssetTypeByExtension.emplace(Standards::modelExtName, AssetType::MODEL);
		AssetsManager::mAssetTypeByExtension.emplace(Standards::prefabExtName, AssetType::PREFAB);
		AssetsManager::mAssetTypeByExtension.emplace(Standards::materialExtName, AssetType::MATERIAL);
		AssetsManager::mAssetTypeByExtension.emplace(".png", AssetType::TEXTURE);
		AssetsManager::mAssetTypeByExtension.emplace(".jpg", AssetType::TEXTURE);
		AssetsManager::mAssetTypeByExtension.emplace(".jpeg", AssetType::TEXTURE);
		AssetsManager::mAssetTypeByExtension.emplace(".dds", AssetType::TEXTURE);
		AssetsManager::mAssetTypeByExtension.emplace(".tga", AssetType::TEXTURE);
		AssetsManager::mAssetTypeByExtension.emplace(Standards::sceneExtName, AssetType::SCENE);
		AssetsManager::mAssetTypeByExtension.emplace(Standards::animationExtName, AssetType::ANIMATION);
		AssetsManager::mAssetTypeByExtension.emplace(".h", AssetType::SCRIPT);
		AssetsManager::mAssetTypeByExtension.emplace(".cs", AssetType::SCRIPT);
		AssetsManager::mAssetTypeByExtension.emplace(".mp3", AssetType::AUDIO);
		AssetsManager::mAssetTypeByExtension.emplace(".vert", AssetType::SHADERS);
		AssetsManager::mAssetTypeByExtension.emplace(".frag", AssetType::SHADERS);
		AssetsManager::mAssetTypeByExtension.emplace(".comp", AssetType::SHADERS);
		AssetsManager::mAssetTypeByExtension.emplace(".geom", AssetType::SHADERS);
		AssetsManager::mAssetTypeByExtension.emplace(".tes", AssetType::SHADERS);
		AssetsManager::mAssetTypeByExtension.emplace(".glsl", AssetType::SHADERS);
		AssetsManager::mAssetTypeByExtension.emplace("", AssetType::NONE);

		AssetsManager::mAssetsTypesWithMetaData.emplace(AssetType::TEXTURE);
		AssetsManager::mAssetsTypesWithMetaData.emplace(AssetType::SCRIPT);
		AssetsManager::mAssetsTypesWithMetaData.emplace(AssetType::AUDIO);
		AssetsManager::mAssetsTypesWithMetaData.emplace(AssetType::SHADERS);

		Texture* defaultTexture = new Texture();
		defaultTexture->mAssetUuid = 1;
		AssetsManager::mTextures.emplace(1, defaultTexture);
	}

	AssetType AssetsManager::GetExtensionType(std::string extension) {
		if (mAssetTypeByExtension.find(extension) != mAssetTypeByExtension.end())
			return mAssetTypeByExtension.at(extension);
		else
			return AssetType::UNKNOWN;
	}

	bool AssetsManager::AssetTypeContainsMetaData(AssetType assetType) {
		return mAssetsTypesWithMetaData.find(assetType) != mAssetsTypesWithMetaData.end();
	}

	bool AssetsManager::AssetContainsMetaData(Asset* asset) {
		return AssetTypeContainsMetaData(GetExtensionType(asset->GetExtension()));
	}

	std::filesystem::path AssetsManager::GetAssetMetaDataPath(std::filesystem::path assetPath) {
		std::filesystem::path path = assetPath;
		path.replace_extension(Standards::metadataExtName);
		return path;
	}

	void AssetsManager::RenameAsset(Asset* asset, std::string oldPath, std::string newPath) {
		asset->mAssetName = std::filesystem::path{newPath}.filename().string();
		auto it = AssetsManager::mAssetsUuidByPath.find(oldPath);
		AssetsManager::mAssetsUuidByPath.emplace(newPath, it->second);
		AssetsManager::mAssetsUuidByPath.erase(it);
		if (std::filesystem::exists(oldPath))
			std::filesystem::rename(oldPath, newPath);
	}

	void AssetsManager::RenameMetaData(Asset* asset, std::string oldPath, std::string newPath) {
		AssetType type = GetExtensionType(asset->GetExtension());

		Asset* contentAsset = AssetsManager::GetAsset(asset->mAssetUuid);

		std::filesystem::path metaDataOldPath(oldPath);
		metaDataOldPath.replace_extension(Standards::metadataExtName);

		std::filesystem::path metaDataNewPath(newPath);
		metaDataNewPath.replace_extension(Standards::metadataExtName);
		std::shared_ptr<Metadata::MetadataStructure> metaData =
			AssetsSerializer::DeSerializeFile<Metadata::MetadataStructure>(
				metaDataOldPath.string(), Application::Get()->mSettings.mMetaDataSerializationMode);

		// Rename the non metadata file
		std::filesystem::path contentOldPath(oldPath);
		contentOldPath.replace_extension(std::filesystem::path{metaData->mContentName}.extension().string());

		std::filesystem::path contentNewPath(newPath);
		contentNewPath.replace_extension(std::filesystem::path{metaData->mContentName}.extension().string());
		AssetsManager::RenameAsset(contentAsset, contentOldPath.string(), contentNewPath.string());

		// Rename the medadata and change its content
		std::filesystem::rename(metaDataOldPath, metaDataNewPath);
		metaData->mAssetName = metaDataNewPath.filename().string();
		contentNewPath.replace_extension(std::filesystem::path{newPath}.extension().string());
		metaData->mContentName = contentNewPath.filename().string();
		AssetsSerializer::SerializeFile<Metadata::MetadataStructure>(
			*metaData.get(), metaDataNewPath.string(), Application::Get()->mSettings.mMetaDataSerializationMode);
	}

	void AssetsManager::AfterRename(Asset* renamedAsset, std::string oldPath, std::string newPath) {
		AssetType type = AssetsManager::GetExtensionType(renamedAsset->GetExtension());
		switch (type) {
			// case AssetType::SCRIPT:
			//	AssetsManager::RemoveScript(renamedAsset->mAssetUuid);
			//	AssetsManager::AddScript(static_cast<Script*>(renamedAsset));
			//	break;
		}
	}

	Asset* AssetsManager::GetAsset(uint64_t uuid) {
		const auto& it = mAssets.find(uuid);
		if (it != mAssets.end())
			return mAssets.at(uuid);
		return nullptr;
	}

	Asset* AssetsManager::GetAsset(std::filesystem::path path) {
		const auto& it = AssetsManager::mAssetsUuidByPath.find(path);
		if (it != mAssetsUuidByPath.end())
			return GetAsset(it->second);
		return nullptr;
	}

	void AssetsManager::AddAsset(Asset* asset) {
		AssetsManager::mAssets.emplace(asset->mAssetUuid, asset);
		AssetsManager::mTypeMap.find(mAssetTypeByExtension.at(asset->GetExtension()))
			->second.emplace(asset->mAssetUuid);
		AssetsManager::mAssetsUuidByPath.emplace(asset->mAssetPath, asset->mAssetUuid);
	}

	bool AssetsManager::HasAssetPath(std::string& path) {
		return AssetsManager::mAssetsUuidByPath.find(path) != AssetsManager::mAssetsUuidByPath.end();
	}

	void AssetsManager::AddModel(std::shared_ptr<Model> model) { mModels.emplace(model->mAssetUuid, model); }

	void AssetsManager::AddPrefab(std::shared_ptr<Prefab> prefab) { mPrefabs.emplace(prefab->mAssetUuid, prefab); }

	Prefab* AssetsManager::GetPrefab(uint64_t uuid) {
		const auto& it = mPrefabs.find(uuid);
		if (it != mPrefabs.end())
			return mPrefabs.at(uuid).get();
		return nullptr;
	}

	Texture* AssetsManager::GetTexture(uint64_t uuid) {
		const auto& it = mTextures.find(uuid);
		if (it != mTextures.end())
			return mTextures.at(uuid);
		return nullptr;
	}

	bool AssetsManager::TextureExists(uint64_t uuid) {
		const auto& it = mTextures.find(uuid);
		if (it != mTextures.end())
			return true;
		return false;
	}

	void AssetsManager::AddMesh(Mesh* mesh) { AssetsManager::mLoadedMeshes.emplace(mesh->uuid, mesh); }

	Mesh* AssetsManager::GetMesh(uint64_t uuid) {
		const auto& it = mLoadedMeshes.find(uuid);
		if (it != mLoadedMeshes.end())
			return mLoadedMeshes.at(uuid);
		return nullptr;
	}

	bool AssetsManager::HasMesh(uint64_t uuid) { return mLoadedMeshes.find(uuid) != mLoadedMeshes.end(); }

	Animation& AssetsManager::AddAnimation(Animation animation) {
		AssetsManager::mLoadedAnimations.emplace(animation.mAssetUuid, animation);
		return mLoadedAnimations.at(animation.mAssetUuid);
	}

	Animation* AssetsManager::GetAnimation(uint64_t uuid) {
		const auto& it = mLoadedAnimations.find(uuid);
		if (it != mLoadedAnimations.end())
			return &mLoadedAnimations.at(uuid);
		return nullptr;
	}

	Asset* AssetsManager::GetSceneAsset(uint64_t uuid) {
		const auto& it = mSceneAssets.find(uuid);
		if (it != mSceneAssets.end())
			return mSceneAssets.at(uuid);
		return nullptr;
	}

	void AssetsManager::AddMaterial(Material* material) {
		mMaterials.emplace(material->mAssetUuid, material);
		mMaterialsNames.emplace(material->mAssetPath.string(), material->mAssetUuid);
	}

	void AssetsManager::AddMaterial(std::shared_ptr<Material> material) {
		mMaterials.emplace(material->mAssetUuid, material);
		mMaterialsNames.emplace(material->mAssetPath.string(), material->mAssetUuid);
	}

	Material* AssetsManager::GetDefaultMaterial() {
		static Material* material = nullptr;
		if (material == nullptr) {
			material = new Material();
			material->mAssetName = "Default Material";
			material->mAssetUuid = 0;
			AssetsManager::AddMaterial(material);
		}
		return material;
	}

	Material* AssetsManager::GetMaterial(uint64_t uuid) {
		auto it = mMaterials.find(uuid);
		if (it != mMaterials.end()) {
			return it->second.get();
		}
		return AssetsManager::GetDefaultMaterial(); //->DefaultMaterial();
	}

	std::vector<Material*> AssetsManager::GetMaterialsVector(const std::vector<uint64_t>& uuids) {
		std::vector<Material*> materials = std::vector<Material*>();
		for (unsigned int i = 0; i < uuids.size(); ++i) {
			auto it = mMaterials.find(uuids[i]);
			if (it != mMaterials.end()) {
				materials.push_back(it->second.get());
			}
			else {
				materials.push_back(AssetsManager::GetDefaultMaterial());
			}
		}
		if (materials.size() == 0)
			return {AssetsManager::GetDefaultMaterial()};
		return materials;
	}

	PhysicsMaterial& AssetsManager::GetPhysicsMaterial(PhysicsMaterial& other) {
		auto it = mPhysicsMaterials.find(other);
		return *it->second.get();
	}
	PhysicsMaterial& AssetsManager::GetPhysicsMaterial(float staticFriction, float dynamicFriction, float restitution) {
		PhysicsMaterial material(staticFriction, dynamicFriction, restitution);
		bool materialDoesntExists = mPhysicsMaterials.find(material) == mPhysicsMaterials.end();
		if (materialDoesntExists) {
			mPhysicsMaterials[material] =
				std::make_shared<PhysicsMaterial>(staticFriction, dynamicFriction, restitution);
			mPhysicsMaterials[material]->mPhysxMaterial =
				Physics::InitializePhysicsMaterial(staticFriction, dynamicFriction, restitution);
		}
		return *mPhysicsMaterials.find(material)->second.get();
	}

	void AssetsManager::AddScript(Script* script) { mScripts.emplace(script->mAssetUuid, script); }

	Script* AssetsManager::GetScript(uint64_t uuid) {
		auto it = mScripts.find(uuid);
		if (it != mScripts.end()) {
			return it->second;
		}
		return nullptr;
	}

	void AssetsManager::RemoveScript(uint64_t uuid) {
		auto it = mScripts.find(uuid);
		if (it != mScripts.end()) {
			mScripts.erase(it);
		}
	}

	Asset* AssetsManager::LoadMetadataAsAsset(std::filesystem::path path) {
		std::ifstream binaryFile(path, std::ios::binary);
		if (!binaryFile.is_open()) {
			std::cerr << "Error opening file for writing!" << std::endl;
			return nullptr;
		}

		uint64_t uuid = 0;
		binaryFile.read(reinterpret_cast<char*>(&uuid), sizeof(uint64_t));
		std::string contentPath = Plaza::Utils::ReadBinaryString(binaryFile);
		std::string extension = Plaza::Utils::ReadBinaryString(binaryFile);

		std::string assetFinalPath =
			path.parent_path().string() + "/" + std::filesystem::path{contentPath}.filename().string();

		binaryFile.close();

		return AssetsManager::NewAsset<Asset>(uuid, assetFinalPath);
	}

	Asset* AssetsManager::LoadBinaryFileAsAsset(std::filesystem::path path) {
		std::ifstream binaryFile(path, std::ios::binary);
		if (!binaryFile.is_open()) {
			std::cerr << "Error opening file for writing!" << std::endl;
			return nullptr;
		}

		uint64_t uuid = 0;
		binaryFile.read(reinterpret_cast<char*>(&uuid), sizeof(uint64_t));

		binaryFile.close();

		return AssetsManager::NewAsset<Asset>(uuid, path.string());
	}

	void AssetsManager::RemoveAssetUuidPath(uint64_t assetUuid) {
		const auto& it = AssetsManager::mAssetsUuidByPath.find(AssetsManager::GetAsset(assetUuid)->mAssetPath);
		if (it != AssetsManager::mAssetsUuidByPath.end())
			AssetsManager::mAssetsUuidByPath.erase(it);
	}

	void AssetsManager::ChangeAssetPath(uint64_t assetUuid, std::string newPath) {
		AssetsManager::RemoveAssetUuidPath(assetUuid);
		AssetsManager::GetAsset(assetUuid)->mAssetPath = newPath;
		AssetsManager::mAssetsUuidByPath.emplace(std::filesystem::path{newPath}, assetUuid);
	}

	Asset* AssetsManager::GetAssetOrImport(std::string path, uint64_t uuid, std::string outDirectory) {
		Asset* asset = AssetsManager::GetAsset(path);
		if (!asset) {
			std::string importedAssetPath =
				AssetsImporter::ImportAsset(path, uuid, AssetsImporterSettings{outDirectory});
			asset = AssetsManager::GetAsset(importedAssetPath);
			if (asset)
				AssetsLoader::LoadAsset(asset);
		}
		return asset;
	}

	void AssetsManager::ReadFolderContent(std::string path, bool readSubFolders) {
		PL_CORE_INFO("Reading at path: " + path);
		for (auto entry =
				 filesystem::recursive_directory_iterator(path, filesystem::directory_options::skip_permission_denied);
			 entry != filesystem::end(entry); ++entry) {
			if (entry->is_directory() && entry->path().filename().string().ends_with(".ignore")) {
				entry.disable_recursion_pending();
			}

			AssetsReader::ReadAssetAtPath(entry->path());
		}
	}
} // namespace Plaza
