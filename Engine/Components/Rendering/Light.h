#pragma once
#include <ThirdParty/glm/glm.hpp>
#include "Engine/Components/Component.h"
#include "Engine/Core/Engine.h"

namespace Plaza {
	class Light : public Component {
	  public:
		glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
		float radius = 1.0f;
		float cutoff = 1.0f;
		float intensity = 1.0f;

		template <class Archive> void serialize(Archive& archive) {
			archive(cereal::base_class<Component>(this), PL_SER(color), PL_SER(radius), PL_SER(cutoff),
					PL_SER(intensity));
		}
	};
} // namespace Plaza
