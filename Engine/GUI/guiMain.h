#pragma once

#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

//#include "Engine/Application/EditorCamera.h"
#include "Engine/Components/Core/GameObject.h"
//#include "Engine/Application/Application.h"

namespace ImGuizmo {
	static bool IsDrawing;
}

namespace Engine {
	class Camera;
}
namespace Editor {
	namespace Gui {
		using namespace Engine;
		static void setupDockspace(GLFWwindow* window, int gameFrameBuffer, Camera& camera);
		extern void changeSelectedGameObject(GameObject* newSelectedGameObject);
		extern void Init(GLFWwindow* window);
		extern void Delete();
		extern void Update();
		extern void NewFrame();

		static void beginScene(int gameFrameBuffer, Camera& camera);
		static void beginHierarchyView(int gameFrameBuffer);
		static void beginInspector(int gameFrameBuffer, Camera camera);

		static void UpdateSizes();
	}
}

namespace ImGui {
	inline bool Compare(ImVec2 firstVec, ImVec2 secondVec) {
		return firstVec.x == secondVec.x && firstVec.y == secondVec.y;
	}
	// Transforms glm::vec2 to ImVec2
	inline ImVec2 imVec2(glm::vec2 vec) {
		return ImVec2(vec.x, vec.y);
	}
	// Transforms ImVec2 to glm::vec2
	inline glm::vec2 glmVec2(ImVec2 imguiVec) {
		return glm::vec2(imguiVec.x, imguiVec.y);
	}
}
