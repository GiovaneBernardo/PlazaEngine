#pragma once
#include "Engine/Core/Scripting/Script.h"
#include <sol/sol.hpp>

namespace Plaza {
	class PLAZA_API LuaScript : public Script {
	  public:
		LuaScript() { this->lastModifiedDate = std::chrono::system_clock::now(); }
		~LuaScript() override {};

		void OnStart(Scene* scene) override;
		void OnUpdate(Scene* scene) override;
		void OnTerminate(Scene* scene) override;

		void GetSolFunctions();
		void LoadScriptFile(const std::string& filePath);

	  private:
		sol::state mLuaScriptState;
		sol::function mOnStartFunction;
		sol::function mOnUpdateFunction;
		sol::function mOnTerminateFunction;
	};
} // namespace Plaza
