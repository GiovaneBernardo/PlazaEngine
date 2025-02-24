#include "Engine/Core/PreCompiledHeaders.h"
#include "ScriptManager.h"
#include "Engine/Core/Scripting/Scripting.h"
#include "Engine/Core/Scene.h"

namespace Plaza::Editor {
	void ScriptManager::ReloadScriptsAssembly(Scene* scene, const std::string& dllPath) {

	}

	void ScriptManager::ReloadScriptsAssembly(Scene* scene) {
		Scripting::LoadProjectCppDll(scene, *Application::Get()->activeProject);
	}

	void ScriptManager::ReloadSpecificAssembly(Scene* scene, const std::string& scriptPath) {

	}

	void ScriptManager::RecompileDll(std::filesystem::path dllPath, std::string scriptPath) {

	}

	std::unordered_map<uint64_t, std::map<std::string, std::unordered_map<std::string, uint32_t>>> ScriptManager::GetAllFields() {
		std::unordered_map<uint64_t, std::map<std::string, std::unordered_map<std::string, uint32_t>>> fields;
		return fields;
	}

	void ScriptManager::GetField() {

	}

	void ScriptManager::SaveAllFields(std::unordered_map<uint64_t, std::map<std::string, std::unordered_map<std::string, uint32_t>>> fields) {

	}

	void ScriptManager::SaveField() {

	}
}