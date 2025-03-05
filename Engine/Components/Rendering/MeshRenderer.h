#pragma once
#include "Engine/Components/Component.h"
// #include "Engine/Components/Rendering/Mesh.h"
// #include "Engine/Core/AssetsManager/AssetsManager.h"
// #include "Engine/Components/Rendering/Material.h"
#include "Engine/Core/RenderGroup.h"
#include "Engine/Components/Core/Transform.h"
#include "Engine/Components/Rendering/Material.h"
#include "Engine/Core/Engine.h"

using namespace std;
namespace Plaza {
	struct Asset;
	struct Material;
	class PLAZA_API MeshRenderer : public Component {
	  public:
		bool castShadows = true;
		string aiMeshName;
		// uint64_t uuid;
		std::string meshName;
		// Mesh* mesh = nullptr;
		// Material* material = nullptr;
		// std::vector<Material*> mMaterials = std::vector<Material*>();
		RenderGroup* renderGroup = nullptr;

		bool instanced = false;
		virtual void OnInstantiate(Scene* scene, uint64_t toInstantiate) override;
		MeshRenderer(Mesh* initialMesh, bool addToScene = false);
		MeshRenderer(Plaza::Mesh* initialMesh, std::vector<Material*> materials, bool addToScene = false);
		MeshRenderer(const MeshRenderer& other) = default;
		~MeshRenderer();
		MeshRenderer() { mUuid = Plaza::UUID::NewUUID(); }

		Mesh* GetMesh() const;
		std::vector<Material*> GetMaterials() const;
		void AddMaterial(Material* newMaterial);
		void ChangeMaterial(Material* newMaterial, unsigned int index = 0);
		void ChangeMesh(Mesh* newMesh);

		template <class Archive> void serialize(Archive& archive) {
			UpdateMaterialsUuids();
			archive(cereal::base_class<Component>(this), PL_SER(castShadows), PL_SER(mMeshUuid),
					PL_SER(mMaterialsUuids));
		}
		uint64_t mMeshUuid = 0;
		std::vector<uint64_t> mMaterialsUuids = std::vector<uint64_t>();
		std::vector<uint64_t> UpdateMaterialsUuids() {
			std::vector<uint64_t> oldMaterialsUuids = mMaterialsUuids;
			// mMaterialsUuids.clear();
			// for (Material* material : mMaterials) {
			//	mMaterialsUuids.push_back(material->mAssetUuid);
			// }
			// if (mMaterialsUuids.size() <= 0) {
			//	mMaterialsUuids = oldMaterialsUuids;
			// }
			return mMaterialsUuids;
		}

	  private:
	};
} // namespace Plaza
