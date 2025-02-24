#include "Engine/Core/PreCompiledHeaders.h"
#include "Editor/GUI/Hierarchy/Hierarchy.h"
#include "Engine/Utils/vectorUtils.h"
#include "Engine/ECS/ECSManager.h"
#include "Engine/Core/Scene.h"

namespace Plaza {
	namespace Editor {
		void payloadDrop(Plaza::Entity* entity, Plaza::Entity* currentObj, ImVec2 treeNodeMin, ImVec2 treeNodeMax, const ImGuiPayload* payload, Plaza::Scene* scene);
		void InsertBefore(Plaza::Entity* payloadObj, Plaza::Entity* currentObj, Plaza::Scene* scene);
		void InsertAsChild(Plaza::Entity* payloadObj, Plaza::Entity* currentObj, Plaza::Scene* scene);
		void InsertAfter(Plaza::Entity* payloadObj, Plaza::Entity* currentObj, Plaza::Scene* scene);


		void HierarchyWindow::Item::HierarchyDragDrop(Entity& entity, Entity* currentObj, ImVec2 treeNodeMin, ImVec2 treeNodeMax, Scene* scene) {
			if (entity.parentUuid && ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadName.c_str())) {
					payloadDrop(&entity, currentObj, treeNodeMin, treeNodeMax, payload, scene);
				}
				ImGui::EndDragDropTarget();
			}
		}

		void payloadDrop(Plaza::Entity* entity, Plaza::Entity* currentObj, ImVec2 treeNodeMin, ImVec2 treeNodeMax, const ImGuiPayload* payload, Plaza::Scene* scene) {
			if (payload->DataSize == sizeof(Plaza::Entity*)) {
				/* The current "object" is the one that the mouse was over when dropped*/
				/* PayloadObj is the dragged object */
				Plaza::Entity* payloadObj = static_cast<Plaza::Entity*>(payload->Data);  // Dereference the pointer			
				std::vector<uint64_t>& currentChildren = currentObj->childrenUuid;
				std::vector<uint64_t>& payloadChildren = payloadObj->childrenUuid;

				ImVec2 mousePos = ImGui::GetMousePos();
				ImVec2 treeNodePos = treeNodeMin;
				ImVec2 treeNodeSize = ImVec2(treeNodeMax.x - treeNodeMin.x, treeNodeMax.y - treeNodeMin.y);
				ImVec2 treeNodeCenter = ImVec2(treeNodePos.x, treeNodePos.y + treeNodeSize.y / 2);

				if (mousePos.y < treeNodeCenter.y - treeNodeSize.y * 0.15f) { // Top
					InsertBefore(payloadObj, currentObj, scene);
				}
				else if (mousePos.y > treeNodeCenter.y + treeNodeSize.y * 0.15f) { // Bottom

					if (currentObj->childrenUuid.size() > 0) {
						InsertAsChild(payloadObj, currentObj, scene);
					}
					else {
						InsertAfter(payloadObj, currentObj, scene);
					}
				}
				else { // Center
					InsertAsChild(payloadObj, currentObj, scene);
				}
			}
		}

		void InsertBefore(Plaza::Entity* payloadObj, Plaza::Entity* currentObj, Plaza::Scene* scene) {
			// Becomes child of the target's parent
			Plaza::Entity& payloadParent = *scene->GetEntity(payloadObj->uuid);
			Plaza::Entity& currentObject = *scene->GetEntity(currentObj->uuid);
			scene->GetEntity(payloadObj->uuid)->childrenUuid.erase(std::remove(scene->GetEntity(payloadObj->uuid)->childrenUuid.begin(), scene->GetEntity(payloadObj->uuid)->childrenUuid.end(), payloadObj->uuid), scene->GetEntity(payloadObj->uuid)->childrenUuid.end()); // Erase this object from its old parent
			scene->GetEntity(payloadObj->uuid)->parentUuid = currentObj->parentUuid; // changes his parent to the target's parent
			payloadObj->parentUuid = currentObj->parentUuid;
			int currentObjIt = std::min(Utils::Vector::indexOf(scene->GetEntity(currentObj->uuid)->childrenUuid, currentObj->uuid), 1);
			scene->GetEntity(scene->GetEntity(payloadObj->uuid)->uuid)->childrenUuid.insert(scene->GetEntity(currentObj->parentUuid)->childrenUuid.begin() + currentObjIt, payloadObj->uuid); // Changes the target position in the vector of the parent's children
			//  PayloadObj index now must be currentObj index - 1, so it comes before

			// Update positions
			ECS::TransformSystem::UpdateSelfAndChildrenTransform(*scene->GetComponent<TransformComponent>(payloadObj->parentUuid), nullptr, scene);
		}

		void InsertAfter(Plaza::Entity* payloadObj, Plaza::Entity* currentObj, Scene* scene) {
			scene->GetEntity(payloadObj->parentUuid)->childrenUuid.erase(std::remove(scene->GetEntity(payloadObj->parentUuid)->childrenUuid.begin(), scene->GetEntity(payloadObj->parentUuid)->childrenUuid.end(), payloadObj->uuid), scene->GetEntity(payloadObj->parentUuid)->childrenUuid.end());
			Scene::GetActiveScene()->GetEntity(payloadObj->uuid)->parentUuid = currentObj->parentUuid;
			Entity& currentObjParent = *scene->GetEntity(currentObj->parentUuid);
			scene->GetEntity(currentObj->parentUuid)->childrenUuid.insert(scene->GetEntity(currentObj->parentUuid)->childrenUuid.begin() + Utils::Vector::indexOf(scene->GetEntity(currentObj->parentUuid)->childrenUuid, currentObj->uuid) + 1, payloadObj->uuid);
			ECS::TransformSystem::UpdateSelfAndChildrenTransform(*scene->GetComponent<TransformComponent>(payloadObj->parentUuid), nullptr, scene);
		}

		void InsertAsChild(Plaza::Entity* payloadObj, Plaza::Entity* currentObj, Scene* scene) {
			payloadObj = Scene::GetActiveScene()->GetEntity(payloadObj->uuid);
			//FIX: Before ECS it was using change parent, that also added the payloadobj to the parent's childrens vector
			scene->GetEntity(payloadObj->parentUuid)->parentUuid = currentObj->uuid;
			ECS::EntitySystem::SetParent(scene, payloadObj, currentObj);
			ECS::TransformSystem::UpdateSelfAndChildrenTransform(*scene->GetComponent<TransformComponent>(payloadObj->uuid), nullptr, scene);
			//payloadObj->GetParent().childrenUuid.erase(std::remove(payloadObj->GetParent().childrenUuid.begin(), payloadObj->GetParent().childrenUuid.end(), payloadObj->uuid), payloadObj->GetParent().childrenUuid.end());
			//Scene::GetActiveScene()->entities.at(payloadObj->uuid).parentUuid = currentObj->uuid;
			//currentObj->childrenUuid.insert(currentObj->childrenUuid.begin(), payloadObj->uuid);
			//payloadObj->GetParent().GetComponent<Transform>()->UpdateSelfAndChildrenTransform();
		}
	}
}