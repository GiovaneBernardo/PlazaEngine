#include "Engine/Core/PreCompiledHeaders.h"
#include "ComponentsInspector.h"
#include "Engine/Components/Physics/RigidBody.h"
#include "Editor/GUI/Utils/DataVisualizer.h"
#include "Editor/GUI/Utils/Utils.h"
#include <functional>
#include <iostream>
#include "Engine/Core/Scene.h"
#include "Engine/ECS/ECSManager.h"

namespace Plaza::Editor {
	Scene* outScene = nullptr;
	uint64_t uuid = 0;
	void UpdateRigidBodyCallbackFloat(float value) { ECS::RigidBodySystem::UpdateRigidBody(outScene, uuid); }
	void UpdateRigidBodyCallbackVec3(glm::vec3 vec3) { ECS::RigidBodySystem::UpdateRigidBody(outScene, uuid); }

	void ComponentsInspector::RigidBodyInspector(Scene* scene, Entity* entity) {
		RigidBody* rigidBody = scene->GetComponent<RigidBody>(entity->uuid);
		if (Utils::ComponentInspectorHeader(rigidBody, "Rigid Body")) {
			ImGui::PushID("RigidBodyInspector");

			physx::PxRigidActor& body = *rigidBody->mRigidActor;
			bool& kinematic = rigidBody->kinematic;
			glm::vec3& gravity = rigidBody->gravity;
			float& dynamicFriction = rigidBody->mDynamicFriction;
			float& staticFriction = rigidBody->mStaticFriction;
			float& restitution = rigidBody->mRestitution;
			float& density = rigidBody->density;
			physx::PxVec3 linearVelocityPxVec;
			if (scene->mRunning)
				linearVelocityPxVec = body.is<physx::PxRigidDynamic>()->getLinearVelocity();
			glm::vec3& linearVelocity =
				*new glm::vec3(linearVelocityPxVec.x, linearVelocityPxVec.y, linearVelocityPxVec.z);

			// if (ImGui::DragFloat3("Gravity: ", &gravity.x, glm::min((gravity.x + gravity.y + gravity.z) / 300.0f,
			// -0.01f), ImGuiInputTextFlags_CallbackEdit)) { UpdateRigidBodyCallbackVec3(gravity); };
			// ImGui::DragFloat("Dynamic Friction: ", &dynamicFriction, glm::min(dynamicFriction / 300.0f, -0.01f));
			outScene = scene;
			uuid = entity->uuid;
			Editor::Utils::DragFloat3("Gravity: ", gravity, 0.01f, &UpdateRigidBodyCallbackVec3);
			Editor::Utils::DragFloat("Dynamic Friction: ", dynamicFriction, 0.01f, &UpdateRigidBodyCallbackFloat, 0.0f);
			Editor::Utils::DragFloat("Static Friction: ", staticFriction, 0.01f, &UpdateRigidBodyCallbackFloat, 0.0f);
			Editor::Utils::DragFloat("Restitution: ", restitution, 0.01f, &UpdateRigidBodyCallbackFloat, 0.0f);
			Editor::Utils::DragFloat("Density: ", density, 0.01f, &UpdateRigidBodyCallbackFloat, 0.0001f);
			Editor::Utils::DragFloat3("Linear Velocity: ", linearVelocity, 0.01f, &UpdateRigidBodyCallbackVec3);

			ImGui::Text("Lock Angular: ");
			bool isXLocked = rigidBody->rigidDynamicLockFlags.isSet(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X);
			if (ImGui::Checkbox("X", &isXLocked)) {
				rigidBody->SetRigidDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, isXLocked);
			}

			bool isYLocked = rigidBody->rigidDynamicLockFlags.isSet(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y);
			if (ImGui::Checkbox("Y", &isYLocked)) {
				rigidBody->SetRigidDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, isYLocked);
			}

			bool isZLocked = rigidBody->rigidDynamicLockFlags.isSet(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z);
			if (ImGui::Checkbox("Z", &isZLocked)) {
				rigidBody->SetRigidDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, isZLocked);
			}

			ImGui::Checkbox("Kinematic: ", &kinematic);
			ImGui::Checkbox("Continuous Detection: ", &rigidBody->continuousDetection);

			ImGui::PopID();
		}
	}
} // namespace Plaza::Editor
