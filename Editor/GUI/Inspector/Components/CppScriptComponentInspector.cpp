#pragma once
#include "Engine/Core/PreCompiledHeaders.h"
#include "ComponentsInspector.h"
#include "Engine/Components/Scripting/CppScriptComponent.h"
#include "Engine/Core/Scene.h"
#include "Editor/GUI/Utils/Utils.h"

namespace Plaza::Editor {
	void ComponentsInspector::CppScriptComponentInspector(Scene* scene, Entity* entity) {
		CppScriptComponent* scriptComponent = scene->GetComponent<CppScriptComponent>(entity->uuid);
		ImGui::PushID("CppScriptComponentInspector");
		if (Utils::ComponentInspectorHeader(scriptComponent, "Cpp Script Component")) {
			ImGui::Text("Cpp Component");
			for (CppScript* script : scriptComponent->mScripts) {
				script->OnUpdateEditorGUI(scene);
			}
		}
		ImGui::PopID();
	}
} // namespace Plaza::Editor