#pragma once
#include "Engine/Core/PreCompiledHeaders.h"
#include "ComponentsInspector.h"
#include "Engine/Components/Core/Camera.h"
#include "Editor/GUI/Utils/Utils.h"
#include "Engine/Core/Scene.h"

namespace Plaza::Editor {
	void ComponentsInspector::CameraInspector(Scene* scene, Entity* entity) {
		Camera* component = scene->GetComponent<Camera>(entity->uuid);
		if (Utils::ComponentInspectorHeader(component, "Camera")) {
			ImGui::PushID("CameraInspector");

			ImGui::PopID();
		}
	}
} // namespace Plaza::Editor
