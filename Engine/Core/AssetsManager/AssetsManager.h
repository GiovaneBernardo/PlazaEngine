#pragma once
#include "Engine/Core/AssetsManager/AssetsType.h"
#include "Engine/Core/Standards.h"
#include "Engine/Core/Renderer/Model.h"
#include "Engine/Components/Physics/Collider.h"
#include "Engine/Components/Rendering/MeshRenderer.h"
#include "Engine/Components/Rendering/Material.h"
#include "Engine/Components/Rendering/AnimationComponent.h"
#include "Engine/Core/AssetsManager/Serializer/AssetsSerializer.h"
#include "Asset.h"
#include "Engine/Components/Physics/PhysicsMaterial.h"
#include "Engine/Core/Scripting/Script.h"
#ifndef COMPILING_GAME_DLL
#include "battery/embed.hpp"
#endif
#include <string>
#include <string_view>
#include "Engine/Core/Engine.h"

namespace Plaza {
	class Prefab;
	class AssetsListStructure : public std::unordered_map<uint64_t, Asset*> {
	public:

	};

	class PLAZA_API AssetsManager {
	public:
		static inline AssetsListStructure mAssets = AssetsListStructure();
		static inline std::map<std::filesystem::path, uint64_t> mAssetsUuidByPath = std::map<std::filesystem::path, uint64_t>();
		static inline std::map<std::string, AssetType> mAssetTypeByExtension = std::map<std::string, AssetType>();
		static inline std::set<AssetType> mAssetsTypesWithMetaData = std::set<AssetType>();

		static inline std::unordered_map<uint64_t, Texture*> mTextures = std::unordered_map<uint64_t, Texture*>();
		static inline std::unordered_map<uint64_t, shared_ptr<Material>> mMaterials = std::unordered_map<uint64_t, shared_ptr<Material>>();
		static inline std::unordered_map<std::string, uint64_t> mMaterialsNames = std::unordered_map<std::string, uint64_t>();
		static inline std::unordered_map<uint64_t, shared_ptr<Prefab>> mPrefabs = std::unordered_map<uint64_t, shared_ptr<Prefab>>();
		static inline std::unordered_map<uint64_t, shared_ptr<Model>> mModels = std::unordered_map<uint64_t, std::shared_ptr<Model>>();
		static inline std::unordered_map<PhysicsMaterial, std::shared_ptr<PhysicsMaterial>> mPhysicsMaterials = std::unordered_map<PhysicsMaterial, std::shared_ptr<PhysicsMaterial>>();
		static inline std::unordered_map<uint64_t, Mesh*> mLoadedMeshes = std::unordered_map<uint64_t, Mesh*>();
		static inline std::unordered_map<uint64_t, Animation> mLoadedAnimations = std::unordered_map<uint64_t, Animation>();
		static inline std::unordered_map<uint64_t, Asset*> mSceneAssets = std::unordered_map<uint64_t, Asset*>();
		static inline std::unordered_map<uint64_t, Script*> mScripts = std::unordered_map<uint64_t, Script*>();
		static inline std::unordered_map<std::string, std::filesystem::path> mShaders = std::unordered_map<std::string, std::filesystem::path>();


		static inline std::unordered_map<AssetType, std::unordered_set<uint64_t>> mTypeMap = std::unordered_map<AssetType, std::unordered_set<uint64_t>>();

		static void Init();

		static AssetType GetExtensionType(std::string extension);

		static bool AssetTypeContainsMetaData(AssetType assetType);
		static bool AssetContainsMetaData(Asset* asset);
		static std::filesystem::path GetAssetMetaDataPath(std::filesystem::path assetPath);

		static void RenameAsset(Asset* asset, std::string oldPath, std::string newPath);
		static void RenameMetaData(Asset* asset, std::string oldPath, std::string newPath);
		static void AfterRename(Asset* renamedAsset, std::string oldPath, std::string newPath);

		template<typename T>
		static T* NewAsset(uint64_t uuid, std::string path) {
			T* newAsset = new T();
			newAsset->mAssetUuid = uuid;
			newAsset->mAssetPath = std::filesystem::path{ path };
			newAsset->mAssetName = newAsset->mAssetPath.filename().string();
			AssetsManager::AddAsset(static_cast<Asset*>(newAsset));
			return newAsset;
		}
		template<typename T>
		static T* NewAsset(std::string path) {
			return NewAsset<T>(Plaza::UUID::NewUUID(), path);
		}
		template<typename T>
		static T* NewAsset(std::shared_ptr<Asset> asset) {
			return NewAsset<T>(asset->mAssetUuid, asset->mAssetPath.string());
		}
		template<typename T>
		static T* NewAsset() {
			return NewAsset<T>(Plaza::UUID::NewUUID(), "");
		}

		static Asset* GetAsset(uint64_t uuid);
		static Asset* GetAsset(std::filesystem::path path);
		static void AddAsset(Asset* asset);
		static bool HasAssetPath(std::string& path);

		static Texture* GetTexture(uint64_t uuid);
		static bool TextureExists(uint64_t uuid);

		static void AddMesh(Mesh* mesh);
		static Mesh* GetMesh(uint64_t uuid);
		static bool HasMesh(uint64_t uuid);

		static void AddModel(std::shared_ptr<Model> model);
		static void AddPrefab(std::shared_ptr<Prefab> prefab);
		static Prefab* GetPrefab(uint64_t uuid);

		static Animation& AddAnimation(Animation animation);
		static Animation* GetAnimation(uint64_t uuid);

		static Asset* GetSceneAsset(uint64_t uuid);

		static void AddMaterial(Material* material);
		static void AddMaterial(std::shared_ptr<Material> material);
		static Material* GetDefaultMaterial();
		static Material* GetMaterial(uint64_t uuid);
		static std::vector<Material*> GetMaterialsVector(const std::vector<uint64_t>& uuids);

		static PhysicsMaterial& GetPhysicsMaterial(PhysicsMaterial& other);
		static PhysicsMaterial& GetPhysicsMaterial(float staticFriction, float dynamicFriction, float restitution);

		static void AddScript(Script* script);
		static Script* GetScript(uint64_t uuid);
		static void RemoveScript(uint64_t uuid);

		static void AddShaders(Asset* asset) {
			mShaders.emplace(asset->mAssetPath.filename().string(), asset->mAssetPath);
		}
		static std::string GetShadersPath(const std::string& path) {
			if (mShaders.find(path) != mShaders.end()) {
				return mShaders.at(path).string();
			}
			return "";
		}

		static Asset* LoadMetadataAsAsset(std::filesystem::path path);
		static Asset* LoadBinaryFileAsAsset(std::filesystem::path path);
		static void RemoveAssetUuidPath(uint64_t assetUuid);
		static void ChangeAssetPath(uint64_t assetUuid, std::string newPath);
		static Asset* GetAssetOrImport(std::string path, uint64_t uuid = 0, std::string outDirectory = "");

#ifndef COMPILING_GAME_DLL
		template <b::embed_string_literal identifier>
		static const char* GetEmbedResource() {
			return b::embed<identifier>().data();
		}
#else
		template <std::basic_string identifier>
		static const char* GetEmbedResource() {
			return nullptr;
		}
#endif

		static void ReadFolderContent(std::string path, bool readSubFolders);
	};
}