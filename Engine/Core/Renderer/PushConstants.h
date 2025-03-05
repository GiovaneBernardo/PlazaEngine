#pragma once
#include "Engine/Core/Renderer/RendererTypes.h"
#include <cstdint>
namespace Plaza {
	class PlPushConstants {
	  public:
		PlPushConstants() {}
		PlPushConstants(PlRenderStage stage, uint32_t offset, uint32_t stride)
			: mStage(stage), mOffset(offset), mStride(stride){};
		virtual ~PlPushConstants() = default;
		PlRenderStage mStage = PL_STAGE_ALL;
		uint32_t mOffset = 0;
		uint32_t mStride = 0;
		void* mData = nullptr;

		template <class Archive> void serialize(Archive& archive) {
			archive(PL_SER(mStage), PL_SER(mOffset), PL_SER(mStride));
		}
	};
} // namespace Plaza
