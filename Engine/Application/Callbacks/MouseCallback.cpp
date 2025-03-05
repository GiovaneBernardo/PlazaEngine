#include "Engine/Core/PreCompiledHeaders.h"
#include "CallbacksHeader.h"
#include "Engine/Core/Input/Input.h"
#include "Engine/Core/Input/Cursor.h"

bool Plaza::Callbacks::rightClickPressed;
bool Plaza::Callbacks::mouseFirstCallback;
bool Plaza::Callbacks::firstMouse = true;

float Plaza::Callbacks::lastX = 0; // Application::Get()->appSizes->appSize.x / 2.0f;
float Plaza::Callbacks::lastY = 0; // Application::Get()->appSizes->appSize.y / 2.0f;

void GetMonitorWorkarea(GLFWwindow* window, int& monitorX, int& monitorY, int& monitorWidth, int& monitorHeight) {
	int windowX, windowY, windowWidth, windowHeight;
	glfwGetWindowPos(window, &windowX, &windowY);
	glfwGetWindowSize(window, &windowWidth, &windowHeight);

	int monitorCount;
	GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);

	for (unsigned int i = 0; i < monitorCount; ++i) {
		GLFWmonitor* monitor = monitors[i];
		int monitorWorkX, monitorWorkY, monitorWorkWidth, monitorWorkHeight;
		glfwGetMonitorWorkarea(monitor, &monitorWorkX, &monitorWorkY, &monitorWorkWidth, &monitorWorkHeight);

		bool windowOnThisMonitor = windowX < monitorWorkX + monitorWorkWidth && windowX + windowWidth > monitorWorkX &&
								   windowY < monitorWorkY + monitorWorkHeight && windowY + windowHeight > monitorWorkY;
		if (windowOnThisMonitor) {
			monitorX = monitorWorkX;
			monitorY = monitorWorkY;
			monitorWidth = monitorWorkWidth;
			monitorHeight = monitorWorkHeight;
			return;
		}
	}

	GLFWmonitor* primary = glfwGetPrimaryMonitor();
	glfwGetMonitorWorkarea(primary, &monitorX, &monitorY, &monitorWidth, &monitorHeight);
}

void HandleMouseWrap(GLFWwindow* window, double& mouseX, double& mouseY, float& lastX, float& lastY) {
	int monitorX, monitorY, monitorWidth, monitorHeight;
	GetMonitorWorkarea(window, monitorX, monitorY, monitorWidth, monitorHeight);

	bool wrapped = false;
	if (mouseX < monitorX) {
		mouseX = monitorX + monitorWidth - 1;
		wrapped = true;
	}
	else if (mouseX >= monitorX + monitorWidth - 1) {
		mouseX = monitorX;
		wrapped = true;
	}
	if (mouseY < monitorY) {
		mouseY = monitorY + monitorHeight - 1;
		wrapped = true;
	}
	else if (mouseY >= monitorY + monitorHeight - 1) {
		mouseY = monitorY;
		wrapped = true;
	}

	if (wrapped) {
		glfwSetCursorPos(window, mouseX, mouseY);
		lastX = (float)mouseX;
		Plaza::Input::Cursor::lastX = (float)mouseX;
		lastY = (float)mouseY;
		Plaza::Input::Cursor::lastY = (float)mouseY;
	}
}

void Plaza::Callbacks::mouseCallback(GLFWwindow* window, double xposIn, double yposIn) {
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);
	if (Application::Get()->focusedMenu == "Editor") {
		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos;
		// Prevent the camera from roughly moving to mouse position when pressed right button
		if (rightClickPressed && mouseFirstCallback) {
			mouseFirstCallback = false;
		}
		else if (rightClickPressed && !mouseFirstCallback) {
			Application::Get()->activeCamera->ProcessMouseMovement(xoffset, yoffset);
			mouseFirstCallback = false;
		}
		else if (!rightClickPressed) {
			mouseFirstCallback = true;
		}

		lastX = xpos;
		lastY = ypos;
	}

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) && !ImGui::IsDragDropActive() &&
		ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
		double x = xposIn;
		double y = yposIn;
		HandleMouseWrap(window, x, y, lastX, lastY);
	}
}
