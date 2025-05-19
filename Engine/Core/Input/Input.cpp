#include "Engine/Core/PreCompiledHeaders.h"
#include "Input.h"
#include "Cursor.h"
namespace Plaza {
	std::unordered_map<int, bool> Input::sCurrentKeyStates = std::unordered_map<int, bool>();
	std::unordered_map<int, bool> Input::sPreviousKeyStates = std::unordered_map<int, bool>();
	void Input::Update() {
		Cursor::Update();

		sPreviousKeyStates = sCurrentKeyStates;

		for (auto& [key, _] : sCurrentKeyStates) {
			sCurrentKeyStates[key] = glfwGetKey(Application::Get()->mWindow->glfwWindow, key) == GLFW_PRESS;
		}
	}

	bool Input::GetKeyDown(int key) {
		if (Application::Get()->focusedMenu == mCheckFocusedMenu) {
			return glfwGetKey(Application::Get()->mWindow->glfwWindow, key) == GLFW_PRESS;
		}
	}

	bool Input::GetKeyDownOnce(int key) {
		if (Application::Get()->focusedMenu != mCheckFocusedMenu) return false;

		bool current = glfwGetKey(Application::Get()->mWindow->glfwWindow, key) == GLFW_PRESS;
		bool previous = sPreviousKeyStates[key];

		// Store current state for next frame
		sCurrentKeyStates[key] = current;

		return current && !previous;
	}
	bool Input::GetKeyReleased(int key) {
		if (Application::Get()->focusedMenu == mCheckFocusedMenu) {
			return glfwGetKey(Application::Get()->mWindow->glfwWindow, key) == GLFW_RELEASE;
		}
	}
	bool Input::GetKeyReleasedOnce() { return false; }

	bool Input::GetMouseDown(int button) {
		if (Application::Get()->focusedMenu == mCheckFocusedMenu) {
			return glfwGetMouseButton(Application::Get()->mWindow->glfwWindow, button) == GLFW_PRESS;
		}
	}

	glm::vec2 Input::GetScreenSize() {
#ifdef EDITOR_MODE
		return glm::vec2(Application::Get()->appSizes->sceneSize.x, Application::Get()->appSizes->sceneSize.y);
#else
		return Application::Get()->appSizes->sceneSize;
#endif
	}

	void Input::SetFocusedMenuCheck(const std::string& menu) { mCheckFocusedMenu = menu; }
} // namespace Plaza
