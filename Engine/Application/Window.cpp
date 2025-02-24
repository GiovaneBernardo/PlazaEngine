#include "Engine/Core/PreCompiledHeaders.h"
#include "Window.h"
#include "Engine/Application/Application.h"
#include "Engine/Application/Callbacks/CallbacksHeader.h"
#include <ThirdParty/stb/stb_image.h>

using Plaza::Application;
namespace Plaza {
	GLFWwindow* Window::InitGLFWWindow() {
		PL_CORE_INFO("Initializing Window");
		glfwInit();

		//vkEnumerateInstanceExtensionProperties();
		// TODO: --------- MUST CHANGE THE WAY IT'S STARTING ON SECOND MONITOR --------- //

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
		glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

#ifdef GAME_MODE
		// glfwWindowHint(GLFW_REFRESH_RATE, 60);
#else
		// glfwWindowHint(GLFW_REFRESH_RATE, 5000);
#endif
		GLFWwindow* window = glfwCreateWindow(Application::Get()->appSizes->appSize.x, Application::Get()->appSizes->appSize.y, "Plaza Engine", 0, 0); // ------------------------------------------------------------------ Change this to 0
		if (!window) {
			// Handle window creation failure
			// You can print an error message or log it
			std::cout << "error \n";
			glfwTerminate(); // Clean up GLFW before exiting
			return nullptr;
		}

#ifdef EDITOR_MODE
		//GLFWimage images[1];
		//images[0].pixels = stbi_load(std::string(Application::Get()->editorPath + "/Images/Other/PlazaEngineLogo.png").c_str(), &images[0].width, &images[0].height, 0, 4); //rgba channels 
		//glfwSetWindowIcon(window, 1, images);
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