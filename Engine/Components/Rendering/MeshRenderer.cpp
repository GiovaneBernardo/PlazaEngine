#include "Engine/Core/PreCompiledHeaders.h"
#include "Engine/Components/Rendering/MeshRenderer.h"
#include "Engine/Core/Scene.h"

namespace Plaza {
	void MeshRenderer::OnInstantiate(Scene* scene, uint64_t toInstantiate) {

	}

	MeshRenderer::MeshRenderer(Plaza::Mesh* initialMesh, bool addToScene) {
		this->mUuid = Plaza::UUID::NewUUID();
		this->mMeshUuid = initialMesh->uuid;
	}

	MeshRenderer::MeshRenderer(Plaza::Mesh* initialMesh, std::vector<Material*> materials, bool addToScene) {
		this->mUuid = Plaza::UUID::NewUUID();
		this->mMeshUuid = initialMesh->uuid;
		for (Material* material : materials) {
			this->mMaterialsUuids.push_back(material->mAssetUuid);
		}

		auto renderGroupIt = Scene::GetActiveScene()->renderGroupsFindMap.find(std::make_pair(this->mMeshUuid, this->mMaterialsUuids));
		if (renderGroupIt != Scene::GetActiveScene()->renderGroupsFindMap.end()) {
			this->renderGroup = &Scene::GetActiveScene()->renderGroups.at(renderGroupIt->second);
		}
		else {
			RenderGroup* renderGroup = Scene::GetActiveScene()->AddRenderGroup(this->GetMesh(), this->GetMaterials());
			uint64_t renderGroupUuid = renderGroup->uuid;
			uint64_t meshUuid = this->mMeshUuid;
			this->renderGroup = &Scene::GetActiveScene()->renderGroups.at(renderGroupUuid);
		}

	}

	MeshRenderer::~MeshRenderer() {
		// TODO: FIX MESHRENDERER DELETION
		//if (!Scene::GetActiveScene()->mIsDeleting) {
		//	if (this->renderGroup)
		//		Scene::GetActiveScene()->RemoveRenderGroup(this->renderGroup->uuid);
		//	Scene::GetActiveScene()->RemoveMeshRenderer(this->uuid);
		//}
		//this->renderGroup.~shared_ptr();
	}

	Mesh* MeshRenderer::GetMesh() const {
		return AssetsManager::GetMesh(mMeshUuid);
	}

	std::vector<Material*> MeshRenderer::GetMaterials() const {
		return AssetsManager::GetMaterialsVector(this->mMaterialsUuids);
	}

	void MeshRenderer::AddMaterial(Material* newMaterial) {
		this->mMaterialsUuids.push_back(newMaterial->mAssetUuid);
		this->renderGroup = Scene::GetActiveScene()->AddRenderGroup(new RenderGroup(this->GetMesh(), this->GetMaterials()));
	}
	void MeshRenderer::ChangeMaterial(Material* newMaterial, unsigned int index) {
		this->mMaterialsUuids[index] = newMaterial->mAssetUuid;
		this->renderGroup = Scene::GetActiveScene()->AddRenderGroup(new RenderGroup(this->GetMesh(), this->GetMaterials()));
	}
	void MeshRenderer::ChangeMesh(Mesh* newMesh) {
		this->mMeshUuid = newMesh ? newMesh->uuid : 0;
		this->renderGroup = Scene::GetActiveScene()->AddRenderGroup(new RenderGroup(newMesh, this->GetMaterials()));
	}
}