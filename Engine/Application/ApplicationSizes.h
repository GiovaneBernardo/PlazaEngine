#pragma once
#include "ThirdParty/glm/glm.hpp"
namespace Plaza {
	class ApplicationSizes {
	  public:
		float appHeaderSize = 50.0f; // The header size above scene view
		glm::vec2 appSize = {2560, 1080};
		glm::vec2 sceneSize = {1820.0f, 720.0f};
		glm::vec2 sceneStart = {0.0f, 0.0f};
		glm::vec2 hierarchySize = {320.0f, 1000.0f};
		glm::vec2 inspectorSize = {420.0f, 1000.0f};
		glm::vec2 sceneImageStart = {0.0f, 0.0f};
	};
} // namespace Plaza
