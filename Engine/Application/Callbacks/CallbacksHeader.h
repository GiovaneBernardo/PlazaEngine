#pragma once
#include "Editor/GUI/GuiWindow.h"
namespace Plaza {
	struct CallbackFunction {
		std::function<void(int key, int scancode, int action, int mods)> function;
		Editor::GuiLayer layerToExecute;
		Editor::GuiState layerStateToExecute = Editor::GuiState::FOCUSED;
	};
	class Callbacks {
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
		static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

		static void AddFunctionToKeyCallback(CallbackFunction callbackFunction) {
			sOnKeyPressFunctions.push_back(callbackFunction);
		}

		static void AddFunctionToMouseCallback(CallbackFunction callbackFunction) {
			sOnMouseFunctions.push_back(callbackFunction);
		}

	  private:
		static inline std::vector<CallbackFunction> sOnKeyPressFunctions = std::vector<CallbackFunction>();
		static inline std::vector<CallbackFunction> sOnKeyEventFunctions = std::vector<CallbackFunction>();
		static inline std::vector<CallbackFunction> sOnMouseFunctions = std::vector<CallbackFunction>();
	};
} // namespace Plaza
