#include "Engine/Core/PreCompiledHeaders.h"
#include "Editor/GUI/GuiWindow.h"
#include "SearchContext.h"
#include "Engine/Components/Rendering/Material.h"

namespace Plaza::Editor {
	void SearchContext::Init() {}

	void SearchContext::Update(Scene* scene) {
		bool wasOpen = mIsOpen;
		if (ImGui::Begin(mName.c_str(), &mIsOpen)) {
			this->UpdateHovered();
			this->UpdateFocused();

			ImGui::Text("Search: ");
			ImGui::SameLine();

			ImGui::InputText("##SearchInput", mSearchBuffer, sizeof(mSearchBuffer));
			std::string searchText(mSearchBuffer);

			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
				this->SetOpen(false);

			ImGui::BeginChild("SearchChild1");

			for (const auto& [key, value] : AssetsManager::mMaterials) {
				if (value->mAssetName.find(searchText) != std::string::npos) {
					ImGui::BeginGroup();
					ImGui::Text(value->mAssetName.c_str());
					ImGui::SameLine();
					if (ImGui::Button(std::string("Set Material##" + std::to_string(value->mAssetUuid)).c_str())) {
						this->ExecuteCallback(value->mAssetUuid);
					}

					ImGui::EndGroup();
				}
			}

			ImGui::EndChild();
		}
		// if (wasOpen && !mIsOpen) {
		//	ExecuteCallback();
		// }
		ImGui::End();
	}
	// void SearchContext::SetValues(std::vector<std::any&>& values) {
	void SearchContext::SetValues() {
		// mValues = values;
	}
} // namespace Plaza::Editor
