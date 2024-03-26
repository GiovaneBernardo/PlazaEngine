#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/vector3.h>
#include "ThirdParty/OpenFBX/src/ofbx.h"

namespace Plaza {
	enum AssetExtension {
		OBJ,
		FBX,
		GLTF,
		PNG,
		JPG,
		JPEG,
		DDS
	};
	struct AssetImported {
		std::string mExtension;
		std::string mPath;
	};

	class AssetsImporter {
	public:
		static inline int count = 0;

		static inline glm::vec3 mModelImporterScale = glm::vec3(1.0f);
		static std::string ImportAsset(std::string path, uint64_t uuid = 0);
		static void ImportModel(AssetImported asset);
		static Entity* ImportOBJ(AssetImported asset, std::filesystem::path outPath);
		static Entity* ImportFBX(AssetImported asset, std::filesystem::path outPath);
		static Entity* ImportGLTF(AssetImported asset, std::filesystem::path outPath);
		static std::string ImportTexture(AssetImported asset, uint64_t uuid = 0);
	private:
		static inline const std::unordered_map<std::string, AssetExtension> mExtensionMapping = {
			{".obj", OBJ},
			{".fbx", FBX},
			{".gltf", GLTF},
			{".png", PNG},
			{".jpg", JPG},
			{".jpeg", JPEG},
			{".dds", DDS}
		};

		static Material* FbxModelMaterialLoader(const ofbx::Material* ofbxMaterial, const std::string materialFolderPath, std::unordered_map<std::string, uint64_t>& loadedTextures);
	};
}