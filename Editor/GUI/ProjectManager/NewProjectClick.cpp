#include "Engine/Core/PreCompiledHeaders.h"
#include "ProjectManager.h"

#include "Engine/Application/FileDialog/FileDialog.h"

#include "Editor/Settings/ProjectGenerator.h"
#include "Editor/SessionCache/Cache.h"
namespace Plaza {
	namespace Editor {
		void ProjectManagerGui::NewProjectClick() {
			std::string filePath = FileDialog::OpenFolderDialog();
			if (filePath != "") {
				Application::Get()->activeProject = std::make_unique<Project>();
				Application::Get()->activeProject->mAssetName;
				Application::Get()->activeProject->mAssetPath = filePath;
				this->currentContent = new NewProjectContent();
			}
		}
	} // namespace Editor
} // namespace Plaza
