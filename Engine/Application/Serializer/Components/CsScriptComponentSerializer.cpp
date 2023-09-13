#include "Engine/Core/PreCompiledHeaders.h"
#include "CsScriptComponentSerializer.h"
namespace Plaza {
	void ComponentSerializer::CsScriptComponentSerializer::SerializeAll(YAML::Emitter& out, uint64_t uuid) {
		out << YAML::Key << "CsScriptComponent" << YAML::BeginMap;
		out << YAML::Key << "Uuid" << YAML::Value << uuid;
		out << YAML::Key << "Scripts" << YAML::Value << YAML::BeginSeq;
		auto range = Application->activeScene->csScriptComponents.equal_range(uuid);
		for (auto it = range.first; it != range.second; ++it) {
			ComponentSerializer::CsScriptComponentSerializer::Serialize(out, it->second);
		}
		out << YAML::EndSeq; // Scripts

		out << YAML::EndMap; // CsScriptComponent
	}
	void ComponentSerializer::CsScriptComponentSerializer::Serialize(YAML::Emitter& out, CsScriptComponent& script) {
		out << YAML::BeginMap;

		out << YAML::Key << "Name" << YAML::Value << std::filesystem::path{ script.scriptPath }.stem().string();
		out << YAML::Key << "Path" << YAML::Value << script.scriptPath;
		out << YAML::Key << "Uuid" << YAML::Value << script.uuid;

		out << YAML::EndMap;
	}

	CsScriptComponent* ComponentSerializer::CsScriptComponentSerializer::DeSerialize(YAML::Node data) {
		CsScriptComponent* script = new CsScriptComponent(data["Uuid"].as<uint64_t>());
		std::string csFileName = data["Path"].as<std::string>();
		script->Init(csFileName);
		Application->activeProject->scripts.at(csFileName).entitiesUsingThisScript.emplace(data["Uuid"].as<uint64_t>());
		return script;
	}

	/*
						for (auto& [key, value] : Application->activeProject->scripts) {
						if (ImGui::MenuItem(filesystem::path{ key }.stem().string().c_str())) {
							CsScriptComponent* script = new CsScriptComponent(entity.uuid);
							std::string csFileName = filesystem::path{ key }.replace_extension(".cs").string();
							script->Init(csFileName);;
							Application->activeProject->scripts.at(csFileName).entitiesUsingThisScript.emplace(entity.uuid);
							if (Application->runningScene) {
								for (auto& [key, value] : script->scriptClasses) {
									Mono::OnStart(value->monoObject);
								}
							}
							entity.AddComponent<CsScriptComponent>(script);
						}
					}
	*/
}

/*
#pragma once
#include "ComponentSerializer.h"
#include "Engine/Components/Scripting/CppScriptComponent.h"
namespace Plaza {
	class ComponentSerializer::CsScriptComponentSerializer {
	public:
		static void Serialize(YAML::Emitter& out, CsScriptComponent& script);
		static CsScriptComponent* DeSerialize(YAML::Node data);
	};
}
*/