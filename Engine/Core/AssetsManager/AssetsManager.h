#pragma once
#include "Engine/Core/AssetsManager/AssetsType.h"
#include "Engine/Core/Standards.h"
#include "Engine/Components/Physics/Collider.h"
#include "Engine/Components/Rendering/MeshRenderer.h"
#include "Engine/Components/Rendering/Material.h"
#include "Engine/Components/Rendering/AnimationComponent.h"
#include "Engine/Core/AssetsManager/Serializer/AssetsSerializer.h"
#include "Asset.h"

namespace Plaza {
	class AssetsListStructure : public std::unordered_map<uint64_t, Asset*> {
	public:

	};

	struct LoadedModel {
		uint64_t uuid;
		SerializablePrefab mSerializablePrefab;
	};

	class AssetsManager {
	public:
		static inline AssetsListStructure mAssets = AssetsListStructure();
		static inline std::map<std::filesystem::path, uint64_t> mAssetsUuidByPath = std::map<std::filesystem::path, uint64_t>();
		static inline std::map<std::string, AssetType> mAssetTypeByExtension = std::map<std::string, AssetType>();

		static inline std::unordered_map<uint64_t, Texture*> mTextures = std::unordered_map<uint64_t, Texture*>();
		static inline std::unordered_map<uint64_t, shared_ptr<Material>> mMaterials = std::unordered_map<uint64_t, shared_ptr<Material>>();
		static inline std::unordered_map<std::string, uint64_t> mMaterialsNames = std::unordered_map<std::string, uint64_t>();
		static inline std::unordered_map<uint64_t, LoadedModel*> mLoadedModels = std::unordered_map<uint64_t, LoadedModel*>();
		static inline std::unordered_map<uint64_t, Mesh*> mLoadedMeshes = std::unordered_map<uint64_t, Mesh*>();
		static inline std::unordered_map<uint64_t, Animation> mLoadedAnimations = std::unordered_map<uint64_t, Animation>();
		static inline std::unordered_map<uint64_t, Asset*> mSceneAssets = std::unordered_map<uint64_t, Asset*>();

		static inline std::unordered_map<AssetType, std::unordered_set<uint64_t>> mTypeMap = std::unordered_map<AssetType, std::unordered_set<uint64_t>>();

		static void Init() {
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
			AssetsManager::mAssetTypeByExtension.emplace(Standards::materialExtName, AssetType::MATERIAL);
			AssetsManager::mAssetTypeByExtension.emplace(".png", AssetType::TEXTURE);
			AssetsManager::mAssetTypeByExtension.emplace(".jpg", AssetType::TEXTURE);
			AssetsManager::mAssetTypeByExtension.emplace(".jpeg", AssetType::TEXTURE);
			AssetsManager::mAssetTypeByExtension.emplace(".dds", AssetType::TEXTURE);
			AssetsManager::mAssetTypeByExtension.emplace(".tga", AssetType::TEXTURE);
			AssetsManager::mAssetTypeByExtension.emplace(Standards::sceneExtName, AssetType::SCENE);
			AssetsManager::mAssetTypeByExtension.emplace(Standards::animationExtName, AssetType::ANIMATION);
			AssetsManager::mAssetTypeByExtension.emplace("", AssetType::NONE);

			Texture* defaultTexture = new Texture();
			defaultTexture->mAssetUuid = 1;
			AssetsManager::mTextures.emplace(1, defaultTexture);
		}

		static Asset* NewAsset(uint64_t uuid, AssetType assetType, std::string path) {
			Asset* newAsset = new Asset();
			newAsset->mAssetUuid = uuid;
			newAsset->mAssetPath = std::filesystem::path{ path };
			AssetsManager::AddAsset(newAsset);
			return newAsset;
		}
		static Asset* NewAsset(AssetType assetType, std::filesystem::path path) {
			return NewAsset(assetType, path);
		}
		static Asset* NewAsset(AssetType assetType, std::string path) {
			return NewAsset(assetType, std::filesystem::path{ path });
		}

		static Asset* NewAsset(std::shared_ptr<Asset> asset) {
			return NewAsset(asset->mAssetUuid, AssetType::UNKNOWN, asset->mAssetPath.string());
		}

		static Asset* NewAsset() {
			return NewAsset(Plaza::UUID::NewUUID(), AssetType::UNKNOWN, "");
		}

		static Asset* GetAsset(uint64_t uuid) {
			const auto& it = mAssets.find(uuid);
			if (it != mAssets.end())
				return mAssets.at(uuid);
			return nullptr;
		}

		static Asset* GetAsset(std::filesystem::path path) {
			const auto& it = AssetsManager::mAssetsUuidByPath.find(path);
			if (it != mAssetsUuidByPath.end())
				return GetAsset(it->second);
			return nullptr;
		}

		static void AddAsset(Asset* asset) {
			AssetsManager::mAssets.emplace(asset->mAssetUuid, asset);
			AssetsManager::mTypeMap.find(mAssetTypeByExtension.at(asset->GetExtension()))->second.emplace(asset->mAssetUuid);
			AssetsManager::mAssetsUuidByPath.emplace(asset->mAssetPath, asset->mAssetUuid);
		}

		static bool HasAssetPath(std::string& path) {
			return AssetsManager::mAssetsUuidByPath.find(path) != AssetsManager::mAssetsUuidByPath.end();
		}

		static Texture* GetTexture(uint64_t uuid) {
			const auto& it = mTextures.find(uuid);
			if (it != mTextures.end())
				return mTextures.at(uuid);
			return new Texture();
		}

		static bool TextureExists(uint64_t uuid) {
			const auto& it = mTextures.find(uuid);
			if (it != mTextures.end())
				return true;
			return false;
		}

		static void AddMesh(Mesh* mesh) {
			AssetsManager::mLoadedMeshes.emplace(mesh->meshId, mesh);
		}

		static Mesh* GetMesh(uint64_t uuid) {
			const auto& it = mLoadedMeshes.find(uuid);
			if (it != mLoadedMeshes.end())
				return mLoadedMeshes.at(uuid);
			return nullptr;
		}

		static bool HasMesh(uint64_t uuid) {
			return  mLoadedMeshes.find(uuid) != mLoadedMeshes.end();
		}

		static Animation& AddAnimation(Animation animation) {
			AssetsManager::mLoadedAnimations.emplace(animation.mUuid, animation);
			return mLoadedAnimations.at(animation.mUuid);
		}

		static Animation* GetAnimation(uint64_t uuid) {
			const auto& it = mLoadedAnimations.find(uuid);
			if (it != mLoadedAnimations.end())
				return &mLoadedAnimations.at(uuid);
			return nullptr;
		}

		static Asset* GetSceneAsset(uint64_t uuid) {
			const auto& it = mSceneAssets.find(uuid);
			if (it != mSceneAssets.end())
				return mSceneAssets.at(uuid);
			return nullptr;
		}

		static void AddMaterial(Material* material) {
			mMaterials.emplace(material->mAssetUuid, material);
			mMaterialsNames.emplace(material->mAssetPath.string(), material->mAssetUuid);
		}

		static void AddMaterial(std::shared_ptr<Material> material) {
			mMaterials.emplace(material->mAssetUuid, material);
			mMaterialsNames.emplace(material->mAssetPath.string(), material->mAssetUuid);
		}

		static Material* GetDefaultMaterial() {
			static Material* material = nullptr;
			if (material == nullptr)
				material = new Material();
			return material;
		}

		static Material* GetMaterial(uint64_t uuid) {
			auto it = mMaterials.find(uuid);
			if (it != mMaterials.end()) {
				return it->second.get();
			}
			return AssetsManager::GetDefaultMaterial(); //->DefaultMaterial();
		}

		static std::vector<Material*> GetMaterialsVector(std::vector<uint64_t>& uuids) {
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
				return { AssetsManager::GetDefaultMaterial() };
			return materials;
		}

		static Asset* LoadMetadataAsAsset(std::filesystem::path path) {
			std::ifstream binaryFile(path, std::ios::binary);
			if (!binaryFile.is_open()) {
				std::cerr << "Error opening file for writing!" << std::endl;
				return nullptr;
			}

			uint64_t uuid = 0;
			binaryFile.read(reinterpret_cast<char*>(&uuid), sizeof(uint64_t));
			std::string contentPath = Plaza::Utils::ReadBinaryString(binaryFile);
			std::string extension = Plaza::Utils::ReadBinaryString(binaryFile);

			std::string assetFinalPath = path.parent_path().string() + "\\" + std::filesystem::path{ contentPath }.filename().string();

			binaryFile.close();

			return AssetsManager::NewAsset(uuid, AssetsManager::mAssetTypeByExtension.at(extension), assetFinalPath);
		}

		static Asset* LoadBinaryFileAsAsset(std::filesystem::path path) {
			std::ifstream binaryFile(path, std::ios::binary);
			if (!binaryFile.is_open()) {
				std::cerr << "Error opening file for writing!" << std::endl;
				return nullptr;
			}

			uint64_t uuid = 0;
			binaryFile.read(reinterpret_cast<char*>(&uuid), sizeof(uint64_t));

			binaryFile.close();

			return AssetsManager::NewAsset(uuid, AssetsManager::mAssetTypeByExtension.at(path.extension().string()), path.string());
		}

		static void RemoveAssetUuidPath(uint64_t assetUuid) {
			const auto& it = AssetsManager::mAssetsUuidByPath.find(AssetsManager::GetAsset(assetUuid)->mAssetPath);
			if (it != AssetsManager::mAssetsUuidByPath.end())
				AssetsManager::mAssetsUuidByPath.erase(it);
		}

		static void ChangeAssetPath(uint64_t assetUuid, std::string newPath) {
			AssetsManager::RemoveAssetUuidPath(assetUuid);
			AssetsManager::GetAsset(assetUuid)->mAssetPath = newPath;
			AssetsManager::mAssetsUuidByPath.emplace(std::filesystem::path{ newPath }, assetUuid);
		}

		static Asset* GetAssetOrImport(std::string path, uint64_t uuid = 0, std::string outDirectory = "");

		static void ReadFolderContent(std::string path, bool readSubFolders);
	};
}