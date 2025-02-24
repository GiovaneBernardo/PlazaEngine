#include "Engine/Core/PreCompiledHeaders.h"
#include "ProjectManager.h"

#include "Engine/Application/FileDialog/FileDialog.h"
#include "Editor/GUI/FileExplorer/FileExplorer.h"
#include "Editor/Filewatcher.h"
#include "Editor/ScriptManager/ScriptManager.h"
#include "Editor/DefaultAssets/Models/DefaultModels.h"
#include "Editor/Project.h"
#include "Editor/SessionCache/Cache.h"
namespace Plaza {
	namespace Editor {
		namespace fs = std::filesystem;
		void ProjectManagerGui::OpenProjectClick() {
			std::string filePath = FileDialog::OpenFileDialog(Standards::projectExtName.c_str());
			// If user didnt canceled, loop through all items found on the directory path and check for the first that haves the project extension. 
			if (filePath != "") {
				Project::Load(filePath);
				Cache::Serialize(Application::Get()->enginePathAppData + "/cache" + Standards::editorCacheExtName);
			}

		}



	}
}
