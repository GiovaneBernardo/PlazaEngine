#include "Engine/Core/PreCompiledHeaders.h"
#include "FileExplorer.h"

namespace Engine {
	namespace Editor {
		std::string Gui::FileExplorer::currentDirectory = "";
		void Gui::FileExplorer::UpdateGui() {
			ApplicationSizes& appSizes = *Application->appSizes;
			ApplicationSizes& lastAppSizes = *Application->lastAppSizes;
			GameObject* selectedGameObject = Editor::selectedGameObject;

			// Set the window to be the content size + header size
			ImGuiWindowFlags  sceneWindowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove;
			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

			ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoNavFocus;
			//windowFlags |= ImGuiWindowFlags_NoScrollbar;
			if (ImGui::Begin("File Explorer", &Gui::isFileExplorerOpen, windowFlags)) {
				if (ImGui::IsWindowFocused())
					Application->focusedMenu = "File Explorer";
				if (ImGui::IsWindowHovered())
					Application->hoveredMenu = "File Explorer";
				Icon::currentPos = ImVec2(-1.0f, 1.0f);

				ImGui::BeginGroup();

				// Back Button
				std::string currentDirectory = Gui::FileExplorer::currentDirectory;
				const std::string& currentDirectoryPath = filesystem::path{ currentDirectory }.string();
				File backFile = File();
				backFile.directory = currentDirectory + "\\asd.back";
				backFile.extension = ".back";
				backFile.name = ".back";
				Editor::Icon::Update(currentDirectory + "\\asd.back", ".back", ".back", backFile);
				// Back Button Click
				if (ImGui::IsItemClicked() && filesystem::path{ currentDirectory }.parent_path().string().starts_with(Application->activeProject->directory)) {
					Editor::Gui::FileExplorer::currentDirectory = filesystem::path{ currentDirectory }.parent_path().string();
					Gui::FileExplorer::UpdateContent(Gui::FileExplorer::currentDirectory);
				}

				// Create all the icons
				for (File file : files) {
					if (file.name != "")
						Icon::Update(file.directory, file.extension, file.name, file);
				}
				ImGui::EndGroup();
			}

			ImGui::End();
			ImGui::PopStyleColor();


		}
		std::vector<File> Gui::FileExplorer::files = std::vector<File>();
		void Gui::FileExplorer::UpdateContent(std::string folderPath) {
			files.clear();
			namespace fs = std::filesystem;
			for (const auto& entry : fs::directory_iterator(folderPath)) {
				std::string filename = entry.path().filename().string();
				std::string extension = entry.path().extension().string();
				std::string deb = entry.path().stem().string();
				if (entry.path().stem().string() == filename)
					extension = filename;
				// Check if its a folder
				if (fs::is_directory(entry.path())) {
					File newFile = File();
					newFile.directory = entry.path().string();
					newFile.name = filename;
					newFile.extension = extension;
					files.push_back(newFile);
				}
				else {
					File newFile = File();
					newFile.directory = entry.path().string();
					newFile.name = filename;
					newFile.extension = extension;
					files.push_back(newFile);
				}
			}
		}
	}
}