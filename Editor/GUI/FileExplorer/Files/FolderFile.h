#pragma once

#include "Editor/GUI/FileExplorer/File.h"

namespace Plaza {
	namespace Editor {
		class FolderFile : public File {
		  public:
			FolderFile(std::string name, std::string directory, std::string extension)
				: File(name, directory, extension) {}

			void DoubleClick();
			void Delete();
			void Rename(std::string oldPath, std::string newPath);
			void Move(std::string oldPath, std::string newPath);
			void Copy();
			void Paste();
		};
	} // namespace Editor
} // namespace Plaza
