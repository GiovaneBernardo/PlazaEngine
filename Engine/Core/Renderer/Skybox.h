#pragma once
#include "PlazaPipeline.h"

namespace Plaza {
	class Skybox {
	  public:
		glm::vec2 mResolution;
		PlazaPipeline* mSkyboxPostEffect;
		virtual void Init() = 0;
		virtual void DrawSkybox() = 0;
		virtual void Terminate() = 0;
	};
} // namespace Plaza
