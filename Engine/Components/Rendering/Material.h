#pragma once

#include "Engine/Components/Component.h"
// #include "Engine/Components/Rendering/Mesh.h"
#include "ThirdParty/cereal/cereal/types/polymorphic.hpp"
#include "Engine/Core/Renderer/Texture.h"
#include "Engine/Core/AssetsManager/Asset.h"
#include "ThirdParty/glm/common.hpp"

namespace Plaza {
	struct Material : public Asset {
	  public:
		unsigned int mIndexHandle = -1;
		std::shared_ptr<Texture> diffuse = std::make_shared<Texture>(glm::vec4(1.0f), 1.0f);
		std::shared_ptr<Texture> normal = std::make_shared<Texture>(glm::vec4(1.0f), 1.0f);
		std::shared_ptr<Texture> metalness = std::make_shared<Texture>(glm::vec4(1.0f), 0.15f);
		std::shared_ptr<Texture> roughness = std::make_shared<Texture>(glm::vec4(1.0f), 0.5f);
		std::shared_ptr<Texture> height = std::make_shared<Texture>(glm::vec4(1.0f), 1.0f);
		std::shared_ptr<Texture> aoMap = std::make_shared<Texture>(glm::vec4(1.0f), 1.0f);
		glm::vec2 flip = glm::vec2(1.0f);

		void LoadTextures(std::string relativePath = "") {
			diffuse->Load(relativePath);
			// albedo->Load(relativePath);
			normal->Load(relativePath);
			// specular->Load(relativePath);
			height->Load(relativePath);
			metalness->Load(relativePath);
			roughness->Load(relativePath);
		}

		Material() { this->mAssetUuid = Plaza::UUID::NewUUID(); }
		~Material() = default;

		bool SameAs(Material& other) {
			return (this->diffuse->SameAs(*other.diffuse) &&
					// this->specular->SameAs(*other.specular) &&
					this->normal->SameAs(*other.normal) && this->height->SameAs(*other.height));
		}
		static std::vector<uint64_t> GetMaterialsUuids(std::vector<Material*>& materials) {
			std::vector<uint64_t> materialsUuid = std::vector<uint64_t>();
			for (Material* material : materials) {
				materialsUuid.push_back(material->mAssetUuid);
			}
			return materialsUuid;
		}

		template <class Archive> void serialize(Archive& archive) {
			uint64_t diffuseUuid = diffuse->mAssetUuid;
			uint64_t normalUuid = normal->mAssetUuid;
			uint64_t metalnessUuid = metalness->mAssetUuid;
			uint64_t roughnessUuid = roughness->mAssetUuid;
			uint64_t heightUuid = height->mAssetUuid;
			uint64_t aoUuid = aoMap->mAssetUuid;

			archive(PL_SER(mAssetUuid), PL_SER(mAssetName), PL_SER(diffuse), PL_SER(normal), PL_SER(metalness),
					PL_SER(roughness), PL_SER(height), PL_SER(aoMap), PL_SER(flip));

			if constexpr (Archive::is_loading::value) {
				uint64_t diffuseUuid = diffuse->mAssetUuid;
				uint64_t normalUuid = normal->mAssetUuid;
				uint64_t metalnessUuid = metalness->mAssetUuid;
				uint64_t roughnessUuid = roughness->mAssetUuid;
				uint64_t heightUuid = height->mAssetUuid;
				uint64_t aoUuid = aoMap->mAssetUuid;
				mDeserializedTexturesUuid = {diffuseUuid, normalUuid, metalnessUuid, roughnessUuid, heightUuid, aoUuid};
			}
		}

		void GetDeserializedTextures();

	  private:
		std::vector<uint64_t> mDeserializedTexturesUuid = std::vector<uint64_t>();
	};
} // namespace Plaza
