#pragma once
#include <iostream>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui/imgui.h>
#include "Engine/Vendor/imgui/ImGuizmo.h"

#include "Engine/Application/Application.h"
#include "Engine/Application/EditorCamera.h"

using Engine::Camera;

namespace Engine::Editor {
	class Overlay {
	public:
		enum ButtonsType {
			WORLD,
			LOCAL,
			POSITION,
			ROTATION,
			SCALE
		};

		static ImGuizmo::MODE activeMode;
		static ImGuizmo::OPERATION activeOperation;
		static void beginTransformOverlay(Camera camera);

	private:
		class Button {
		public:
			std::string name;
			ImGuizmo::OPERATION activeOperation;
			ImGuizmo::MODE activeMode;
		};

		//
		// Creates a button that changes the background depending on the active mode or operation
		// 
		//
		static void createButton(Button* button, ImGuizmo::MODE& activeMode, ImGuizmo::OPERATION& activeOperation) {
			bool popStyle = false;
			bool buttonModeSameActive = (button->activeMode == activeMode && button->activeMode != ImGuizmo::MODE(-1)); // the button mode is the same as the active mode, and its not empty
			bool buttonOperationSameActive = (button->activeOperation == activeOperation && button->activeOperation != ImGuizmo::OPERATION(-1)); // the button operation is the same as the active mode, and its not empty
			// Change the button background
			if (buttonModeSameActive || buttonOperationSameActive) {
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.4f, 1.0f));
				popStyle = true;
			}

			// Update the active mode or operation
			if (ImGui::Button(button->name.c_str())) {
				if (button->activeMode != activeMode && button->activeMode != ImGuizmo::MODE(-1)) activeMode = button->activeMode;
				if (button->activeOperation != activeOperation && button->activeOperation != ImGuizmo::OPERATION(-1)) activeOperation = button->activeOperation;
			}

			if (popStyle) ImGui::PopStyleColor();
		}
	};
}