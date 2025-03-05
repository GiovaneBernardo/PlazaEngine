#pragma once
#include "Engine/Core/AssetsManager/Metadata/Metadata.h"
#include "ThirdParty/cereal/cereal/types/polymorphic.hpp"

namespace Plaza {
	class AssetsLoader {
	  public:
		static inline const std::unordered_set<std::string> mSupportedLoadFormats = {
			Standards::metadataExtName, Standards::modelExtName,	 Standards::prefabExtName,
			Standards::materialExtName, Standards::animationExtName, Standards::sceneExtName};

		static inline const std::unordered_set<std::string> mSupportedTextureLoadFormats = {".png", ".jpg", ".jpeg",
																							".dds", ".hdr", ".tga"};

		static void LoadAsset(Asset* asset);
		static void LoadMetadata(Asset* asset);
		static std::shared_ptr<Scene> LoadScene(Asset* asset, SerializationMode serializationMode);
		static void LoadPrefab(Asset* asset);
		static void LoadPrefabToMemory(const std::string& path);
		static void LoadPrefabToScene(Model* model, bool loadToScene);
		static void LoadScript(Asset* asset);
		static Texture* LoadTexture(Asset* asset);
		static Material* LoadMaterial(Asset* asset, SerializationMode serializationMode);
		static std::shared_ptr<Model> LoadModel(Asset* asset);
		static Animation& LoadAnimation(Asset* asset, SerializationMode serializationMode);

	  private:
	};
} // namespace Plaza
