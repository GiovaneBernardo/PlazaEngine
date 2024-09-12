#ifndef PLAZA_MATERIAL_H
#define PLAZA_MATERIAL_H

#include "Engine/Components/Component.h"
//#include "Engine/Components/Rendering/Mesh.h"
#include "Engine/Core/Renderer/Texture.h"
#include "Engine/Core/AssetsManager/Asset.h"

namespace Plaza {
	struct Material : public Asset {
	public:
		unsigned int mIndexHandle = -1;
		Texture* diffuse = new Texture(glm::vec4(1.0f), 1.0f);
		Texture* normal = new Texture(glm::vec4(1.0f), 1.0f);
		Texture* metalness = new Texture(glm::vec4(1.0f), 0.35f);
		Texture* roughness = new Texture(glm::vec4(1.0f), 1.0f);
		Texture* height = new Texture(glm::vec4(1.0f), 1.0f);
		Texture* aoMap = new Texture(glm::vec4(1.0f), 1.0f);
		glm::vec2 flip = glm::vec2(1.0f);

		void LoadTextures(std::string relativePath = "") {
			diffuse->Load(relativePath);
			//albedo->Load(relativePath);
			normal->Load(relativePath);
			//specular->Load(relativePath);
			height->Load(relativePath);
			metalness->Load(relativePath);
			roughness->Load(relativePath);
		}

		Material() {
			this->mAssetUuid = Plaza::UUID::NewUUID();
		}
		~Material() = default;

		bool SameAs(Material& other) {
			return (
				this->diffuse->SameAs(*other.diffuse) &&
				//this->specular->SameAs(*other.specular) &&
				this->normal->SameAs(*other.normal) &&
				this->height->SameAs(*other.height)
				);
		}
		static std::vector<uint64_t> GetMaterialsUuids(std::vector<Material*>& materials) {
			std::vector<uint64_t> materialsUuid = std::vector<uint64_t>();
			for (Material* material : materials) {
				materialsUuid.push_back(material->mAssetUuid);
			}
			return materialsUuid;
		}

		template <class Archive>
		void serialize(Archive& archive) {
			uint64_t diffuseUuid = diffuse->mAssetUuid;
			uint64_t normalUuid = normal->mAssetUuid;
			uint64_t metalnessUuid = metalness->mAssetUuid;
			uint64_t roughnessUuid = roughness->mAssetUuid;
			uint64_t heightUuid = height->mAssetUuid;
			uint64_t aoUuid = aoMap->mAssetUuid;

			archive(mAssetUuid, mAssetName, diffuseUuid, normalUuid, metalnessUuid, roughnessUuid, heightUuid, aoUuid, flip);

			if constexpr (Archive::is_loading::value) {
				mDeserializedTexturesUuid = { diffuseUuid, normalUuid, metalnessUuid, roughnessUuid, heightUuid, aoUuid };
			}
		}

		void GetDeserializedTextures();

	private:
		std::vector<uint64_t> mDeserializedTexturesUuid = std::vector<uint64_t>();
	};
}
#endif // PLAZA_MATERIAL_H