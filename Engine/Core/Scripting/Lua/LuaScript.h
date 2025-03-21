#pragma once
#include "Engine/Core/Scripting/Script.h"
#include <sol/sol.hpp>
#include "LuaScriptManager.h"

namespace Plaza {
	class PLAZA_API LuaScript : public Script {
	  public:
		LuaScript() : mLuaScriptStateView(GetGlobalSolState()) {
			this->lastModifiedDate = std::chrono::system_clock::now();
		}

		~LuaScript() override {};

		void OnStart(Scene* scene) override;
		void OnUpdate(Scene* scene) override;
		void OnTerminate(Scene* scene) override;

		void GetSolFunctions();
		void LoadScriptFile(const std::string& filePath);

		sol::state_view& GetSolStateView() { return mLuaScriptStateView; };

	  private:
		sol::state_view mLuaScriptStateView;
		sol::function mOnStartFunction;
		sol::function mOnUpdateFunction;
		sol::function mOnTerminateFunction;

		sol::state& GetGlobalSolState();
	};
} // namespace Plaza
