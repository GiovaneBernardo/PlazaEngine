#pragma once
#include "Engine/Core/Debugging/Log.h"
#include "Engine/Core/PreCompiledHeaders.h"
#include "Editor/GUI/GuiWindow.h"

#include "Console.h"
#include "Editor/GUI/Utils/DataVisualizer.h"
#include "GLFW/glfw3.h"
#include "imgui.h"
#include <cmath>
#include <ctime>

namespace Plaza {
	namespace Editor {
		void Console::Init() {}

		void Console::AddMessage(const std::string& messageContent, int severity) {
			mMessages.push_back(ConsoleMessage{messageContent, severity});
		}

		void Console::Update(Scene* scene) {
			ImGuiWindowFlags windowFlags =
				ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoNavFocus;
			if (ImGui::Begin("Console", &mExpanded, windowFlags)) {
				ImGui::Checkbox("Update Indirect Instances", &mTemporaryVariables.updateIndirectInstances);
			}

			for (const LogMessage& message : Plaza::Log::GetVectorSink()->GetLogs()) {
				ImGui::Text(message.mMessage.c_str());
				ImGui::SameLine();
				ImGui::Text(ctime(&message.mTime));
			}

			if (ImGui::InputText("##ConsoleInput", mConsoleInput, sizeof(mConsoleInput),
								 ImGuiInputTextFlags_EnterReturnsTrue)) {
				this->AddMessage(mConsoleInput, 0);
				mConsoleInput[0] = '\0';
			}
			ImGui::End();
		}
	} // namespace Editor
} // namespace Plaza
