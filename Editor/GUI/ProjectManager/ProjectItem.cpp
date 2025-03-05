#include "Engine/Core/PreCompiledHeaders.h"
#include "ProjectItem.h"
namespace Plaza {
	namespace Editor {
		void ProjectManagerGui::ProjectItem::Setup() {
			ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
										   ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoFocusOnAppearing;
			windowFlags |= ImGuiWindowFlags_NoScrollbar;

			if (ImGui::BeginChild(ImGui::GetID(name.c_str()), ImVec2(500, 100), false, windowFlags)) {
				ImGui::Text(name.c_str());
				ImGui::Text(path.c_str());
			}
			ImGui::EndChild();

			if (ImGui::IsItemClicked()) {
				OnClick();
			}
		}

		void ProjectManagerGui::ProjectItem::OnClick() { std::cout << "Clicked" << std::endl; }
	} // namespace Editor
} // namespace Plaza
