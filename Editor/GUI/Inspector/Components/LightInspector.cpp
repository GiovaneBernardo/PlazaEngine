#pragma once
#include "Engine/Core/PreCompiledHeaders.h"
#include "Engine/Components/Rendering/Light.h"
#include "Editor/GUI/Utils/DataVisualizer.h"
#include "Editor/GUI/Utils/Utils.h"
#include <functional>
#include <iostream>
#include "Engine/Core/Scene.h"
#include "ComponentsInspector.h"

namespace Plaza::Editor {
	void ComponentsInspector::LightInspector(Scene* scene, Entity* entity) {
		Light* light = scene->GetComponent<Light>(entity->uuid);
		if (Utils::ComponentInspectorHeader(light, "Light Component")) {
			ImGui::PushID("LightInspector");

			Editor::Utils::DragFloat3("Color: ", light->color, 0.01f);
			Editor::Utils::DragFloat("Radius: ", light->radius, 0.01f);
			Editor::Utils::DragFloat("Cutoff: ", light->cutoff, 0.01f);
			Editor::Utils::DragFloat("Intensity: ", light->intensity, 0.01f);

			ImGui::PopID();
		}
	}
} // namespace Plaza::Editor
