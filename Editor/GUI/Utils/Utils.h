#pragma once
#include "Engine/Core/PreCompiledHeaders.h"
namespace Plaza::Editor::Utils {
	static bool ComponentInspectorHeader(Component* component, std::string name, ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen) {
		ImGui::PushID(("ComponentInspectorHeader" + name).c_str());
		bool header;
		if (component) {
			ImGui::SetNextItemOpen(true);
			ImVec2 old = ImGui::GetCursorPos();
			float arrowSize = old.x + 30.0f;
			ImGui::SetCursorPos(ImVec2(ImGui::CalcTextSize("Collider").x + arrowSize, ImGui::GetCursorPos().y));
			if (ImGui::Button("Remove Component")) {
				component->GetGameObject()->RemoveComponent(component);
			}
			ImGui::SameLine();
			ImGui::SetCursorPos(old);
			header = ImGui::CollapsingHeader("Collider", ImGuiTreeNodeFlags_DefaultOpen);
			ImGui::SameLine();
			ImGui::Button("Remove Component");
		}
		else
			header = ImGui::CollapsingHeader(name.c_str(), flags);

		ImGui::PopID();
		return header;
	}
}

/*
			ImGui::SetNextItemOpen(true);
			ImVec2 old = ImGui::GetCursorPos();
			float arrowSize = old.x + 30.0f;
			ImGui::SetCursorPos(ImVec2(ImGui::CalcTextSize("Collider").x + arrowSize, ImGui::GetCursorPos().y));
			if (ImGui::Button("Remove Component")) {
				collider->GetGameObject()->RemoveComponent<Collider>();
			}
			ImGui::SameLine();
			ImGui::SetCursorPos(old);
			bool header = ImGui::CollapsingHeader("Collider", ImGuiTreeNodeFlags_DefaultOpen);
			ImGui::SameLine();
			ImGui::Button("Remove Component");
*/