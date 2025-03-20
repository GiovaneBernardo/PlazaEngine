#include "Engine/Core/PreCompiledHeaders.h"
#include "Engine/ECS/ECSManager.h"
#include "LuaScriptManager.h"

namespace Plaza {
	void LuaScriptManager::Init() { mLua->open_libraries(sol::lib::base, sol::lib::math, sol::lib::io); }

	void LuaScriptManager::Terminate() { mLua.reset(); }

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
