#include "Window.h"

#include "Engine/Core/PreCompiledHeaders.h"

//#include "Engine/Application/Callbacks/Callbacks.h"
#include "Engine/Application/Callbacks/CallbacksHeader.h"
//using namespace Plaza;
//using Application->;

//using namespace Plaza;
//using Application->;
using Plaza::ApplicationClass;
namespace Plaza {
	GLFWwindow* WindowClass::InitGLFWWindow() {
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		// glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		// glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		// glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		// glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		// 
		// if (Application->mRenderer->api = RendererAPI::OpenGL) {
		// 	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		// 	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
		// }
		// glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);
		// glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
		// glfwWindowHint(GLFW_SAMPLES, 1);

		//vkEnumerateInstanceExtensionProperties();
		// TODO: --------- MUST CHANGE THE WAY IT'S STARTING ON SECOND MONITOR --------- //

		// // Get monitors and create GLFW window on second monitor
		// int monitorCount;
		// GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
		// GLFWmonitor* secondMonitor = monitors[1]; // ------------------------------------------------------------------ Change this to 0
		//const GLFWvidmode* videoMode = glfwGetVideoMode(secondMonitor);
		// const GLFWvidmode* mode = glfwGetVideoMode(secondMonitor);
		// glfwWindowHint(GLFW_RED_BITS, mode->redBits);
		// glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
		// glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
#ifdef GAME_REL
		// glfwWindowHint(GLFW_REFRESH_RATE, 60);
#else
		// glfwWindowHint(GLFW_REFRESH_RATE, 5000);
#endif
		GLFWwindow* window = glfwCreateWindow(Application->appSizes->appSize.x / 2, Application->appSizes->appSize.y / 2, "Plaza Engine", nullptr, nullptr); // ------------------------------------------------------------------ Change this to 0
		if (!window) {
			// Handle window creation failure
			// You can print an error message or log it
			std::cout << "error \n";
			glfwTerminate(); // Clean up GLFW before exiting
			return nullptr;
		}

		GLFWimage images[1];
		images[0].pixels = stbi_load("C:\\Users\\Giovane\\Downloads\\PlazaEngineLogo.png", &images[0].width, &images[0].height, 0, 4); //rgba channels 
		glfwSetWindowIcon(window, 1, images);

		glfwSetWindowUserPointer(window, this);
		// // Make it be fullscreen
		// glfwMakeContextCurrent(window);
		// glfwSetWindowMonitor(window, NULL, 0, 0, Application->appSizes->appSize.x, Application->appSizes->appSize.y, GLFW_DONT_CARE);
		// //glfwSetWindowPos(window, -2560, 0);
		// //glfwMaximizeWindow(window);
		// glfwMaximizeWindow(window);
		// glfwSetWindowPos(window, 0, 0);


		if (window == NULL)
		{
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
			return nullptr;
		}
		//  Set callbacks
		 glfwSetFramebufferSizeCallback(window, ApplicationClass::Callbacks::framebufferSizeCallback);
		// glfwSetCursorPosCallback(window, ApplicationClass::Callbacks::mouseCallback);
		// glfwSetScrollCallback(window, ApplicationClass::Callbacks::scrollCallback);
		// glfwSetMouseButtonCallback(window, ApplicationClass::Callbacks::mouseButtonCallback);
		// glfwSetDropCallback(window, ApplicationClass::Callbacks::dropCallback);
		// glfwSetKeyCallback(window, ApplicationClass::Callbacks::keyCallback);


#ifndef GAME_REL
		// glfwSwapInterval(0);
#endif

		// if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		// {
		// 	std::cout << "Failed to initialize GLAD" << std::endl;
		// 	return nullptr;
		// }
		// 
		// stbi_set_flip_vertically_on_load(false);
		return window;
	}
}