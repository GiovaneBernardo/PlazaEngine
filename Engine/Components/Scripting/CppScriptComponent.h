#pragma once
#include "Engine/Components/Component.h"
#include "Engine/Core/Scripting/CppScript.h"

namespace Plaza {
	class CppScriptComponent : public  Component {
	public:
		CppScriptComponent() {}
		CppScriptComponent(uint64_t uuid) { this->mUuid = uuid; };
		std::vector<uint64_t> mScriptsUuid = std::vector<uint64_t>();
		std::vector<CppScript*> mScripts = std::vector<CppScript*>();
		
		//std::map<std::string, PlazaScriptClass*> scriptClasses;;

		void Init() {};
		void UpdateMethods() {

		}

		~CppScriptComponent() {};

		template <class Archive>
		void serialize(Archive& archive) {
			archive(cereal::base_class<Component>(this), PL_SER(mScriptsUuid));
		}
	};
}