#pragma once
#include "FolderFile.h"
#include "Editor/GUI/FileExplorer/FileExplorer.h"

namespace Plaza {
	namespace Editor {
		void FolderFile::DoubleClick() {
			Editor::Gui::FileExplorer::currentDirectory = this->directory;
			Gui::FileExplorer::UpdateContent(Gui::FileExplorer::currentDirectory);
			Editor::selectedFiles.clear();
			Gui::FileExplorer::breakFilesLoop = true;
		}

		void FolderFile::Delete() {}

		void FolderFile::Rename(std::string oldPath, std::string newPath) {}

		void FolderFile::Move(std::string oldPath, std::string newPath) {}

		void FolderFile::Copy() {}

		void FolderFile::Paste() {}
	} // namespace Editor
} // namespace Plaza
