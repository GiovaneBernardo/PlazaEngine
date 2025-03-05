#pragma once
#include "Engine/Core/Time.h"
namespace Plaza::Editor {
	class FpsCounter {
	  public:
		FpsCounter() {}

		void Update() {
			PLAZA_PROFILE_SECTION("Update FPS Counter");
			ImGuiWindowFlags windowflags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
										   ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing;
			windowflags |= ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking;
			bool showFPSCounter = true;
			if (ImGui::Begin("FPS Overlay", &showFPSCounter, windowflags)) {
				ImGui::SetWindowPos(ImVec2(Application::Get()->appSizes->sceneStart.x +
											   Application::Get()->appSizes->sceneSize.x +
											   (ImGui::GetWindowSize().x / 2),
										   Application::Get()->appSizes->sceneStart.y + ImGui::GetWindowPos().y),
									ImGuiCond_Once);
				ImGui::Text(("MS/Frame:" + std::to_string(Time::msPerFrame)).c_str());
				ImGui::Text(("FPS:" + std::to_string(Time::fps)).c_str());
				ImGui::Text(("Scene Size X:" + std::to_string(Application::Get()->appSizes->sceneSize.x)).c_str());
				ImGui::Text(("Scene Size Y:" + std::to_string(Application::Get()->appSizes->sceneSize.y)).c_str());
				ImGui::Text(("Scene Start X:" + std::to_string(Application::Get()->appSizes->sceneStart.x)).c_str());
				ImGui::Text(("Scene Start Y:" + std::to_string(Application::Get()->appSizes->sceneStart.y)).c_str());
				ImGui::Text(("Draw Calls:" + std::to_string(Time::drawCalls)).c_str());
				ImGui::Text(("Add Instance Calls:" + std::to_string(Time::addInstanceCalls)).c_str());
				ImGui::Text(("Unique Triangles: " + std::to_string(Time::mUniqueTriangles)).c_str());
				ImGui::Text(("Total Triangles: " + std::to_string(Time::mTotalTriangles)).c_str());

				ImGui::End();
			}
		}
	};
} // namespace Plaza::Editor
