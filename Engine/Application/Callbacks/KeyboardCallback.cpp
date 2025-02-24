#include "Engine/Core/PreCompiledHeaders.h"
#include "CallbacksHeader.h"
#include "Engine/Core/Time.h"

#include "Editor/DefaultAssets/Models/DefaultModels.h"
#include "Engine/Core/Physics.h"

namespace Plaza {
	void Callbacks::processInput(GLFWwindow* window) {
		if (Application::Get()->focusedMenu == "Editor") {
			ApplicationSizes& appSizes = *Application::Get()->appSizes;
			ApplicationSizes& lastAppSizes = *Application::Get()->lastAppSizes;

			if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
				Application::Get()->activeCamera->MovementSpeedTemporaryBoost = 5.0f;

			if (Application::Get()->activeCamera->isEditorCamera) {
				if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
					Application::Get()->activeCamera->ProcessKeyboard(Plaza::Camera::FORWARD, Time::deltaTime);
				if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
					Application::Get()->activeCamera->ProcessKeyboard(Plaza::Camera::BACKWARD, Time::deltaTime);
				if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
					Application::Get()->activeCamera->ProcessKeyboard(Plaza::Camera::LEFT, Time::deltaTime);
				if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
					Application::Get()->activeCamera->ProcessKeyboard(Plaza::Camera::RIGHT, Time::deltaTime);
				if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
					Application::Get()->activeCamera->ProcessKeyboard(Plaza::Camera::UP, Time::deltaTime);
				if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
					Application::Get()->activeCamera->ProcessKeyboard(Plaza::Camera::DOWN, Time::deltaTime);
				if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
					Application::Get()->activeCamera->ProcessKeyboard(Plaza::Camera::UP, Time::deltaTime);
				if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
					Application::Get()->activeCamera->ProcessKeyboard(Plaza::Camera::DOWN, Time::deltaTime);
			}

			if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS)
				Application::Get()->activeCamera->MovementSpeedTemporaryBoost = 1.0f;
		}
	}
}