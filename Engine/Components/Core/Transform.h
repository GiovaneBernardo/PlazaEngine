#pragma once
#include "Engine/Vendor/glm/glm.hpp"
#include "Engine/Vendor/glm/gtc/matrix_transform.hpp"
#include "Engine/Vendor/glm/gtc/type_ptr.hpp"


#include "Engine/Components/Core/GameObject.h"
#include "Engine/Components/Component.h"

namespace Engine {
	class Transform : public Component {
	public:
		uint64_t uuid;
		glm::vec3 position = { 0,0,0 };
		glm::vec3 worldPosition = { 0, 0, 0 };
		glm::vec3 relativePosition = { 0, 0, 0 };
		glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 worldRotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 scale = { 1,1,1, };
		glm::vec3 worldScale = { 1, 1, 1 };
		glm::mat4 modelMatrix;
		std::string scene = "Editor Scene";
		Transform();
		Transform(const Transform&) = default;
		glm::mat4 GetTransform(glm::vec3 position, glm::vec3 scale);
		glm::mat4 GetTransform(glm::vec3 position);
		glm::mat4 GetTransform();
		void UpdateChildrenTransform(GameObject* gameObject);
		void UpdateChildrenTransform();
		void UpdateChildrenScale(GameObject* gameObject);
		void UpdateChildrenScale();
	};
}