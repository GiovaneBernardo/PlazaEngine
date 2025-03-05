#include "Engine/Core/PreCompiledHeaders.h"
#include "AssetsImporterWindow.h"
#include "Engine/Application/Callbacks/CallbacksHeader.h"
#include "Editor/GUI/Utils/DataVisualizer.h"

namespace Plaza {
	namespace Editor {
		void AssetsImporterWindow::Init() {
			auto onKeyPressLambda = [this](int key, int scancode, int action, int mods) {
				this->OnKeyPress(key, scancode, action, mods);
			};
			Callbacks::AddFunctionToKeyCallback({onKeyPressLambda, GuiLayer::ASSETS_IMPORTER});
		}
		void AssetsImporterWindow::Update(Scene* scene) {
			if (!this->IsOpen())
				return;

			ImGui::OpenPopup("Assets Importer");

			if (ImGui::BeginPopupModal("Assets Importer", NULL, ImGuiWindowFlags_MenuBar)) {
				this->ModelImporterMenu();

				if (ImGui::Button("Import")) {
					this->SetOpen(false);
					ImGui::CloseCurrentPopup();
					Application::Get()->mThreadsManager->mFrameStartThread->AddToQueue(
						[&]() { AssetsImporter::ImportAsset(mFileToImport, 0, settings); });
				}
				if (ImGui::Button("Cancel")) {
					this->SetOpen(false);
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}

		void AssetsImporterWindow::ModelImporterMenu() {
			Utils::GuiTable table{};
			if (table.tableName.empty())
				table.tableName = table.tableUuid;

			ImGui::BeginTable("Importer Menu", 2, table.flags);
			Utils::AddTableCheckbox("Import Model", &settings.mImportModel);
			Utils::AddTableCheckbox("Import Textures", &settings.mImportTextures);
			Utils::AddTableCheckbox("Import Materials", &settings.mImportMaterials);
			Utils::AddTableCheckbox("Import Animations", &settings.mImportAnimations);
			Utils::AddTableVector2("Flip Textures", &settings.mFlipTextures);
			ImGui::EndTable();
			// Utils::AddTableValue(table, "Import Model", )
			// AddTableValue(table, "Import Model", [&, table]() {
			//	ImGui::Checkbox("##", &mSettings.importModel);
			//	});
			// AddTableValue(table, "Import Animations", [&, table]() {
			//	ImGui::Checkbox("##", &mSettings.importAnimations);
			//	});
			// Utils::ShowTable(table);
			// ImGui::EndTable();
			// ImGui::TableNextRow();
			// ImGui::TableNextColumn();
			// ImGui::Text("Import Model");
			// ImGui::Text("Import Animations");
			// ImGui::TableNextColumn();
			// ImGui::Checkbox("", &mSettings.importModel);
			// ImGui::Checkbox("", &mSettings.importAnimations);
		}

		void AssetsImporterWindow::OnKeyPress(int key, int scancode, int action, int mods) {
			if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
				this->SetOpen(false);
			}
		}
	} // namespace Editor
} // namespace Plaza
