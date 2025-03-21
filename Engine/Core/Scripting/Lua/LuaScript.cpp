#include "Engine/Core/PreCompiledHeaders.h"
#include "LuaScript.h"
#include "LuaScriptManager.h"
#include <sol/sol.hpp>

namespace Plaza {
	void LuaScript::OnStart(Scene* scene) {
		if (mOnStartFunction)
			mOnStartFunction();
	};
	void LuaScript::OnUpdate(Scene* scene) {
		if (mOnUpdateFunction)
			mOnUpdateFunction();
	};
	void LuaScript::OnTerminate(Scene* scene) {
		if (mOnTerminateFunction)
			mOnTerminateFunction();
	};

	void LuaScript::GetSolFunctions() {
		mOnStartFunction = mLuaScriptStateView["OnStart"];
		mOnUpdateFunction = mLuaScriptStateView["OnUpdate"];
		mOnTerminateFunction = mLuaScriptStateView["OnTerminate"];
	}

	void LuaScript::LoadScriptFile(const std::string& filePath) {
		try {
			mLuaScriptStateView = sol::state_view(LuaScriptManager::GetLuaState());
			mLuaScriptStateView.script_file(filePath);
		} catch (const sol::error& error) {
			PL_CORE_ERROR("Lua Error: {}", error.what());
		}

		GetSolFunctions();
	}

	sol::state& LuaScript::GetGlobalSolState() { return LuaScriptManager::GetLuaState(); }
} // namespace Plaza
