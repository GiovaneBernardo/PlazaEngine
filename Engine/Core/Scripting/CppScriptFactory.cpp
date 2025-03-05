#include "CppScriptFactory.h"
#include <unordered_map>
#include <string>

namespace Plaza {
	void ScriptFactory::InitializeRegistry() {
		// sRegistry = new std::unordered_map<std::string, CppScriptCreator>();
	}

	void ScriptFactory::RegisterScript(const std::string& name, CppScriptCreator creator, CppScriptDeleter deleter) {
		GetCreateRegistry()[name] = creator;
		GetDeleteRegistry()[name] = deleter;
	}

	CppScript* ScriptFactory::CreateScript(const std::string& name) {
		auto it = GetCreateRegistry().find(name);
		if (it != GetCreateRegistry().end()) {
			return it->second(); // Call the creation function
		}
		return nullptr;
	}

	void ScriptFactory::DeleteScript(CppScript* script) {
		auto it = GetDeleteRegistry().find(script->mAssetName);
		if (it != GetDeleteRegistry().end()) {
			it->second(script);
		}
	}

	std::unordered_map<std::string, CppScriptCreator>& ScriptFactory::GetCreateRegistry() { return sCreateRegistry; };

	std::unordered_map<std::string, CppScriptDeleter>& ScriptFactory::GetDeleteRegistry() { return sDeleteRegistry; };
} // namespace Plaza
