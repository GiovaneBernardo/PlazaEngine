#include "Engine/Core/Debugging/Log.h"
#include "Engine/Core/PreCompiledHeaders.h"
#include "Window.h"
#include "Engine/Application/Application.h"
#include "Engine/Application/Callbacks/CallbacksHeader.h"
#include "GLFW/glfw3.h"
#include <ThirdParty/stb/stb_image.h>
#include <cstddef>

using Plaza::Application;
namespace Plaza {
	GLFWwindow* Window::InitGLFWWindow() {
		PL_CORE_INFO("Initializing Window");
		glfwInit();

		if (!glfwVulkanSupported()) {
			PL_CORE_ERROR("Vulkan not supported by GLFW");
			glfwTerminate();
			return nullptr;
		}

		//vkEnumerateInstanceExtensionProperties();
		// TODO: --------- MUST CHANGE THE WAY IT'S STARTING ON SECOND MONITOR --------- //

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
		//glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);
		glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);

#ifdef GAME_MODE
		// glfwWindowHint(GLFW_REFRESH_RATE, 60);
#else
		// glfwWindowHint(GLFW_REFRESH_RATE, 5000);
#endif
		int monitorCount;
		GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
		GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

		GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Plaza Engine", primaryMonitor, nullptr);

		if (!window) {
			std::cout << "error \n";
			glfwTerminate();
			return nullptr;
		}
		
		int width, height;
		glfwGetWindowSize(window, &width, &height);
		glfwMaximizeWindow(window);
		glfwSetWindowMonitor(window, nullptr, 0, 0, mode->width, mode->height, 0);
		
		#ifdef EDITOR_MODE
				GLFWimage images[1];
				images[0].pixels = stbi_load(std::string(Application::Get()->editorPath + "/Images/Other/PlazaEngineLogo32x32.png").c_str(), &images[0].width, &images[0].height, 0, 4);
				glfwSetWindowIcon(window, 1, images);
		#endif

		glfwSetWindowUserPointer(window, this);
		if (window == NULL)
		{
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
			return nullptr;
		}
		//  Set callbacks
		glfwSetFramebufferSizeCallback(window, Callbacks::framebufferSizeCallback);
		glfwSetCursorPosCallback(window, Callbacks::mouseCallback);
		glfwSetScrollCallback(window, Callbacks::scrollCallback);
		glfwSetMouseButtonCallback(window, Callbacks::mouseButtonCallback);
		glfwSetDropCallback(window, Callbacks::dropCallback);
		glfwSetKeyCallback(window, Callbacks::keyCallback);
		
		return window;
	}
}