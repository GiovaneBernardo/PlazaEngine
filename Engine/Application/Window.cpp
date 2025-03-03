#include "Engine/Core/Debugging/Log.h"
#include "Engine/Core/PreCompiledHeaders.h"
#include "Window.h"
#include "Engine/Application/Application.h"
#include "Engine/Application/Callbacks/CallbacksHeader.h"
#include "GLFW/glfw3.h"
#include <ThirdParty/stb/stb_image.h>
#include <cstddef>

std::vector<unsigned char> load_file_to_memory(const char* filepath) {
	std::ifstream file(filepath, std::ios::binary | std::ios::ate);
	if (!file) {
		std::cerr << "Failed to open file: " << filepath << std::endl;
		return {};
	}
	size_t file_size = file.tellg();
	file.seekg(0, std::ios::beg);
	std::vector<unsigned char> buffer(file_size);
	file.read(reinterpret_cast<char*>(buffer.data()), file_size);
	return buffer;
}

std::vector<unsigned char> read_file(const std::string& filename) {
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file: " + filename);
	}
	
	std::streamsize file_size = file.tellg();
	if (file_size <= 0) {
		throw std::runtime_error("File is empty or size could not be determined: " + filename);
	}
	
	std::vector<unsigned char> buffer(static_cast<size_t>(file_size));
	file.seekg(0, std::ios::beg);
	file.read(reinterpret_cast<char*>(buffer.data()), file_size);
	
	if (!file) {
		throw std::runtime_error("Failed to read file: " + filename);
	}

	return buffer;
}

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
		glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
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
		glfwSetWindowMonitor(window, nullptr, 0, 0, mode->width, mode->height, 0);

		width = 0;
		height = 0;
#ifdef EDITOR_MODE
GLFWimage images[1] = {GLFWimage()};
//images[0].pixels = static_cast<unsigned char*>(stbi_load(std::string(Application::Get()->editorPath + "/Images/Other/PlazaEngineLogo.png").c_str(), &images[0].width, &images[0].height, &channels, 4)); //rgba channels 

			int channels = 0;

			auto datae = read_file(std::string(Application::Get()->editorPath + "/Images/Other/PlazaEngineLogo.png").c_str());
			
			images[0].pixels = static_cast<unsigned char*>(stbi_load_from_memory(datae.data(), datae.size(), &width, &height, &channels, 4));

			//images[0].pixels = static_cast<unsigned char*>(data);
			images[0].width = width;
			images[0].height = height;
			
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