#pragma once
#include "Engine/Core/PreCompiledHeaders.h"



#include "Engine/Components/Core/Transform.h"
//#include "Editor/GUI/gizmo.h"
//#include "Engine/Components/Core/Entity.h"
namespace Plaza {
	Transform::Transform() {};

	glm::vec3 Transform::GetWorldPosition() {
		return this->modelMatrix[3];
	}

	glm::vec3 Transform::GetWorldRotation() {
		glm::vec3 scale;
		glm::vec3 translation;
		glm::quat rotation;
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(this->modelMatrix, scale, rotation, translation, skew, perspective);
		return glm::eulerAngles(rotation);
	}

	glm::vec3 Transform::GetWorldScale() {
		glm::vec3 scale;
		glm::vec3 translation;
		glm::quat rotation;
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(this->modelMatrix, scale, rotation, translation, skew, perspective);
		return scale;
	}

	/// <summary>
	/// Returns a Matrix 4x4 
	/// Order: WorldPosition - WorldRotation - Scale
	/// </summary>
	/// <param name="position"></param>
	/// <param name="vec3"></param>
	/// <returns></returns>
	glm::mat4 Transform::GetTransform(glm::vec3 position, glm::vec3 scale)
	{
		glm::mat4 parentMatrix;
		if (Application->activeScene->entities.at(this->uuid).parentUuid == 0) {
			parentMatrix = glm::mat4(1.0f);
		}
		else {
			parentMatrix = Application->activeScene->entities.at(this->GetGameObject()->parentUuid).GetComponent<Transform>()->modelMatrix;
		}
		this->modelMatrix = parentMatrix * this->GetLocalMatrix();
		return this->modelMatrix;
	}

	glm::mat4 Transform::GetTransform() {
		return GetTransform(this->worldPosition, this->worldScale);
	}
	glm::mat4 Transform::GetTransform(glm::vec3 position) {
		return GetTransform(position, this->worldScale);
	}

	void Transform::UpdateWorldMatrix() {
		if (this->GetGameObject()->parentUuid)
			this->modelMatrix = Application->activeScene->transformComponents.at(this->GetGameObject()->parentUuid).modelMatrix * this->localMatrix;
	}

	/// <summary>
	/// Returns the Quaternion of the Transform Local Rotation in radians
	/// </summary>
	/// <returns></returns>
	glm::quat Transform::GetLocalQuaternion() {
		return glm::normalize(glm::quat_cast(this->localMatrix));
	}

	/// <summary>
	/// Returns the Quaternion of the Transform World Rotation in radians
	/// </summary>
	/// <returns></returns>
	glm::quat Transform::GetWorldQuaternion() {
		return glm::normalize(glm::quat_cast(this->modelMatrix));
	}

	void Transform::UpdateLocalMatrix() {
		this->localMatrix = glm::translate(glm::mat4(1.0f), this->relativePosition)
			* glm::toMat4(glm::quat(rotation))
			* glm::scale(glm::mat4(1.0f), scale);
	}

	glm::mat4 Transform::GetLocalMatrix() {
		this->localMatrix = glm::translate(glm::mat4(1.0f), this->relativePosition)
			* glm::toMat4(glm::quat(rotation))
			* glm::scale(glm::mat4(1.0f), scale);
		return this->localMatrix;
	}


	/// <summary>
	/// Rotates around parent, then positionates its relative position based on the rotation and returns this position.
	/// </summary>
	glm::vec3 newWorldPosition(Entity* entity) {
		glm::mat4 rotationMatrix = glm::mat4(1.0f);
		rotationMatrix *= glm::toMat4(glm::quat(Application->activeScene->entities[entity->parentUuid].GetComponent<Transform>()->worldRotation));
		rotationMatrix = glm::scale(rotationMatrix, Application->activeScene->entities[entity->parentUuid].GetComponent<Transform>()->worldScale);
		glm::vec3 transformedPoint = glm::vec3(rotationMatrix * glm::vec4(entity->GetComponent<Transform>()->relativePosition, 1.0f));
		glm::vec3 finalWorldPoint = transformedPoint + Application->activeScene->entities[entity->parentUuid].GetComponent<Transform>()->worldPosition;
		return finalWorldPoint;
	}
	/// <summary>
	/// First rotates with parent rotation and then its own rotation.
	/// </summary>
	glm::vec3 newWorldRotation(Entity* entity) {
		glm::mat4 newRotationMatrix = glm::translate(glm::mat4(1.0f), entity->GetComponent<Transform>()->worldPosition)
			* glm::toMat4(glm::quat(Application->activeScene->entities[entity->parentUuid].GetComponent<Transform>()->worldRotation))
			* glm::toMat4(glm::quat(entity->GetComponent<Transform>()->rotation))
			* glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));

		glm::vec3 eulerAngles = glm::eulerAngles(glm::quat_cast(newRotationMatrix));
		return eulerAngles;

	}



	void Transform::UpdateObjectTransform(Entity* entity) {
		Transform& transform = *entity->GetComponent<Transform>();
		transform.UpdateWorldMatrix();
		//transform.UpdatePhysics();
	}


	void Transform::UpdateChildrenTransform(Entity* entity) {
		UpdateObjectTransform(entity);

		for (uint64_t child : entity->childrenUuid) {
			UpdateObjectTransform(&Application->activeScene->entities[child]);

			UpdateChildrenTransform(&Application->activeScene->entities[child]);
		}
	}

	void Transform::UpdateSelfAndChildrenTransform() {
		this->UpdateLocalMatrix();
		this->UpdateWorldMatrix();
		this->UpdatePhysics();
		for (uint64_t child : this->GetGameObject()->childrenUuid) {
			Application->activeScene->transformComponents.at(child).UpdateSelfAndChildrenTransform();
		}
	}

	void Transform::UpdateChildrenTransform() {
		if (uuid) {
			UpdateChildrenTransform(&Application->activeScene->entities[uuid]);
		}
	}

	void Transform::MoveTowards(glm::vec3 vector) {
		glm::mat4 matrix = this->GetTransform();
		glm::vec3 currentPosition = glm::vec3(matrix[3]);
		// Extract the forward, left, and up vectors from the matrix
		glm::vec3 forwardVector = glm::normalize(glm::vec3(matrix[2]));
		glm::vec3 leftVector = glm::normalize(glm::cross(glm::vec3(matrix[1]), forwardVector));
		glm::vec3 upVector = glm::normalize(glm::vec3(matrix[1]));
		this->relativePosition += forwardVector * vector.x + leftVector * vector.z + upVector * vector.y;
		this->UpdateSelfAndChildrenTransform();
	}

	// Set Functions
	void Transform::SetRelativePosition(glm::vec3 vector) {
		this->relativePosition = vector;
		this->UpdateChildrenTransform();
		if (Collider* collider = GetGameObject()->GetComponent<Collider>()) {
			//collider->UpdatePose(this);
		}
	}

	void Transform::SetRelativeRotation(glm::vec3 vector) {
		this->rotation = vector;
		this->UpdateSelfAndChildrenTransform();
	}

	void Transform::SetRelativeScale(glm::vec3 vector) {
		this->scale = vector;
		//this->UpdateChildrenTransform();
		if (Collider* collider = GetGameObject()->GetComponent<Collider>()) {
			collider->UpdateShapeScale(this->worldScale);
		}
	}

	void Transform::UpdatePhysics() {
		if (Collider* collider = GetGameObject()->GetComponent<Collider>()) {
			collider->UpdatePose(this);
			collider->UpdateShapeScale(this->GetWorldScale());
		}
	}
}