#pragma once
#include "Engine/Core/PreCompiledHeaders.h"
namespace Plaza::Editor::Utils {
	static bool ComponentInspectorHeader(Component* component, std::string name,
										 ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen) {
		ImGui::PushID(("ComponentInspectorHeader" + name).c_str());
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
		bool header;
		bool deleted = false;
		if (component) {
			ImGui::SetNextItemOpen(true);
			ImVec2 old = ImGui::GetCursorPos();
			float arrowSize = old.x + 30.0f;
			ImGui::SetCursorPos(ImVec2(ImGui::CalcTextSize(name.c_str()).x + arrowSize, ImGui::GetCursorPos().y));
			if (ImGui::Button("Remove Component")) {
				// FIX: Scene::GetActiveScene()->entities.find(component->mUuid)->second.RemoveComponent(component);
				deleted = true;
			}
			ImGui::SameLine();
			ImGui::Checkbox("Enabled", &component->mEnabled);
			ImGui::SameLine();
			ImGui::SetCursorPos(old);
			header = ImGui::CollapsingHeader(name.c_str(), ImGuiTreeNodeFlags_DefaultOpen);
			ImGui::SameLine();
			ImGui::Button("Remove Component");
			ImGui::SameLine();
			ImGui::Checkbox("Enabled", &component->mEnabled);
			if (deleted)
				header = false;
		}
		else
			header = ImGui::CollapsingHeader(name.c_str(), flags);

		ImGui::PopID();
		ImGui::PopStyleVar();
		return header;
	}
} // namespace Plaza::Editor::Utils

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
