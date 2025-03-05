#pragma once
#include "Editor/GUI/FileExplorer/File.h"
#include "Editor/GUI/guiMain.h"

namespace Plaza {
	namespace Editor {
		class Gui::FileExplorer {
		  public:
			static inline int currentColumn = -1;
			static inline float lastY = 0.0f;
			static inline bool breakFilesLoop = false;
			static std::vector<std::unique_ptr<File>> files;
			static std::string currentDirectory;
			static void Init();
			static void UpdateGui();
			static void DrawFile(File* file);
			/// <summary>
			/// Read the project directory to get the present files
			/// </summary>
			static void UpdateContent(std::string folderPath);

		  private:
			static void OrderFilesByAlphabete(std::vector<std::unique_ptr<File>>& files);
			static void OrderFilesByCreationDate(std::vector<std::unique_ptr<File>>& files);
		};
	} // namespace Editor
} // namespace Plaza
