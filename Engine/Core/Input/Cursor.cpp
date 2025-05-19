#include "Engine/Core/PreCompiledHeaders.h"
#include "Input.h"
#include "Cursor.h"
namespace Plaza {
	bool Input::Cursor::show = true;
	double Input::Cursor::lastX = 0;
	double Input::Cursor::lastY = 0;
	double Input::Cursor::deltaX = 0;
	double Input::Cursor::deltaY = 0;
	void Input::Cursor::Update() {
		// if (Application::Get()->focusedMenu == "Scene") {
		double currentX;
		double currentY;
		glfwGetCursorPos(Application::Get()->mWindow->glfwWindow, &currentX, &currentY);

		deltaX = currentX - lastX;
		deltaY = currentY - lastY;

		lastX = currentX;
		lastY = currentY;
		//}
	}

	void Input::Cursor::SetX(float value) { glfwSetCursorPos(Application::Get()->mWindow->glfwWindow, value, lastY); }

	void Input::Cursor::SetY(float value) { glfwSetCursorPos(Application::Get()->mWindow->glfwWindow, lastX, value); }

	const glm::vec2& Input::Cursor::GetMousePosition() {
#ifdef EDITOR_MODE
		return glm::vec2(lastX - Application::Get()->appSizes->hierarchySize.x,
						 lastY - Application::Get()->appSizes->sceneImageStart.y);
#else
		return glm::vec2(this->lastX, this->lastY);
#endif
	}
	const glm::vec2& Input::Cursor::GetDeltaMousePosition() {
		if (deltaX != 0 || deltaY != 0) {
			PL_CORE_INFO("Puts");
		}
		return glm::vec2(deltaX, deltaY);
	}
} // namespace Plaza
