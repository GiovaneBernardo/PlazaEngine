#include "Engine/Core/PreCompiledHeaders.h"
#include "Input.h"
#include "Cursor.h"
namespace Plaza {
	void Input::Update() { Cursor::Update(); }

	bool Input::GetKeyDown(int key) {
		if (Application::Get()->focusedMenu == mCheckFocusedMenu) {
			return glfwGetKey(Application::Get()->mWindow->glfwWindow, key) == GLFW_PRESS;
		}
	}

	bool Input::GetKeyDownOnce() { return false; }
	bool Input::GetKeyReleased() { return false; }
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
