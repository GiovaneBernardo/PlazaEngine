#pragma once
#include "Engine/Core/Script.h"
#include "ThirdParty/mono/mono/mini/jit.h"
#include "ThirdParty/mono/mono/metadata/assembly.h"
#include "Engine/Core/Scripting/Script.h"
namespace Plaza {
	namespace Editor {
		class Project {
		public:
			std::string name;
			std::string directory;

			std::string creationDate;
			std::string lastModifyDate;

			std::string scriptsConfigFilePath;

			std::map<std::string, Script> scripts;
			std::map<std::string, MonoObject*> monoObjects;

			static void Load(const std::string filePath);
		};
	}
}