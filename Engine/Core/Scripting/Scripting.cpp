#pragma once
#include "Engine/Core/PreCompiledHeaders.h"
#include "Scripting.h"
#include "CppScript.h"
#include "Engine/Components/Scripting/CppScriptComponent.h"
#include "Engine/Core/Scripting/CppScriptFactory.h"
#include "Engine/Core/Input/Input.h"
#include "Engine/Core/Scene.h"

#ifdef __linux
#include <dlfcn.h>
#endif

namespace Plaza {
	void Scripting::LoadProjectCppDll(Scene* scene, const Editor::Project& project) {
		Scripting::UnloadAllScripts(scene);
		ScriptFactory::GetCreateRegistry().clear();
		ScriptFactory::GetDeleteRegistry().clear();
		Scripting::UnloadCurrentLoadedCppDll();

#ifdef EDITOR_MODE
		bool loaded = Scripting::LoadCppDll(Scripting::CopyPasteDevelopmentLibraryFiles(project));
#else
		bool loaded = xScripting::LoadCppDll(project.mAssetPath.parent_path() / "GameLib.dll");
#endif
		if (!loaded)
			return;

		Scripting::ReloadAllScripts(scene);

		sReloadIndex++;
	}

	void Scripting::ReloadAllScripts(Scene* scene) {
		for (const uint64_t& uuid : SceneView<CppScriptComponent>(scene)) {
			auto& component = *scene->GetComponent<CppScriptComponent>(uuid);
			std::vector<uint64_t> scriptsUuid = component.mScriptsUuid;
			for (uint64_t uuid : scriptsUuid) {
				CppScript* script = ScriptFactory::CreateScript(std::filesystem::path(AssetsManager::GetAsset(uuid)->mAssetName).stem().string());
				script->mAssetUuid = uuid;
				component.AddScript(script);
				script->OnStart(scene);
			}
		}
	}

	void Scripting::UnloadAllScripts(Scene* scene) {
		for (const uint64_t& uuid : SceneView<CppScriptComponent>(scene)) {
			auto& component = *scene->GetComponent<CppScriptComponent>(uuid);
			for (auto& script : component.mScripts) {
				if (std::find(component.mScripts.begin(), component.mScripts.end(), script) != component.mScripts.end())
					component.mScripts.erase(std::find(component.mScripts.begin(), component.mScripts.end(), script));
				ScriptFactory::DeleteScript(script);
				//delete script;
			}
		}
	}

	std::filesystem::path Scripting::CopyPasteDevelopmentLibraryFiles(const Editor::Project& project) {
		std::filesystem::path dllPath = std::filesystem::path(project.mAssetPath).parent_path() / "bin" / ("GameLib.dll");
		if (!FilesManager::PathExists(dllPath)) {
			PL_CORE_WARN("C++ Game Dll not found");
			return dllPath;
		}
		std::filesystem::path copyFolder = ("bin/copy" + std::to_string(sReloadIndex) + "/");
		FilesManager::CreateNewDirectory(std::filesystem::path(project.mAssetPath).parent_path() / copyFolder);

		std::filesystem::path dllPastePath = std::filesystem::path(project.mAssetPath).parent_path() / copyFolder / ("GameLib.dll");
		dllPastePath = FilesManager::CopyPasteFile(dllPath, dllPastePath, true);

		// Get the pdb and make a copy of it
		std::filesystem::path pdbPath = std::filesystem::path(project.mAssetPath).parent_path() / "bin" / ("GameLib.pdb");
		std::filesystem::path pdbPastePath = std::filesystem::path(dllPastePath);
		pdbPastePath.replace_extension(".pdb");
		if (FilesManager::PathExists(pdbPath))
			FilesManager::CopyPasteFile(pdbPath, pdbPastePath, true);

		return dllPastePath;
	}

	void Scripting::PasteEngineLibToGameProject(const Editor::Project& project) {

	}

	void Scripting::Update(Scene* scene) {
		PLAZA_PROFILE_SECTION("Scripting: Update");

		Input::SetFocusedMenuCheck("Scene");

		for (const uint64_t& uuid : SceneView<CppScriptComponent>(scene)) {
			auto& component = *scene->GetComponent<CppScriptComponent>(uuid);
			for (auto& script : component.mScripts) {
				script->OnUpdate(scene);
			}
		}
	}

	bool Scripting::LoadCppDll(const std::filesystem::path& path) {
		if (!FilesManager::PathExists(path)) {
			PL_CORE_WARN("Dll not found");
			return false;
		}

#ifdef WIN32
		sCurrentLoadedCppDll = LoadLibrary(path.string().c_str());
		if (sCurrentLoadedCppDll == NULL) {
			DWORD errorMessageID = GetLastError();
			PL_CORE_WARN("Failed to load DLL, error: ${0}", errorMessageID);
			return false;
		}
#elif __linux__
	sCurrentLoadedCppHandle = dlopen(path.c_str(), RTLD_LAZY);
#endif
		return true;
	}

	void Scripting::UnloadCurrentLoadedCppDll() {
#ifdef WIN32
		FreeLibrary(sCurrentLoadedCppDll);
#elif __linux__
		dlclose(sCurrentLoadedCppHandle);
#endif
	}
}