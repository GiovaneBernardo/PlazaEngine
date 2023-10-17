#include "Engine/Core/PreCompiledHeaders.h"
#include "Inspector.h"

#include "Editor/GUI/Inspector/RigidBodyInspector.h"
#include "Editor/GUI/Inspector/SceneInspector.h"
#include "Editor/GUI/Inspector/ColliderInspector.h"
#include "Editor/GUI/Inspector/CppScriptComponentInspector.h"
#include "Editor/GUI/Inspector/AudioListenerInspector.h"
#include "Editor/GUI/Inspector/AudioSourceInspector.h"

namespace Plaza::Editor {
	std::vector<Component*> Inspector::ComponentInspector::components;
	void Inspector::ComponentInspector::CreateInspector() {
		ImGui::SetCursorPosY(50);
		ImGui::Indent(10);
		if (Editor::selectedGameObject->uuid == Application->activeScene->mainSceneEntity->uuid) {
			SceneInspector::SceneInspector(Application->activeScene);
		}
		else {
			ImGui::Text(Editor::selectedGameObject->name.c_str());
			for (Component* component : components) {
				CreateRespectiveInspector(component);
			}
			ImGui::Text("Parent Uuid: ");
			std::string uuidString = std::to_string(Editor::selectedGameObject->parentUuid);
			ImGui::Text(uuidString.c_str());
		}
	}
	void Inspector::ComponentInspector::UpdateComponents() {
		components.clear();
		uint64_t uuid = Editor::selectedGameObject->uuid;
		Scene* activeScene = Application->activeScene;

		if (activeScene->transformComponents.contains(uuid))
			components.push_back(&activeScene->transformComponents.at(uuid));

		if (activeScene->meshRendererComponents.contains(uuid))
			components.push_back(&activeScene->meshRendererComponents.at(uuid));

		if (activeScene->rigidBodyComponents.contains(uuid))
			components.push_back(&activeScene->rigidBodyComponents.at(uuid));

		if (activeScene->colliderComponents.contains(uuid))
			components.push_back(&activeScene->colliderComponents.at(uuid));

		if (activeScene->csScriptComponents.contains(uuid)) {
			auto range = activeScene->csScriptComponents.equal_range(uuid);
			for (auto it = range.first; it != range.second; ++it) {
				components.push_back(&it->second);
			}
		}

		if (activeScene->audioSourceComponents.contains(uuid))
			components.push_back(&activeScene->audioSourceComponents.at(uuid));
		if (activeScene->audioListenerComponents.contains(uuid))
			components.push_back(&activeScene->audioListenerComponents.at(uuid));
	}

	void Inspector::ComponentInspector::CreateRespectiveInspector(Component* component) {
		if (Transform* transform = dynamic_cast<Transform*>(component)) {
			Editor::Gui::TransformInspector inspector{ Editor::selectedGameObject };
		}
		else if (MeshRenderer* meshRenderer = dynamic_cast<MeshRenderer*>(component)) {
			Plaza::Editor::MaterialInspector::MaterialInspector(Editor::selectedGameObject);
		}
		else if (RigidBody* rigidBody = dynamic_cast<RigidBody*>(component)) {
			Plaza::Editor::RigidBodyInspector::RigidBodyInspector(rigidBody);
		}
		else if (Collider* collider = dynamic_cast<Collider*>(component)) {
			Plaza::Editor::ColliderInspector::ColliderInspector(collider);
		}
		else if (CsScriptComponent* csScriptComponent = dynamic_cast<CsScriptComponent*>(component)) {
			Plaza::Editor::CppScriptComponentInspector::CppScriptComponentInspector(csScriptComponent);
		}
		else if (AudioSource* audioSource = dynamic_cast<AudioSource*>(component)) {
			Plaza::Editor::AudioSourceInspector::AudioSourceInspector(audioSource);
		}
		else if (AudioListener* audioListener = dynamic_cast<AudioListener*>(component)) {
			Plaza::Editor::AudioListenerInspector::AudioListenerInspector(audioListener);
		}
	}
}