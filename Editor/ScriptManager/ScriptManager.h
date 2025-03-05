#pragma once
#include "Engine/Core/PreCompiledHeaders.h"
namespace Plaza::Editor {
	class ScriptManager {
	  public:
		static void NewCsScript(std::string fullPath);
		static void ReloadScriptsAssembly(Scene* scene, const std::string& dllPath);
		static void ReloadScriptsAssembly(Scene* scene);
		static void ReloadSpecificAssembly(Scene* scene, const std::string& scriptPath);

		static void RecompileDll(std::filesystem::path dllPath, std::string scriptPath);
		static std::unordered_map<uint64_t, std::map<std::string, std::unordered_map<std::string, uint32_t>>>
		GetAllFields();
		static void GetField();
		static void SaveAllFields(
			std::unordered_map<uint64_t, std::map<std::string, std::unordered_map<std::string, uint32_t>>> fields);
		static void SaveField();
	};
} // namespace Plaza::Editor
