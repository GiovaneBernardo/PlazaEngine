#include "Engine/Core/PreCompiledHeaders.h"
#include "LuaScript.h"

namespace Plaza {
	void PLAZA_API PrintMessageLua(const std::string& msg) { PL_INFO("LUA:: {}", msg); }
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
		mOnStartFunction = mLuaScriptState["OnStart"];
		mOnUpdateFunction = mLuaScriptState["OnUpdate"];
		mOnTerminateFunction = mLuaScriptState["OnTerminate"];
	}

	void LuaScript::LoadScriptFile(const std::string& filePath) {
		try {
			mLuaScriptState.script_file(filePath);
			mLuaScriptState.set_function("print_message", PrintMessageLua);
		} catch (const sol::error& error) {
			PL_CORE_ERROR("Lua Error: {}", error.what());
		}
		GetSolFunctions();
	}
} // namespace Plaza
