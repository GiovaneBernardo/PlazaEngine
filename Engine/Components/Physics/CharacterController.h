#pragma once
#include "Engine/Components/Component.h"
#include "Engine/Core/Engine.h"

namespace Plaza {
	class PLAZA_API CharacterController : public Component {
	  public:
		CharacterController(uint64_t uuid = 0) { this->mUuid = uuid != 0 ? uuid : Plaza::UUID::NewUUID(); }
		void Init() {};
		void Update() {};
		void Move(glm::vec3 position, float minimumDistance = 5.0f, bool followOrientation = true,
				  float elapsedTime = 1.0f) {};

		bool hasMovedInThisFrame = false;

		std::chrono::system_clock::time_point lastMoveTime;

		template <class Archive> void serialize(Archive& archive) { archive(cereal::base_class<Component>(this)); }
	};
} // namespace Plaza
