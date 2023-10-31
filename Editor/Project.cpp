#include "Engine/Core/PreCompiledHeaders.h"
#include "Project.h"

#include "Engine/Application/FileDialog/FileDialog.h"
#include "Engine/Application/Serializer/SceneSerializer.h"
#include "Engine/Application/Serializer/ProjectSerializer.h"
#include "Editor/GUI/FileExplorer/FileExplorer.h"
#include "Engine/Application/Serializer/ScriptManagerSerializer.h"
#include "Engine/Core/Scripting/Mono.h"
#include "Editor/Filewatcher.h"
#include "Editor/ScriptManager/ScriptManager.h"
#include "Editor/DefaultAssets/Models/DefaultModels.h"
#include "Engine/Application/Serializer/FileSerializer/FileSerializer.h"
#include "Engine/Core/ModelLoader/ModelLoader.h"

namespace Plaza::Editor {
	void Project::Load(const std::string filePath) {
		filesystem::path projectFile(filePath);

		std::string fileName = projectFile.filename().string();
		std::string extension = projectFile.extension().string();
		// Check if its extension is the project extension
		if (extension == Standards::projectExtName) {
			std::cout << filePath << std::endl;
			Application->projectPath = projectFile.parent_path().string();


			Application->activeProject = new Project();
			Application->activeProject->name = fileName;
			Application->activeProject->directory = projectFile.parent_path().string();
			Application->activeProject->scriptsConfigFilePath = Application->activeProject->directory + "\\Scripts" + Standards::scriptConfigExtName;
			//Application->activeProject->scripts = ScriptManagerSerializer::DeSerialize(Application->activeProject->scriptsConfigFilePath);
			Filewatcher::Start(Application->activeProject->directory);

			Application->runEngine = true;
			Application->runProjectManagerGui = false;
			std::cout << "Mid \n";
			//this->currentContent = new NewProjectContent();

#ifndef GAME_REL
			Gui::FileExplorer::currentDirectory = Application->activeProject->directory;
			Gui::FileExplorer::UpdateContent(Gui::FileExplorer::currentDirectory);
#endif // !GAME_REL

			Application->activeProject->directory = projectFile.parent_path().string();


			//free(Application->editorScene);
			//free(Application->runtimeScene);

			Application->editorScene = new Scene();
			Application->runtimeScene = new Scene();
			Application->editorScene->mainSceneEntity = new Entity("Scene");
			Application->activeScene = Application->editorScene;

			std::cout << "Mono \n";
			Mono::Init();

			/* Iterate over all files and subfolders of the project folder*/
			for (const auto& entry : filesystem::recursive_directory_iterator(Application->activeProject->directory)) {
				if (entry.is_regular_file() && entry.path().extension() == ".cs") {
					Application->activeProject->scripts.emplace(entry.path().string(), Script());
				}

				if (entry.is_regular_file() && entry.path().extension() == Standards::materialExtName) {
					std::shared_ptr<Material> deserializedMaterial = std::shared_ptr<Material>(MaterialFileSerializer::DeSerialize(entry.path().string()));
					if (deserializedMaterial->uuid) {
						deserializedMaterial->LoadTextures(entry.path().parent_path().string());
						Application->activeScene->AddMaterial(deserializedMaterial);
						//Application->activeScene->materials.emplace(deserializedMaterial->uuid, deserializedMaterial);
					}
				}
			}

			std::cout << "Deserializing \n";
			ProjectSerializer::DeSerialize(filePath);
			std::cout << "Finished Deserializing \n";


			return;
		}
		std::cout << "Project has not been found!" << "\n";
	}
}