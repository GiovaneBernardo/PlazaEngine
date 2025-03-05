#pragma once
namespace Plaza {
	class Shadows {
	  public:
		unsigned int mShadowResolution = 4096 / 2;
		glm::vec3 mLightDirection = glm::radians(glm::normalize(glm::vec3(20.0f, 50.0f, 20.0f)));

		virtual void Init() = 0;
		virtual void RenderToShadowMap() = 0;
		virtual void Terminate() = 0;

		std::vector<float> shadowCascadeLevels;
	};
} // namespace Plaza
