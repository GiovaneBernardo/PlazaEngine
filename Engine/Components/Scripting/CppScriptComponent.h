#pragma once
#include "Engine/Components/Component.h"
#include "Engine/Core/Scripting/CppScript.h"
#include "Engine/Core/Engine.h"

namespace Plaza {
	class PLAZA_API CppScriptComponent : public Component {
	  public:
		CppScriptComponent() {}
		CppScriptComponent(uint64_t uuid) { this->mUuid = uuid; };
		std::vector<uint64_t> mScriptsUuid = std::vector<uint64_t>();
		std::vector<CppScript*> mScripts = std::vector<CppScript*>();

		void AddScript(CppScript* script);
		CppScript* AddScriptNewInstance(Scene* scene, uint64_t scriptUuid);
		// std::map<std::string, PlazaScriptClass*> scriptClasses;;

		void Init() {};
		void UpdateMethods() {}

		~CppScriptComponent(){};

		template <class Archive> void serialize(Archive& archive) {
			archive(cereal::base_class<Component>(this), PL_SER(mScriptsUuid));
		}
	};
} // namespace Plaza
