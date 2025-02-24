#include "Engine/Core/PreCompiledHeaders.h"
#include "Inspector.h"

#include "Editor/GUI/Inspector/Components/ComponentsInspector.h"
#include "Editor/GUI/Inspector/FileInspector/MaterialFileInspector.h"
#include "Editor/GUI/Inspector/FileInspector/TextEditor.h"
#include "Engine/Core/Scene.h"

namespace Plaza::Editor {
	/* File Inspector*/
	std::vector<Component*> Inspector::ComponentInspector::components;
	void Inspector::FileInspector::CreateInspector() {
		ImGui::SetCursorPosY(50);
		ImGui::Indent(10);
		for (auto [key, value] : Editor::selectedFiles) {
			CreateRespectiveInspector(value);
		}
	}

	void Inspector::FileInspector::CreateRespectiveInspector(File* file) {
		if (!file->name.empty()) {
			std::string extension = std::filesystem::path{ file->directory }.extension().string();
			if (extension == Standards::materialExtName) {
				Editor::MaterialFileInspector(AssetsManager::GetMaterial(AssetsManager::GetAsset(std::filesystem::path(file->directory))->mAssetUuid));
			}
			else if (extension == Standards::modelExtName) {
				Editor::TextEditor(file);
			}
		}
	}

	/* Component Inspector*/
	void Inspector::ComponentInspector::CreateInspector(Scene* scene) {
		if (Editor::selectedGameObject) {
			ImGui::SetCursorPosY(50);
			ImGui::Indent(10);
			bool selectedEntityIsSceneEntity = Editor::selectedGameObject->uuid == Scene::GetActiveScene()->mainSceneEntity->uuid;
			if (selectedEntityIsSceneEntity) {
				Editor::ComponentsInspector::SceneInspector(Scene::GetActiveScene(), nullptr);
			}
			else {
				ImGui::Text(Editor::selectedGameObject->name.c_str());
				for (Component* component : components) {
					ComponentInspector::CreateRespectiveInspector(scene, component);
				}
				ImGui::Text("Parent Uuid: ");
				std::string uuidString = std::to_string(Editor::selectedGameObject->parentUuid);
				ImGui::Text(uuidString.c_str());
			}
		}
	}
	void Inspector::ComponentInspector::UpdateComponents() {
		components.clear();
		uint64_t uuid = Editor::selectedGameObject->uuid;
		Scene* scene = Scene::GetActiveScene();

		ComponentMask entityComponentMask = scene->GetEntity(uuid)->mComponentMask;
		for (ComponentPool* pool : scene->mComponentPools) {
			if (pool == nullptr)
				continue;
			if (entityComponentMask.test(pool->mComponentMask)) {
				components.push_back(static_cast<Component*>(pool->Get(uuid)));
			}
		}
	}

	void Inspector::ComponentInspector::CreateRespectiveInspector(Scene* scene, Component* component) {
		if (TransformComponent* transform = dynamic_cast<TransformComponent*>(component)) {
			Plaza::Editor::ComponentsInspector::TransformInspector(scene, scene->GetEntity(component->mUuid));
		}
		else if (MeshRenderer* meshRenderer = dynamic_cast<MeshRenderer*>(component)) {
			Plaza::Editor::ComponentsInspector::MeshRendererInspector(scene, scene->GetEntity(component->mUuid));
			//Plaza::Editor::MaterialInspector::MaterialInspector(Editor::selectedGameObject);
		}
		else if (Camera* camera = dynamic_cast<Camera*>(component)) {
			Plaza::Editor::ComponentsInspector::CameraInspector(scene, scene->GetEntity(component->mUuid));
		}
		else if (RigidBody* rigidBody = dynamic_cast<RigidBody*>(component)) {
			Plaza::Editor::ComponentsInspector::RigidBodyInspector(scene, scene->GetEntity(component->mUuid));
		}
		else if (Collider* collider = dynamic_cast<Collider*>(component)) {
			Plaza::Editor::ComponentsInspector::ColliderInspector(scene, scene->GetEntity(component->mUuid));
		}
		else if (CppScriptComponent* cppScriptComponent = dynamic_cast<CppScriptComponent*>(component)) {
			Plaza::Editor::ComponentsInspector::CppScriptComponentInspector(scene, scene->GetEntity(component->mUuid));
		}
		else if (AudioSource* audioSource = dynamic_cast<AudioSource*>(component)) {
			Plaza::Editor::ComponentsInspector::AudioSourceInspector(scene, scene->GetEntity(component->mUuid));
		}
		else if (AudioListener* audioListener = dynamic_cast<AudioListener*>(component)) {
			Plaza::Editor::ComponentsInspector::AudioListenerInspector(scene, scene->GetEntity(component->mUuid));
		}
		else if (Light* light = dynamic_cast<Light*>(component)) {
			Plaza::Editor::ComponentsInspector::LightInspector(scene, scene->GetEntity(component->mUuid));
		}
		else if (Drawing::UI::TextRenderer* textRenderer = dynamic_cast<Drawing::UI::TextRenderer*>(component)) {
			Plaza::Editor::ComponentsInspector::TextRendererInspector(scene, scene->GetEntity(component->mUuid));
		}
		else if (AnimationComponent* animationComponent = dynamic_cast<AnimationComponent*>(component)) {
			Plaza::Editor::ComponentsInspector::AnimationComponentInspector(scene, scene->GetEntity(component->mUuid));
		}
		else if (GuiComponent* guiComponent = dynamic_cast<GuiComponent*>(component)) {
			Plaza::Editor::ComponentsInspector::GuiComponentInspector(scene, scene->GetEntity(component->mUuid));
		}
	}
}