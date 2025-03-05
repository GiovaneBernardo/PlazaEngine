#pragma once
#include "ThirdParty/glm/glm.hpp"
#include "ThirdParty/glm/gtc/matrix_transform.hpp"
#include "ThirdParty/glm/gtc/type_ptr.hpp"

// #include "Engine/Components/Core/Entity.h"
#include "Engine/Components/Component.h"
#include "Engine/Core/Engine.h"

namespace Plaza {
	class PLAZA_API TransformComponent : public Plaza::Component {
	  public:
		virtual void OnInstantiate(Scene* scene, uint64_t toInstantiate) override;

		glm::vec3 mLocalPosition = {0, 0, 0};
		glm::quat mLocalRotation = glm::quat(0.0f, 0.0f, 0.0f, 0.0f);
		glm::vec3 mLocalScale = {
			1,
			1,
			1,
		};
		glm::mat4 mWorldMatrix = glm::mat4(1.0f);
		glm::mat4 mLocalMatrix = glm::mat4(1.0f);
		bool mDirty = false;

		TransformComponent();
		TransformComponent(const TransformComponent&) = default;

		glm::vec3 GetLocalRotation();
		glm::quat GetLocalQuaternion() { return mLocalRotation; };
		glm::mat4 GetWorldMatrix() { return mWorldMatrix; }
		glm::mat4 GetLocalMatrix() { return mLocalMatrix; }

		glm::vec3 GetWorldPosition();
		glm::vec3 GetWorldRotation();
		glm::quat GetWorldQuaternion();
		glm::vec3 GetWorldScale();

		template <class Archive> void serialize(Archive& archive) {
			archive(cereal::base_class<Component>(this), PL_SER(mLocalPosition), PL_SER(mLocalRotation),
					PL_SER(mLocalScale));
		}

	  private:
		// this->localMatrix = glm::translate(glm::mat4(1.0f), this->relativePosition)
		//	* glm::toMat4(glm::quat(rotation))
		//	* glm::scale(glm::mat4(1.0f), scale);
	};
} // namespace Plaza
