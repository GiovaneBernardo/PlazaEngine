#include "Engine/Core/PreCompiledHeaders.h"
#include "Project.h"

#include "Engine/Core/Scripting/Script.h"
#include "Engine/Core/Scripting/Script.h"
#include "Engine/Application/FileDialog/FileDialog.h"
#include "Editor/GUI/FileExplorer/FileExplorer.h"
#include "Editor/Filewatcher.h"
#include "Editor/ScriptManager/ScriptManager.h"
#include "Editor/DefaultAssets/Models/DefaultModels.h"
#include "Engine/Core/AssetsManager/Loader/AssetsLoader.h"
#include "Engine/Core/AssetsManager/AssetsReader.h"
#include "Engine/Core/Scene.h"
#include "Engine/Core/AssetsManager/Serializer/AssetsSerializer.h"

namespace Plaza::Editor {
	void Project::Load(const std::string filePath) {
		filesystem::path projectFile(filePath);

		std::string fileName = projectFile.filename().string();
		std::string extension = projectFile.extension().string();
		// Check if its extension is the project extension
		bool projectNotFound = extension != Standards::projectExtName || !std::filesystem::exists(projectFile);
		if (projectNotFound) {
			Application::Get()->runEngine = false;
			PL_CORE_WARN("{0}:{1}: Project has not been found!", __FILE__, __LINE__);
			return;
		}

		Application::Get()->projectPath = projectFile.parent_path().string();

		Application::Get()->activeProject = std::make_unique<Project>(*AssetsSerializer::DeSerializeFile<Project>(filePath, Application::Get()->mSettings.mProjectSerializationMode));
		Application::Get()->activeProject->mAssetPath = filePath;

#ifdef EDITOR_MODE
		Filewatcher::Start(Application::Get()->activeProject->mAssetPath.parent_path().string());
#endif

		Application::Get()->runEngine = true;
		Application::Get()->runProjectManagerGui = false;
		std::cout << "Mid \n";

#ifdef EDITOR_MODE
		Gui::FileExplorer::currentDirectory = Application::Get()->activeProject->mAssetPath.parent_path().string();
		Gui::FileExplorer::UpdateContent(Gui::FileExplorer::currentDirectory);
#endif // EDITOR_MODE

		std::cout << "Read Game Folder \n";
		Scene::ClearEditorScene();
		Scene::SetActiveScene(Scene::GetEditorScene());
		std::map<std::string, Script> scripts = std::map<std::string, Script>();
		/* Iterate over all files and subfolders of the project folder to load assets */
		std::string path = Application::Get()->activeProject->mAssetPath.parent_path().string();
		Profiler::NewProfiler("TextureLoading");
		Profiler::NewProfiler("ReadGameAssets");
		/*
				if (asset->GetExtension() == Standards::metadataExtName)
			AssetsLoader::LoadMetadata(asset);
		else
			AssetsManager::NewAsset<Asset>(asset->mAssetUuid, asset->mAssetPath.string());

		if (asset->GetExtension() == Standards::modelExtName)
			AssetsLoader::LoadPrefab(asset);
		else if (asset->GetExtension() == Standards::materialExtName)
			AssetsLoader::LoadMaterial(asset, Application::Get()->mSettings.mCommonSerializationMode);
		else if (asset->GetExtension() == Standards::animationExtName)
			AssetsLoader::LoadAnimation(asset, Application::Get()->mSettings.mAnimationSerializationMode);
		*/
		SectionProfiler profiler;
		AssetsManager::ReadFolderContent(path, true);
		profiler.Stop();
		Profiler::GetProfiler("ReadGameAssets")->AddDuration(profiler.GetDuration());
		//for (auto entry = filesystem::recursive_directory_iterator(Application::Get()->activeProject->mAssetPath.parent_path(), filesystem::directory_options::skip_permission_denied); entry != filesystem::end(entry); ++entry) {
		//	if (entry->is_directory() && entry->path().filename().string().ends_with(".ignore")) {
		//		entry.disable_recursion_pending();
		//	}
		//
		//	AssetsReader::ReadAssetAtPath(entry->path());
		//}

		PL_CORE_INFO("Load materials textures");
		uint64_t defaultMaterialUuid = AssetsManager::GetDefaultMaterial()->mAssetUuid;
		for (auto& [key, value] : AssetsManager::mMaterials) {
			//Application::Get()->mRenderer->LoadTexture(AssetsManager::GetAssetOrImport(FileDialog::OpenFileDialog(".jpeg"))->mPath.string())
			if (key == defaultMaterialUuid || key == 0)
				continue;
			value->GetDeserializedTextures();
			if (value->diffuse->mAssetUuid)
				value->diffuse = std::shared_ptr<Texture>(AssetsManager::GetTexture(value->diffuse->mAssetUuid));
			if (value->normal->mAssetUuid)
				value->normal = std::shared_ptr<Texture>(AssetsManager::GetTexture(value->normal->mAssetUuid));
			if (value->roughness->mAssetUuid)
				value->roughness = std::shared_ptr<Texture>(AssetsManager::GetTexture(value->roughness->mAssetUuid));
			if (value->metalness->mAssetUuid)
				value->metalness = std::shared_ptr<Texture>(AssetsManager::GetTexture(value->metalness->mAssetUuid));
		}

		std::cout << "Deserializing \n";
		//ProjectSerializer::DeSerialize(filePath);

		DefaultModels::Init();
		PL_CORE_INFO(Application::Get()->activeProject->mLastSceneUuid);
		if (Application::Get()->activeProject->mLastSceneUuid != 0 && AssetsManager::GetAsset(Application::Get()->activeProject->mLastSceneUuid)) {
			const std::string sceneFilePath = AssetsManager::GetAsset(Application::Get()->activeProject->mLastSceneUuid)->mAssetPath.string();//Application::Get()->projectPath + "/" + AssetsManager::lastActiveScenePath;
			bool sceneFileExists = std::filesystem::exists(sceneFilePath);
			if (sceneFileExists) {
				Scene* scene = Scene::GetEditorScene();
				Scene::SetEditorScene(AssetsSerializer::DeSerializeFile<Scene>(sceneFilePath, Application::Get()->mSettings.mSceneSerializationMode));
				Scene::SetActiveScene(Scene::GetEditorScene());
				Scene::GetActiveScene()->RecalculateAddedComponents();
			}
			//if (sceneFileExists)
			//	Serializer::DeSerialize(sceneFilePath, true);
		}
		else {
			Scene::ClearEditorScene();
			Scene::SetActiveScene(Scene::GetEditorScene());
			Scene::GetEditorScene()->InitMainEntity();
		}
		std::cout << "Finished Deserializing \n";
	}

	void Project::PasteCmakeFile(const std::filesystem::path& directory) {
		std::filesystem::path cmakeFilePath = directory / "CMakeLists.txt";
		FilesManager::CreateFileWithData(cmakeFilePath, AssetsManager::GetEmbedResource<"Editor/DefaultAssets/NewProject/CMakeLists.txt">());
		std::filesystem::path cmakePresetsFilePath = directory / "CMakePresets.json";
		FilesManager::CreateFileWithData(cmakePresetsFilePath, AssetsManager::GetEmbedResource<"Editor/DefaultAssets/NewProject/CMakePresets.json">());
	}

	void Project::PasteGitIgnore(const std::filesystem::path& directory) {
		std::filesystem::path filePath = directory / ".gitignore";
		FilesManager::CreateFileWithData(filePath, AssetsManager::GetEmbedResource<"Editor/DefaultAssets/NewProject/.gitignore">());
	}

	void Project::PasteAllProjectFiles(const std::filesystem::path& directory) {
		Project::PasteCmakeFile(directory);
		Project::PasteGitIgnore(directory);
	}
}