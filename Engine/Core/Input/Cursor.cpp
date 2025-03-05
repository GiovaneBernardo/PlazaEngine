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

	glm::vec2 Input::Cursor::GetMousePosition() {
#ifdef EDITOR_MODE
		return glm::vec2(Input::Cursor::lastX - Application::Get()->appSizes->hierarchySize.x,
						 Input::Cursor::lastY - Application::Get()->appSizes->sceneImageStart.y - 35);
#else
		return glm::vec2(Input::Cursor::lastX, Input::Cursor::lastY);
#endif
	}
	glm::vec2 Input::Cursor::GetDeltaMousePosition() { return glm::vec2(deltaX, deltaY); }
} // namespace Plaza
