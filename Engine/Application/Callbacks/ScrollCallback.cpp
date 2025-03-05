#include "Engine/Core/PreCompiledHeaders.h"
#include "CallbacksHeader.h"
#include "Engine/Core/Time.h"
#include "Engine/Utils/glmUtils.h"
void Plaza::Callbacks::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	double mousePosX = 0;
	double mousePosY = 0;
	glfwGetCursorPos(window, &mousePosX, &mousePosY);
	glm::vec2 mousePos = glm::vec2(mousePosX, mousePosY);
	if (Application::Get()->focusedMenu == "Editor" &&
		glm::isInside(mousePos, Application::Get()->appSizes->sceneStart,
					  Application::Get()->appSizes->sceneStart + Application::Get()->appSizes->sceneSize)) {
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
			Application::Get()->activeCamera->MovementSpeed =
				glm::max(Application::Get()->activeCamera->MovementSpeed +
							 (Application::Get()->activeCamera->MovementSpeed * yoffset / 10),
						 0.0001);
		}

		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS &&
			glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
			Application::Get()->activeCamera->ProcessMouseScroll(static_cast<float>(yoffset));
	}
}
