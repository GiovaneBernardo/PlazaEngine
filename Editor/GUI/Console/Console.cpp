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

			if (ImGui::BeginTable("ConsoleTable", 2,
					ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_ScrollY))
			{
				ImGui::TableSetupColumn("Message", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_WidthFixed, 120.0f);

				int index = 0;
				for (const LogMessage& message : Plaza::Log::GetVectorSink()->GetLogs()) {
					ImGui::TableNextRow();

					// Message column
					ImGui::TableSetColumnIndex(0);

					ImVec4 color;
					switch (message.mLevel) {
						case spdlog::level::trace:    color = ImVec4(0.6f, 0.6f, 0.6f, 1.0f); break;
						case spdlog::level::debug:    color = ImVec4(0.4f, 0.8f, 1.0f, 1.0f); break;
						case spdlog::level::info:     color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); break;
						case spdlog::level::warn:     color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); break;
						case spdlog::level::err:      color = ImVec4(1.0f, 0.2f, 0.2f, 1.0f); break;
						case spdlog::level::critical: color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); break;
						default:                      color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); break;
					}

					ImGui::TextColored(color, "%s", message.mMessage.c_str());

					// Time column
					ImGui::TableSetColumnIndex(1);
					ImGui::TextDisabled("%s", ctime(&message.mTime));

					index++;
				}

				ImGui::EndTable();
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
