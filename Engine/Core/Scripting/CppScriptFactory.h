#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <functional>
#include <memory>
#include <type_traits>
#include "Script.h"
#ifdef WIN32
#include <Windows.h>
#endif
#include "Engine/Core/Engine.h"
#include "Engine/Components/Scripting/CppScriptComponent.h"
#include "Engine/Core/Scripting/CppScript.h"

typedef void (*RegisterScriptsFunc)();
namespace Plaza {
	typedef std::function<CppScript*()> CppScriptCreator;
	typedef std::function<void(CppScript*)> CppScriptDeleter;

	typedef std::unordered_map<std::string, Plaza::CppScriptCreator>& (*GetRegisterScriptsFunc)();

	extern class PLAZA_API ScriptFactory {
	  public:
		static void InitializeRegistry();
		static void RegisterScript(const std::string& name, CppScriptCreator creator, CppScriptDeleter deleter);
		static CppScript* CreateScript(const std::string& name);
		static void DeleteScript(CppScript* script);

		static std::unordered_map<std::string, CppScriptCreator>& GetCreateRegistry();
		static std::unordered_map<std::string, CppScriptDeleter>& GetDeleteRegistry();

	  private:
		static inline std::unordered_map<std::string, CppScriptCreator> sCreateRegistry =
			std::unordered_map<std::string, CppScriptCreator>();
		static inline std::unordered_map<std::string, CppScriptDeleter> sDeleteRegistry =
			std::unordered_map<std::string, CppScriptDeleter>();
	};
	// Macro to automate the script registration
#define PL_REGISTER_SCRIPT(T)                                                                                          \
	static struct T##Registration {                                                                                    \
		T##Registration() {                                                                                            \
			Plaza::ScriptFactory::RegisterScript(                                                                      \
				#T, []() -> CppScript* { return new T(); },                                                            \
				[](CppScript* script) { delete static_cast<T*>(script); });                                            \
		}                                                                                                              \
	} T##Reg;
} // namespace Plaza
