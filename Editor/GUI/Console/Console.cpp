#pragma once
#include "Engine/Core/PreCompiledHeaders.h"
#include "Editor/GUI/GuiWindow.h"

#include "Console.h"
#include "Editor/GUI/Utils/DataVisualizer.h"

namespace Plaza {
	namespace Editor {
		void Console::Init() {}

		void Console::Update(Scene* scene) {
			ImGuiWindowFlags windowFlags =
				ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoNavFocus;
			if (ImGui::Begin("Console", &mExpanded, windowFlags)) {
				ImGui::Checkbox("Update Indirect Instances", &mTemporaryVariables.updateIndirectInstances);
			}
			ImGui::End();
		}
	} // namespace Editor
} // namespace Plaza
