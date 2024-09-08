#include "Engine/Core/PreCompiledHeaders.h"
#include "Engine/Components/Rendering/MeshRenderer.h"

namespace Plaza {
	MeshRenderer::MeshRenderer(Plaza::Mesh* initialMesh, bool addToScene) {
		this->mUuid = Plaza::UUID::NewUUID();
		this->mesh = initialMesh;//std::make_unique<Mesh>(initialMesh);
		if (addToScene)
			Application::Get()->activeScene->meshRenderers.emplace_back(this);
	}

	MeshRenderer::MeshRenderer(Plaza::Mesh* initialMesh, std::vector<Material*> materials, bool addToScene) {
		this->mUuid = Plaza::UUID::NewUUID();
		this->mesh = initialMesh;
		this->mMaterials = materials;

		auto renderGroupIt = Application::Get()->activeScene->renderGroupsFindMap.find(std::make_pair(this->mesh->uuid, Material::GetMaterialsUuids(this->mMaterials)));
		if (renderGroupIt != Application::Get()->activeScene->renderGroupsFindMap.end()) {
			this->renderGroup = &Application::Get()->activeScene->renderGroups.at(renderGroupIt->second);
		}
		else {
			RenderGroup* renderGroup = Application::Get()->activeScene->AddRenderGroup(this->mesh, this->mMaterials);//new RenderGroup(this->mesh, this->mMaterials);
			uint64_t renderGroupUuid = renderGroup->uuid;
			//Application::Get()->activeScene->AddRenderGroup(renderGroup);
			//Application::Get()->activeScene->renderGroups.emplace(renderGroupUuid, new RenderGroup(this->mesh, this->material));

			uint64_t meshUuid = this->mesh->uuid;
			//uint64_t materialUuid = this->material->uuid;
			//Application::Get()->activeScene->renderGroupsFindMap.emplace(std::make_pair(meshUuid, materialUuid), renderGroupUuid);
			this->renderGroup = &Application::Get()->activeScene->renderGroups.at(renderGroupUuid);
		}
		if (addToScene)
			Application::Get()->activeScene->meshRenderers.emplace_back(this);

	}

	MeshRenderer::~MeshRenderer() {
		// TODO: FIX MESHRENDERER DELETION
		//if (!Application::Get()->activeScene->mIsDeleting) {
		//	if (this->renderGroup)
		//		Application::Get()->activeScene->RemoveRenderGroup(this->renderGroup->uuid);
		//	Application::Get()->activeScene->RemoveMeshRenderer(this->uuid);
		//}
		//this->renderGroup.~shared_ptr();
	}

	void MeshRenderer::ChangeMaterial(Material* newMaterial, unsigned int index) {
		uint64_t oldUuid = newMaterial->mAssetUuid;
		this->mMaterials[index] = newMaterial;
		this->renderGroup = Application::Get()->activeScene->AddRenderGroup(new RenderGroup(this->mesh, this->mMaterials));
		//this->renderGroup->ChangeMaterial(newMaterial);
	}
	void MeshRenderer::ChangeMesh(Mesh* newMesh) {
		this->mesh = newMesh;
		this->renderGroup = Application::Get()->activeScene->AddRenderGroup(new RenderGroup(newMesh, this->mMaterials));
	}
}