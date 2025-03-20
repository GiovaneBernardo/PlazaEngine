#include "Engine/Core/PreCompiledHeaders.h"
#include "Engine/ECS/ECSManager.h"
#include "LuaScriptManager.h"
#include "LuaScript.h"

namespace Plaza {
	void PLAZA_API PrintMessageLua(const std::string& msg) { PL_INFO("LUA:: {}", msg); }
	void LuaScriptManager::Init() {
		mLua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::io);
		mLua.set_function("print_message", PrintMessageLua);
	}

	void LuaScriptManager::Terminate() { }

	void LuaScriptManager::AddLuaScriptToEntity(Scene* scene, uint64_t entity, Asset* scriptAsset) {
		if (!scene->HasComponent<LuaScriptComponent>(entity))
			scene->NewComponent<LuaScriptComponent>(entity);

		LuaScript* script = new LuaScript();
		script->mAssetUuid = scriptAsset->mAssetUuid;
		script->mAssetPath = scriptAsset->mAssetPath;
		script->mAssetName = scriptAsset->mAssetName;
		script->LoadScriptFile(scriptAsset->mAssetPath.string());
		scene->GetComponent<LuaScriptComponent>(entity)->AddScript(script);
		if (scene->mRunning) {
			script->OnStart(scene);
		}
	}
} // namespace Plaza
