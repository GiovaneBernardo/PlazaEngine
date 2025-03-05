#include "Engine/Core/PreCompiledHeaders.h"
#include "ECSManager.h"
#include "Engine/Core/Scene.h"

namespace Plaza {
	void ECS::TransformSystem::OnInstantiate(Component* componentToInstantiate, TransformComponent& transform) {
		// TransformComponent* component = static_cast<TransformComponent*>(componentToInstantiate);
		// transform.SetRelativePosition(component->relativePosition);
		// transform.SetRelativeRotation(glm::eulerAngles(component->rotation));
		// transform.scale = component->scale;
	}

	void ECS::TransformSystem::UpdateTransform(TransformComponent& transform, Scene* scene) {
		// mDirty = false;
		// this->UpdateLocalMatrix();
		// this->UpdateWorldMatrix();
		// this->UpdateChildrenTransform();
	}

	void ECS::TransformSystem::UpdateLocalMatrix(TransformComponent& transform, Scene* scene) {
		transform.mLocalMatrix = glm::translate(glm::mat4(1.0f), transform.mLocalPosition) *
								 glm::toMat4(transform.mLocalRotation) *
								 glm::scale(glm::mat4(1.0f), transform.mLocalScale);
	}

	void ECS::TransformSystem::UpdateWorldMatrix(TransformComponent& transform, const glm::mat4& parentWorldMatrix,
												 Scene* scene) {
		transform.mWorldMatrix = parentWorldMatrix * transform.mLocalMatrix;
	}

	void ECS::TransformSystem::UpdateChildrenTransform(TransformComponent& transform, Scene* scene) {
		ECS::TransformSystem::UpdateChildrenTransform(transform, scene);
	}

	void ECS::TransformSystem::SetLocalPosition(TransformComponent& transform, Scene* scene, const glm::vec3& vector,
												bool updateWorldMatrix) {
		transform.mLocalPosition = vector;
		ECS::TransformSystem::UpdateSelfAndChildrenTransform(
			transform, scene->GetComponent<TransformComponent>(scene->GetEntity(transform.mUuid)->parentUuid), scene,
			true);
	}
	void ECS::TransformSystem::SetLocalRotation(TransformComponent& transform, Scene* scene, const glm::quat& quat,
												bool updateWorldMatrix) {
		transform.mLocalRotation = quat;
		ECS::TransformSystem::UpdateSelfAndChildrenTransform(
			transform, scene->GetComponent<TransformComponent>(scene->GetEntity(transform.mUuid)->parentUuid), scene,
			true);
	}
	void ECS::TransformSystem::SetLocalScale(TransformComponent& transform, Scene* scene, const glm::vec3& vector,
											 bool updateWorldMatrix) {
		transform.mLocalScale = vector;
		ECS::TransformSystem::UpdateSelfAndChildrenTransform(
			transform, scene->GetComponent<TransformComponent>(scene->GetEntity(transform.mUuid)->parentUuid), scene,
			true);
	}

	void ECS::TransformSystem::SetWorldPosition(TransformComponent& transform, Scene* scene, const glm::vec3& vector,
												bool updateWorldMatrix) {}
	void ECS::TransformSystem::SetWorldRotation(TransformComponent& transform, Scene* scene, const glm::vec3& vector,
												bool updateWorldMatrix) {}
	void ECS::TransformSystem::SetWorldScale(TransformComponent& transform, Scene* scene, const glm::vec3& vector,
											 bool updateWorldMatrix) {}

	void ECS::TransformSystem::UpdateSelfAndChildrenTransform(TransformComponent& transform,
															  TransformComponent* parentTransform, Scene* scene,
															  bool updateLocal, bool forceUpdateLocal) {
		if (!parentTransform)
			parentTransform = scene->GetComponent<TransformComponent>(scene->GetEntity(transform.mUuid)->parentUuid);

		if (updateLocal)
			ECS::TransformSystem::UpdateLocalMatrix(transform, scene);
		ECS::TransformSystem::UpdateWorldMatrix(
			transform, parentTransform ? parentTransform->mWorldMatrix : glm::mat4(1.0f), scene);

		if (scene->mRunning && scene->HasComponent<Collider>(transform.mUuid))
			ECS::ColliderSystem::UpdatePose(scene->GetComponent<Collider>(transform.mUuid), &transform);
		for (uint64_t child : scene->GetEntity(transform.mUuid)->childrenUuid) {
			ECS::TransformSystem::UpdateSelfAndChildrenTransform(*scene->GetComponent<TransformComponent>(child),
																 &transform, scene, forceUpdateLocal ? true : false,
																 forceUpdateLocal);
		}
	}

	// FIX: Move these functions to CppHelper
	// void TransformComponent::MoveTowards(glm::vec3 vector) {
	//	glm::mat4 matrix = this->GetTransform();
	//	glm::vec3 currentPosition = glm::vec3(matrix[3]);
	//	glm::vec3 forwardVector = glm::normalize(glm::vec3(matrix[2]));
	//	glm::vec3 leftVector = glm::normalize(glm::cross(glm::vec3(matrix[1]), forwardVector));
	//	glm::vec3 upVector = glm::normalize(glm::vec3(matrix[1]));
	//	this->relativePosition += leftVector * vector.x + forwardVector * vector.z + upVector * vector.y;
	//	this->UpdateSelfAndChildrenTransform();
	//}
	//
	// const glm::vec3& TransformComponent::MoveTowardsReturn(glm::vec3 vector) {
	//	glm::mat4 matrix = this->GetTransform();
	//	glm::vec3 currentPosition = glm::vec3(matrix[3]);
	//	glm::vec3 forwardVector = glm::normalize(glm::vec3(matrix[2]));
	//	glm::vec3 leftVector = glm::normalize(glm::cross(glm::vec3(matrix[1]), forwardVector));
	//	glm::vec3 upVector = glm::normalize(glm::vec3(matrix[1]));
	//	glm::vec3 outVector = leftVector * vector.x + forwardVector * vector.z + upVector * vector.y;
	//	return outVector;
	//}
} // namespace Plaza
