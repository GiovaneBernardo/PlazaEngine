#pragma once
#include "Engine/Core/PreCompiledHeaders.h"
#include "ComponentsInspector.h"
#include "Engine/Components/Physics/RigidBody.h"
#include "Engine/Components/Physics/Collider.h"
#include "Editor/GUI/Utils/DataVisualizer.h"
#include <functional>
#include <iostream>
#include "Engine/Core/Scene.h"
#include "Engine/Core/Physics.h"
namespace Plaza::Editor {
	void AddChildrenMeshShape(Scene* scene, Collider* collider, uint64_t parentUuid) {
		for (uint64_t childUuid : Scene::GetActiveScene()->GetEntity(parentUuid)->childrenUuid) {
			if (scene->HasComponent<MeshRenderer>(childUuid)) {
				collider->AddConvexMeshShape(scene->GetComponent<MeshRenderer>(childUuid)->GetMesh());
			}
			AddChildrenMeshShape(scene, collider, childUuid);
		}
	}

	void ComponentsInspector::ColliderInspector(Scene* scene, Entity* entity) {
		Collider* collider = scene->GetComponent<Collider>(entity->uuid);
		ImGui::SetNextItemOpen(true);
		ImVec2 old = ImGui::GetCursorPos();
		float arrowSize = old.x + 30.0f;
		ImGui::SetCursorPos(ImVec2(ImGui::CalcTextSize("Collider").x + arrowSize, ImGui::GetCursorPos().y));
		if (ImGui::Button("Remove Component")) {
			scene->RemoveComponent<Collider>(collider->mUuid);
		}
		ImGui::SameLine();
		ImGui::SetCursorPos(old);
		bool header = ImGui::CollapsingHeader("Collider", ImGuiTreeNodeFlags_DefaultOpen);
		ImGui::SameLine();
		ImGui::Button("Remove Component");
		if (header) {
			ImGui::PushID("ColliderInspector");

			// Button for opening collider popup
			if (ImGui::Button("Add Shape")) {
				ImGui::OpenPopup("AddShapeContexMenu"); // Open the popup when the button is clicked
			}

			for (auto& shape : collider->mShapes) {
				ImGui::Text("Shape");
			}

			// Check if the context menu is open
			if (ImGui::BeginPopupContextWindow("AddShapeContexMenu")) {
				if (ImGui::IsMouseReleased(1)) // Check if right mouse button was released
				{
					ImGui::CloseCurrentPopup(); // Close the context menu on right-click
				}

				TransformComponent& transform = *scene->GetComponent<TransformComponent>(collider->mUuid);
				// Create a dynamic rigid body

				physx::PxMaterial* defaultMaterial = Physics::defaultMaterial;
				if (ImGui::MenuItem("Box")) {
					physx::PxBoxGeometry geometry(transform.mLocalScale.x / 2.1, transform.mLocalScale.y / 2.1, transform.mLocalScale.z / 2.1);
					physx::PxShape* shape = Physics::m_physics->createShape(geometry, *defaultMaterial);
					collider->AddShape(new ColliderShape(shape, ColliderShape::ColliderShapeEnum::BOX, 0));
				}
				if (ImGui::MenuItem("Plane")) {
					physx::PxBoxGeometry geometry(transform.mLocalScale.x / 2.1, 0.001f, transform.mLocalScale.z / 2.1);
					physx::PxShape* shape = Physics::m_physics->createShape(geometry, *defaultMaterial);
					collider->AddShape(new ColliderShape(shape, ColliderShape::ColliderShapeEnum::PLANE, 0));
				}
				if (ImGui::MenuItem("Sphere")) {
					physx::PxSphereGeometry geometry(1.0f);
					physx::PxShape* shape = Physics::m_physics->createShape(geometry, *defaultMaterial);
					collider->AddShape(new ColliderShape(shape, ColliderShape::ColliderShapeEnum::SPHERE, 0));
				}
				if (ImGui::MenuItem("Capsule")) {
					physx::PxCapsuleGeometry geometry(0.5f, 1.0f);
					physx::PxShape* shape = Physics::m_physics->createShape(geometry, *defaultMaterial);
					collider->AddShape(new ColliderShape(shape, ColliderShape::ColliderShapeEnum::CAPSULE, 0));
				}
				if (ImGui::MenuItem("Mesh")) {
					if (scene->HasComponent<MeshRenderer>(collider->mUuid)) {
						collider->AddMeshShape(scene->GetComponent<MeshRenderer>(collider->mUuid)->GetMesh());
					}
					if (scene->HasComponent<RigidBody>(collider->mUuid)) {
						AddChildrenMeshShape(scene, collider, collider->mUuid);

					}
				}
				if (ImGui::MenuItem("Convex Mesh")) {
					if (scene->HasComponent<MeshRenderer>(collider->mUuid)) {
						collider->AddConvexMeshShape(scene->GetComponent<MeshRenderer>(collider->mUuid)->GetMesh());
					}
					if (scene->HasComponent<RigidBody>(collider->mUuid)) {
						AddChildrenMeshShape(scene, collider, collider->mUuid);

					}
				}

				ImGui::EndPopup();
			}

			//for (physx::PxShape* shape : collider->mShapes) {
			//	ImGui::Text(shape->getName());


			//}

			ImGui::PopID();
		}
		// Button outside the header
		if (ImGui::Button("Button Outside Header")) {
			std::cout << "Button outside header clicked" << std::endl;
		}
	}
}