#pragma once
#include "PlazaPipeline.h"

namespace Plaza {
	class Picking {
	  public:
		glm::vec2 mResolution;
		PlazaPipeline* mRenderPickingTexturePostEffects;
		PlazaPipeline* mOutlinePostEffects;
		virtual void Init() = 0;
		virtual void DrawSelectedObjectsUuid(Scene* scene) = 0;
		virtual void DrawOutline() = 0;
		virtual uint64_t ReadPickingTexture(glm::vec2 position) = 0;
		virtual uint64_t DrawAndRead(Scene* scene, glm::vec2 position) = 0;
		virtual void Terminate() = 0;
	};
} // namespace Plaza
