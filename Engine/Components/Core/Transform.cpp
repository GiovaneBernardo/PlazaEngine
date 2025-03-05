#pragma once
#include "Engine/Core/PreCompiledHeaders.h"
#include "Engine/Components/Core/Transform.h"

#include "Editor/GUI/gizmo.h"
// #include "Editor/GUI/gizmo.h"
// #include "Engine/Components/Core/Entity.h"
namespace Plaza {
	void TransformComponent::OnInstantiate(Scene* scene, uint64_t toInstantiate) {
		// TransformComponent* component = static_cast<TransformComponent*>(componentToInstantiate);
		// this->SetRelativePosition(component->relativePosition);
		// this->SetRelativeRotation(glm::eulerAngles(component->rotation));
		// this->scale = component->scale;
	}

	TransformComponent::TransformComponent(){};

	glm::vec3 TransformComponent::GetWorldPosition() { return mWorldMatrix[3]; }
	glm::quat TransformComponent::GetWorldQuaternion() { return glm::quat_cast(mWorldMatrix); }
	glm::vec3 TransformComponent::GetWorldRotation() { return glm::eulerAngles(glm::quat_cast(mWorldMatrix)); }
	glm::vec3 TransformComponent::GetWorldScale() {
		glm::vec3 scale;
		scale.x = glm::length(glm::vec3(mWorldMatrix[0]));
		scale.y = glm::length(glm::vec3(mWorldMatrix[1]));
		scale.z = glm::length(glm::vec3(mWorldMatrix[2]));
		return scale;
	}

	// const glm::vec3& TransformComponent::GetWorldPosition() {
	//	return this->modelMatrix[3];
	// }
	//
	// const glm::vec3& TransformComponent::GetWorldRotation() {
	//	glm::vec3 scale;
	//	glm::vec3 translation;
	//	glm::quat rotation;
	//	glm::vec3 skew;
	//	glm::vec4 perspective;
	//	glm::decompose(this->modelMatrix, scale, rotation, translation, skew, perspective);
	//	return glm::eulerAngles(rotation);
	// }
	//
	// const glm::vec3& TransformComponent::GetWorldScale() {
	//	glm::vec3 scale;
	//
	//	scale.x = glm::length(modelMatrix[0]);
	//	scale.y = glm::length(modelMatrix[1]);
	//	scale.z = glm::length(modelMatrix[2]);
	//	return scale;
	// }
	//
	///// <summary>
	///// Returns a Matrix 4x4
	///// Order: WorldPosition - WorldRotation - Scale
	///// </summary>
	///// <param name="position"></param>
	///// <param name="vec3"></param>
	///// <returns></returns>
	// const glm::mat4& TransformComponent::GetTransform(glm::vec3 position, glm::vec3 scale)
	//{
	//	glm::mat4 parentMatrix;
	//	// FIX: Move this to a manager
	//	//if (Scene::GetActiveScene()->entities.at(this->mUuid).parentUuid == 0) {
	//	//	parentMatrix = glm::mat4(1.0f);
	//	//}
	//	//else {
	//	//	parentMatrix = this->GetGameObject()->GetParent().GetComponent<TransformComponent>()->modelMatrix;
	//	//}
	//	this->modelMatrix = parentMatrix * this->GetLocalMatrix();
	//	return this->modelMatrix;
	// }
	//
	// const glm::mat4& TransformComponent::GetTransform() {
	//	return GetTransform(this->worldPosition, this->worldScale);
	// }
	// const glm::mat4& TransformComponent::GetTransform(glm::vec3 position) {
	//	return GetTransform(position, this->worldScale);
	// }
	//
	// void TransformComponent::UpdateWorldMatrix() {
	//	// FIX: Move this to a manager
	//	//PLAZA_PROFILE_SECTION("Transform: Update World Matrix");
	//	//uint64_t parentUuid = Scene::GetActiveScene()->entities.find(this->mUuid)->second.parentUuid;
	//	//if (parentUuid && Scene::GetActiveScene()->transformComponents.find(parentUuid) !=
	//Scene::GetActiveScene()->transformComponents.end()) {
	//	//	glm::mat4 parentModelMatrix =
	//Scene::GetActiveScene()->transformComponents.find(parentUuid)->second.modelMatrix;
	//	//	if (this->lastParentModelMatrix != parentModelMatrix || this->lastLocalMatrix != this->localMatrix) {
	//	//		this->modelMatrix = parentModelMatrix * this->localMatrix;
	//	//		this->lastParentModelMatrix = parentModelMatrix;
	//	//		this->lastLocalMatrix = this->localMatrix;
	//	//	}
	//	//}
	//	//else
	//	//	this->modelMatrix = glm::mat4(1.0f) * this->localMatrix;
	// }
	//
	///// <summary>
	///// Returns the Quaternion of the Transform Local Rotation in radians
	///// </summary>
	///// <returns></returns>
	// const glm::quat& TransformComponent::GetLocalQuaternion() {
	//	return glm::normalize(glm::quat_cast(this->localMatrix));
	// }
	//
	///// <summary>
	///// Returns the Quaternion of the Transform World Rotation in radians
	///// </summary>
	///// <returns></returns>
	// const glm::quat& TransformComponent::GetWorldQuaternion() {
	//	return glm::normalize(glm::quat_cast(this->modelMatrix));
	// }
	//
	// void TransformComponent::UpdateLocalMatrix() {
	//	// FIX: Move this to a manager
	//	//PLAZA_PROFILE_SECTION("Transform: Update Local Matrix");
	//	//if (this->relativePosition != this->lastRelativePositionLocalMatrix || this->rotation !=
	//this->lastRotationLocalMatrix || this->scale != this->lastScaleLocalMatrix) {
	//	//	this->localMatrix = glm::translate(glm::mat4(1.0f), this->relativePosition)
	//	//		* glm::toMat4(rotation)
	//	//		* glm::scale(glm::mat4(1.0f), scale);
	//	//	//return this->localMatrix;
	//	//}
	//	////return this->localMatrix;
	// }
	//
	// const glm::mat4& TransformComponent::GetLocalMatrix() {
	//	return this->localMatrix;
	// }
	//
	// void TransformComponent::UpdateObjectTransform(Entity* entity) {
	//	// FIX: Move this to a manager
	//	//TransformComponent& transform = *entity->GetComponent<TransformComponent>();
	//	//transform.UpdateWorldMatrix();
	//	////transform.UpdatePhysics();
	// }
	//
	//
	// void TransformComponent::UpdateChildrenTransform(Entity* entity) {
	//	UpdateObjectTransform(entity);
	//
	//	for (uint64_t child : entity->childrenUuid) {
	//		UpdateObjectTransform(Scene::GetActiveScene()->GetEntity(child));
	//
	//		UpdateChildrenTransform(Scene::GetActiveScene()->GetEntity(child));
	//	}
	// }
	//
	// void TransformComponent::UpdateSelfAndChildrenTransform() {
	//	ECS::TransformSystem::UpdateSelfAndChildrenTransform(*this, Scene::GetActiveScene());
	//	// FIX: Move this to a manager
	//	//PLAZA_PROFILE_SECTION("Transform: Update Self And Children Transform");
	//	//this->UpdateLocalMatrix();
	//	//this->UpdateWorldMatrix();
	//	//if (Application::Get()->runningScene)
	//	//	this->UpdatePhysics();
	//	//for (uint64_t child : this->GetGameObject()->childrenUuid) {
	//	//	Scene::GetActiveScene()->transformComponents.at(child).UpdateSelfAndChildrenTransform();
	//	//}
	//	//if (haveCamera) {
	//	//	Application::Get()->activeCamera->Update();
	//	//}
	// }
	//
	// void TransformComponent::UpdateChildrenTransform() {
	//	if (mUuid) {
	//		UpdateChildrenTransform(Scene::GetActiveScene()->GetEntity(mUuid));
	//	}
	// }
	//
	// void TransformComponent::MoveTowards(glm::vec3 vector) {
	//	glm::mat4 matrix = this->GetTransform();
	//	glm::vec3 currentPosition = glm::vec3(matrix[3]);
	//	glm::vec3 forwardVector = glm::normalize(glm::vec3(matrix[2]));
	//	glm::vec3 leftVector = glm::normalize(glm::cross(glm::vec3(matrix[1]), forwardVector));
	//	glm::vec3 upVector = glm::normalize(glm::vec3(matrix[1]));
	//	this->relativePosition += leftVector * vector.x + forwardVector * vector.z + upVector * vector.y;
	//	this->UpdateSelfAndChildrenTransform();
	// }
	//
	// const glm::vec3& TransformComponent::MoveTowardsReturn(glm::vec3 vector) {
	//	glm::mat4 matrix = this->GetTransform();
	//	glm::vec3 currentPosition = glm::vec3(matrix[3]);
	//	glm::vec3 forwardVector = glm::normalize(glm::vec3(matrix[2]));
	//	glm::vec3 leftVector = glm::normalize(glm::cross(glm::vec3(matrix[1]), forwardVector));
	//	glm::vec3 upVector = glm::normalize(glm::vec3(matrix[1]));
	//	glm::vec3 outVector = leftVector * vector.x + forwardVector * vector.z + upVector * vector.y;
	//	return outVector;
	// }
	//
	//// Set Functions
	// void TransformComponent::SetRelativePosition(glm::vec3 vector) {
	//	PLAZA_PROFILE_SECTION("Transform: Set Relative Position");
	//	this->relativePosition = vector;
	//	this->UpdateSelfAndChildrenTransform();
	// }
	//
	// void TransformComponent::SetRelativeRotation(glm::quat quat) {
	//	this->rotation = quat;
	//	this->UpdateSelfAndChildrenTransform();
	// }
	//
	// void TransformComponent::SetRelativeScale(glm::vec3 vector) {
	//	// FIX: Move this to a manager
	//	//this->scale = vector;
	//	//this->UpdateSelfAndChildrenTransform();
	//	//auto it = Scene::GetActiveScene()->colliderComponents.find(this->mUuid);
	//	//if (it != Scene::GetActiveScene()->colliderComponents.end()) {
	//	//	it->second.UpdateShapeScale(this->worldScale);
	//	//}
	// }
	//
	// void TransformComponent::UpdatePhysics() {
	//	// FIX: Move this to a manager
	//	//PLAZA_PROFILE_SECTION("Transform: Update Physics");
	//	//auto it = Scene::GetActiveScene()->colliderComponents.find(this->mUuid);
	//	//if (it != Scene::GetActiveScene()->colliderComponents.end()) {
	//	//	it->second.UpdateShapeScale(this->GetWorldScale());
	//	//	it->second.UpdatePose(this);
	//	//}
	// }
	//
	// void TransformComponent::Rotate(glm::vec3 vector) {
	//	// FIX: Move this to a manager
	//	//glm::vec3 position, rotation, scale;
	//	//
	//	//// Convert the rotation component from PxQuat to glm::quat
	//	//glm::mat4 mat4 = glm::translate(glm::mat4(1.0f), this->relativePosition)
	//	//	* glm::toMat4(glm::quat(rotation))
	//	//	* glm::scale(glm::mat4(1.0f), scale);
	//	//glm::quat glmRotation = glm::normalize(glm::quat_cast(mat4));
	//	//
	//	//// Create the glm::mat4 matrix
	//	//glm::mat4 gizmoTransform = glm::mat4_cast(glmRotation); // Initialize the matrix with rotation
	//	//gizmoTransform = glm::translate(gizmoTransform, this->relativePosition); // Apply translation
	//	//
	//	//Editor::Gizmo::DecomposeTransform(gizmoTransform, position, rotation, scale);
	//	//
	//	//// --- Rotation
	//	//glm::mat4 updatedTransform =
	//glm::inverse(this->GetGameObject()->GetParent().GetComponent<TransformComponent>()->GetTransform()) *
	//glm::toMat4(glm::quat(rotation));
	//	//
	//	//glm::vec3 updatedPosition, updatedRotation, updatedScale;
	//	//Editor::Gizmo::DecomposeTransform(updatedTransform, updatedPosition, updatedRotation, updatedScale); // The
	//rotation is radians
	//	//
	//	////this->rotation += glm::vec3(0.0f, 0.2f, 0.0f);
	//	//
	//	//this->UpdateLocalMatrix();
	//	//this->UpdateWorldMatrix();
	//	////this->UpdateSelfAndChildrenTransform();
	// }
} // namespace Plaza
