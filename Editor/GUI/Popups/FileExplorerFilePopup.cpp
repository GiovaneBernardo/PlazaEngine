#include "Engine/Core/PreCompiledHeaders.h"
#include "FileExplorerFilePopup.h"
#include "Editor/GUI/Utils/Filesystem.h"
#include "Engine/Core/Script.h"

#include "Editor/ScriptManager/ScriptManager.h"
#include "Editor/GUI/FileExplorer/File.h"
#include "FileExplorerPopup.h"
#include "Engine/Core/FilesManager.h"

namespace Plaza::Editor {
	void Popup::FileExplorerFilePopup::UpdateContent(File* file) {
		if (!Editor::selectedFiles.contains(file->name)) {
			Editor::selectedFiles.clear();
			Editor::selectedFiles.emplace(file->directory, new File(*file));
		}

		if (ImGui::MenuItem("Open")) {
			Plaza::FilesManager::OpenFile(std::string("\"" + file->directory + "\"").c_str());
		}

		if (ImGui::MenuItem("Open in file explorer")) {
			FilesManager::OpenFileParentFolder(file->directory);
		}

		if (ImGui::MenuItem("Rename")) {
			Editor::File::changingName = Editor::selectedFiles.begin()->second->name;
			Editor::File::firstFocus = true;
		}

		if (ImGui::MenuItem("Delete")) {
			for (const auto& [key, value] : Editor::selectedFiles) {
				Utils::Filesystem::DeleteFileF(value->directory);
			}
		}
	}
	void Popup::FileExplorerFilePopup::Update(File* file) {
		if (ImGui::BeginPopupContextWindow("FileExplorerFilePopup"))
		{
			UpdateContent(file);
			//FileExplorerPopup::UpdateContent();
			ImGui::EndPopup();
		}
	}
}