#pragma once
#define GLFW_INCLUDE_VULKAN
#include "ThirdParty/GLFW/include/GLFW/glfw3.h"

namespace Plaza {
	class Window {
	  public:
		GLFWwindow* glfwWindow;
		GLFWwindow* InitGLFWWindow();
	};
} // namespace Plaza
