#pragma once
#include "Editor/GUI/guiMain.h"
#include "Editor/GUI/FileExplorer/File.h"
namespace Plaza::Editor {
	class RenamedFileManager {
	  public:
		static void Run(std::string oldPath, std::string newPath);
	};
} // namespace Plaza::Editor
