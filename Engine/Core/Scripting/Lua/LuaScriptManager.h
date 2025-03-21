#pragma once

#include "Engine/Core/Scripting/Script.h"
#include <sol/sol.hpp>

namespace Plaza {
	class PLAZA_API LuaScriptManager {
	  public:
		static void Init();
		static void Terminate();

		static void AddLuaScriptToEntity(Scene* scene, uint64_t entity, Asset* scriptAsset);

		static sol::state& GetLuaState() { return mLua; };

	  private:
		static inline sol::state mLua;
	};
} // namespace Plaza
