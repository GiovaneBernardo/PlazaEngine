#pragma once
#include "Engine/Core/PreCompiledHeaders.h"
#include "Scripting.h"
#include "Mono.h"
#include "CppScript.h"
#include "Engine/Components/Scripting/CppScriptComponent.h"
#include "Engine/Core/Scripting/CppScriptFactory.h"

namespace Plaza {
	void Scripting::LoadProjectCppDll(const Editor::Project& project) {
		Scripting::UnloadAllScripts();
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

		Scripting::ReloadAllScripts();

		sReloadIndex++;
	}

	void Scripting::ReloadAllScripts() {
		for (auto& [componentUuid, component] : Scene::GetActiveScene()->cppScriptComponents) {
			std::vector<uint64_t> scriptsUuid = component.mScriptsUuid;
			for (uint64_t uuid : scriptsUuid) {
				CppScript* script = ScriptFactory::CreateScript(std::filesystem::path(AssetsManager::GetAsset(uuid)->mAssetName).stem().string());
				script->mAssetUuid = uuid;
				component.AddScript(script);
				script->OnStart();
			}
		}
	}

	void Scripting::UnloadAllScripts() {
		for (auto& [componentUuid, component] : Scene::GetActiveScene()->cppScriptComponents) {
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

	void Scripting::Update() {
		PLAZA_PROFILE_SECTION("Scripting: Update");
		Mono::Update();

		for (auto& [key, value] : Scene::GetActiveScene()->cppScriptComponents) {
			for (auto& script : value.mScripts) {
				script->OnUpdate();
			}
		}
	}

	bool Scripting::LoadCppDll(const std::filesystem::path& path) {
		if (!FilesManager::PathExists(path)) {
			PL_CORE_WARN("Dll not found");
			return false;
		}
		sCurrentLoadedCppDll = LoadLibrary(path.string().c_str());
		if (sCurrentLoadedCppDll == NULL) {
			DWORD errorMessageID = GetLastError();
			PL_CORE_WARN("Failed to load DLL, error: ${0}", errorMessageID);
			return false;
		}
		return true;
	}

	void Scripting::UnloadCurrentLoadedCppDll() {
		FreeLibrary(sCurrentLoadedCppDll);
	}
}