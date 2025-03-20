#pragma once
#include "Engine/Components/Component.h"
#include "Engine/Core/Scripting/Lua/LuaScript.h"
#include "Engine/Core/Engine.h"

namespace Plaza {
	class PLAZA_API LuaScriptComponent : public Component {
	  public:
		LuaScriptComponent() {}
		LuaScriptComponent(uint64_t uuid) { this->mUuid = uuid; };
		std::vector<uint64_t> mScriptsUuid = std::vector<uint64_t>();
		std::vector<LuaScript*> mScripts = std::vector<LuaScript*>();

		void AddScript(LuaScript* script) {
			mScriptsUuid.push_back(script->mAssetUuid);
			mScripts.push_back(script);
		}

		void Init() {};
		void UpdateMethods() {}

		~LuaScriptComponent() {};

		template <class Archive> void serialize(Archive& archive) {
			archive(cereal::base_class<Component>(this), PL_SER(mScriptsUuid));
		}
	};
} // namespace Plaza
