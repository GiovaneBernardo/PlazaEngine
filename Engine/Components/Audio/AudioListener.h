#pragma once
#include "Engine/Components/Component.h"
#include "Engine/Core/Engine.h"

namespace Plaza {
	class PLAZA_API AudioListener : public Component {
	  public:
		glm::vec3 mPosition;
		glm::vec3 mOrientation;
		int volume = 100;
		void UpdateListener();

		template <class Archive> void serialize(Archive& archive) {
			archive(cereal::base_class<Component>(this), PL_SER(mPosition), PL_SER(mOrientation), PL_SER(volume));
		}
	};
} // namespace Plaza
