#include "Engine/Core/PreCompiledHeaders.h"
#include "NewEntityPopup.h"

#include "Editor/DefaultAssets/DefaultAssets.h"
#include "Engine/Core/Physics.h"
#include "Editor/GUI/Hierarchy/Hierarchy.h"
#include "Engine/Core/Scene.h"
#include "Engine/ECS/ECSManager.h"

namespace Plaza::Editor {
	Entity* NewEntity(string name, Entity* parent, Mesh* mesh, bool instanced = true, bool addToScene = true,
					  Scene* scene = nullptr) {
		Entity* obj = scene->NewEntity(name, parent); // new Entity(name, parent, addToScene);
		obj->changingName = true;
		Scene::GetActiveScene()->GetEntity(obj->uuid)->changingName = true;
		HierarchyWindow::Item::firstFocus = true;
		ECS::TransformSystem::UpdateSelfAndChildrenTransform(*scene->GetComponent<TransformComponent>(obj->uuid),
															 nullptr, scene);
		MeshRenderer* meshRenderer = scene->NewComponent<MeshRenderer>(
			obj->uuid); // new MeshRenderer(mesh, { AssetsManager::GetDefaultMaterial() }, true);
		meshRenderer->ChangeMesh(mesh);
		meshRenderer->AddMaterial(AssetsManager::GetDefaultMaterial());
		meshRenderer->instanced = true;

		// meshRenderer->mesh = new Mesh(*mesh);
		// meshRenderer->mMaterials.push_back(AssetsManager::GetDefaultMaterial());
		// RenderGroup* newRenderGroup = new RenderGroup(meshRenderer->mesh, meshRenderer->mMaterials);
		// meshRenderer->renderGroup = Scene::GetActiveScene()->AddRenderGroup(newRenderGroup);
		// meshRenderer->renderGroup->material = make_shared<Material>(*AssetsManager::GetDefaultMaterial());
		Editor::selectedGameObject = obj;

		return obj;
	}
	void Popup::NewEntityPopup::Init(Entity* parent, Scene* scene) {
		if (!parent) {
			parent = Scene::GetActiveScene()->mainSceneEntity;
		}
		// FIX: Remake new entity menu
		if (ImGui::BeginMenu("New Entity")) {
			Entity* entity = nullptr;
			if (ImGui::MenuItem("Empty Entity")) {
				entity = scene->NewEntity("New Entity", parent); // obj = new Entity("New Entity", parent, true);
				ECS::TransformSystem::UpdateSelfAndChildrenTransform(
					*scene->GetComponent<TransformComponent>(entity->uuid), nullptr, scene);
			}

			if (ImGui::MenuItem("Cube")) {
				entity = NewEntity("Cube", parent, DefaultModels::Cube(), true, true, scene);
				TransformComponent* transform = scene->GetComponent<TransformComponent>(entity->uuid);
				Collider* collider = scene->NewComponent<Collider>(entity->uuid);
				ECS::ColliderSystem::CreateShape(collider, transform, ColliderShape::ColliderShapeEnum::BOX);
				ECS::ColliderSystem::InitCollider(scene, collider->mUuid);
			}

			if (ImGui::MenuItem("Sphere")) {
				entity = NewEntity("Sphere", parent, DefaultModels::Sphere(), true, true, scene);
				TransformComponent* transform = scene->GetComponent<TransformComponent>(entity->uuid);
				Collider* collider = scene->NewComponent<Collider>(entity->uuid);
				ECS::ColliderSystem::CreateShape(collider, transform, ColliderShape::ColliderShapeEnum::SPHERE);
				ECS::ColliderSystem::InitCollider(scene, collider->mUuid);
			}

			if (ImGui::MenuItem("Plane")) {
				entity = NewEntity("Plane", parent, DefaultModels::Plane(), true, true, scene);
				TransformComponent* transform = scene->GetComponent<TransformComponent>(entity->uuid);
				transform->mLocalScale = glm::vec3(10.0f, 0.05f, 10.0f);
				ECS::TransformSystem::UpdateSelfAndChildrenTransform(*transform, nullptr, scene);
				Collider* collider = scene->NewComponent<Collider>(entity->uuid);
				ECS::ColliderSystem::CreateShape(collider, transform, ColliderShape::ColliderShapeEnum::PLANE);
				ECS::ColliderSystem::InitCollider(scene, collider->mUuid);
			}

			if (ImGui::MenuItem("Cylinder")) {
				NewEntity("Cylinder", parent, DefaultModels::Cylinder(), true, true, scene);
			}

			if (ImGui::MenuItem("Capsule")) {
				entity = NewEntity("Capsule", parent, DefaultModels::Capsule(), true, true, scene);
				TransformComponent* transform = scene->GetComponent<TransformComponent>(entity->uuid);
				transform->mLocalScale = glm::vec3(1.0f, 1.0f, 1.0f);
				ECS::TransformSystem::UpdateSelfAndChildrenTransform(*transform, nullptr, scene);
				Collider* collider = scene->NewComponent<Collider>(entity->uuid);
				ECS::ColliderSystem::CreateShape(collider, transform, ColliderShape::ColliderShapeEnum::CAPSULE);
				ECS::ColliderSystem::InitCollider(scene, collider->mUuid);
			}

			if (entity)
				Editor::selectedGameObject = entity;

			ImGui::EndMenu();
		}
	}
} // namespace Plaza::Editor
