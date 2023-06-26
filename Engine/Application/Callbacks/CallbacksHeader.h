#pragma once
//#include "Mouse.h"
#include "Engine/Application/Application.h"
namespace Engine {
    class Application::Callbacks {
    public:
		static bool rightClickPressed;
		static bool mouseFirstCallback;
		static bool firstMouse;

		static float lastX;
		static float lastY;

        static void mouseCallback(GLFWwindow* window, double xposIn, double yposIn);
		static void dropCallback(GLFWwindow* window, int count, const char** paths);
		static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
		static void processInput(GLFWwindow* window);
		static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
		static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    };
}