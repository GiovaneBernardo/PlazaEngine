#pragma once
#include "Engine/Core/Renderer/PushConstants.h"
#include "Engine/Core/Renderer/RendererTypes.h"

namespace Plaza {
	class PlVkPushConstants : public PlPushConstants {
	  public:
		PlVkPushConstants() {}
		PlVkPushConstants(PlRenderStage stage, uint32_t offset, uint32_t stride)
			: PlPushConstants(stage, offset, stride){};

		template <class Archive> void serialize(Archive& archive) {
			archive(cereal::base_class<PlPushConstants>(this));
		}
	};
} // namespace Plaza
