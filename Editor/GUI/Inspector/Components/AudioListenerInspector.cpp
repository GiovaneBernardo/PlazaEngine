#pragma once
#include "Engine/Core/PreCompiledHeaders.h"
#include "ComponentsInspector.h"
#include "Engine/Components/Audio/AudioSource.h"
#include "Editor/GUI/Utils/Utils.h"
#include "Engine/Core/Scene.h"

namespace Plaza::Editor {
	void ComponentsInspector::AudioListenerInspector(Scene* scene, Entity* entity) {
		AudioListener* component = scene->GetComponent<AudioListener>(entity->uuid);
		if (Utils::ComponentInspectorHeader(component, "Audio Listener")) {
			ImGui::PushID("AudioListenerInspector");

			ImGui::PopID();
		}
	}
} // namespace Plaza::Editor
