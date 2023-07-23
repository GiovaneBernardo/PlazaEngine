#include "Engine/Core/PreCompiledHeaders.h"
#include "MenuBar.h"
#include "Engine/Application/FileDialog/FileDialog.h"
#include "Engine/Application/Serializer/SceneSerializer.h"
namespace Engine {
	namespace Editor {
		void Gui::MainMenuBar::Begin() {
			// MenuBar / TitleBar
			if (ImGui::BeginMainMenuBar()) {
				if (ImGui::BeginMenu("File")) {
					if (ImGui::Button("Save")) {
						Serializer::Serialize(FileDialog::SaveFileDialog("Engine (*.engine)"));
					}
					if(ImGui::Button("Open")){
						Serializer::DeSerialize(FileDialog::OpenFileDialog("Engine (*.engine)"));
					}
					ImGui::EndMenu();
				}




				// Create an invisible margin to align the buttons to the right
				float spacingX = ImGui::GetWindowSize().x - ImGui::GetCursorPosX() - ImGui::GetStyle().ItemSpacing.x * 3;
				spacingX -= 300;
				ImGui::Dummy(ImVec2(spacingX, 0));

				// Create the right corner buttons
				ImGui::SameLine();
				if (ImGui::Button("Minimize")) {
					glfwIconifyWindow(Application->Window->glfwWindow);
				}
				ImGui::SameLine();
				if (ImGui::Button("Maximize")) {
					glfwMaximizeWindow(Application->Window->glfwWindow);
				}
				ImGui::SameLine();
				if (ImGui::Button("Close")) {
					glfwSetWindowShouldClose(Application->Window->glfwWindow, true);
				}

				ImGui::EndMainMenuBar();

			}
		}
	}
}