#pragma once
#include "Engine/Core/PreCompiledHeaders.h"
#include "ComponentsInspector.h"
#include "Editor/GUI/Utils/DataVisualizer.h"
#include "Editor/GUI/Utils/Utils.h"
#include "Engine/Core/Scene.h"
#include <math.h>
#include "Engine/ECS/ECSManager.h"

namespace Plaza::Editor {
	void ComponentsInspector::TransformInspector(Scene* scene, Entity* entity) {
		TransformComponent* transform = scene->GetComponent<TransformComponent>(entity->uuid);
		if (Utils::ComponentInspectorHeader(transform, "Transform")) {
			glm::vec3 startingPosition = transform->mLocalPosition;
			glm::quat startingRotation = transform->mLocalRotation;
			glm::vec3 startingScale = transform->mLocalScale;

			glm::vec3& currentPosition = transform->mLocalPosition;
			glm::quat& currentRotation = transform->mLocalRotation;
			glm::vec3 rotationField =
				glm::degrees(glm::eulerAngles(currentRotation)); // glm::degrees(glm::eulerAngles(currentRotation));
			glm::vec3& currentScale = transform->mLocalScale;

			if (Utils::DragFloat3("Position: ", currentPosition, 0.1f)) {
				// ImGuiSliderFlags_Logarithmic
				ECS::TransformSystem::UpdateSelfAndChildrenTransform(*transform, nullptr, scene);
			}

			ImGui::PushID("Rotation");
			if (Utils::DragFloat3("Rotation: ", rotationField, 0.1f)) {
				// ImGuiSliderFlags_Logarithmic
				glm::vec3 radians = glm::radians(rotationField);
				transform->mLocalRotation = glm::quat(radians);
				// currentRotation *= glm::quat(glm::inverse(currentRotation) *
				// glm::quat(radians));//glm::quat(glm::vec3(fmod(radians.x, 360.0f), fmod(radians.y, 360.0f),
				// fmod(radians.z, 360.0f)));
				ECS::TransformSystem::UpdateSelfAndChildrenTransform(*transform, nullptr, scene);
			}
			ImGui::PopID();

			ImGui::PushID("Scale");
			if (Utils::DragFloat3("Scale: ", currentScale, 0.1f)) {
				// ImGuiSliderFlags_Logarithmic
				ECS::TransformSystem::UpdateSelfAndChildrenTransform(*transform, nullptr, scene);
			}

			if (currentPosition != startingPosition || currentRotation != startingRotation ||
				currentScale != startingScale) {
				//					entity->GetComponent<Transform>()->UpdateSelfAndChildrenTransform();
				if (scene->HasComponent<RigidBody>(entity->uuid)) {
					// RigidBody* rigidBody = &Scene::GetActiveScene()->rigidBodyComponents.at(entity->uuid);
					// rigidBody->UpdateGlobalPose();
				}
			}

			if (ImGui::TreeNode("Childs")) {
				for (uint64_t child : entity->childrenUuid) {
					ImGui::Text(Scene::GetActiveScene()->GetEntity(child)->name.c_str());
				}
				ImGui::TreePop();
			}

			ImGui::PopID();
		}
	}
} // namespace Plaza::Editor
