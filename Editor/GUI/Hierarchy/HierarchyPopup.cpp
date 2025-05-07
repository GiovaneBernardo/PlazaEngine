#include "Engine/Core/PreCompiledHeaders.h"
#include "HierarchyPopup.h"

#include "Editor/DefaultAssets/DefaultAssets.h"

#include "Editor/GUI/Popups/NewEntityPopup.h"
#include "Engine/Core/Scripting/CppScriptFactory.h"
#include "Editor/GUI/Hierarchy/Hierarchy.h"
#include "Engine/Core/Scene.h"
#include "Engine/ECS/ECSManager.h"

namespace Plaza::Editor {
	void HierarchyPopup::Update(Scene* scene, Entity* entity) {
		if (ImGui::BeginPopupContextWindow("ItemPopup")) {
			Popup::NewEntityPopup::Init(entity, scene);
			if (entity)
				HierarchyPopup::UpdateAddComponentPopup(scene, entity);

			ImGui::EndPopup();
		}
	}

	void HierarchyPopup::UpdateAddComponentPopup(Scene* scene, Entity* entity) {
		if (ImGui::BeginMenu("Add Component")) {
			if (ImGui::MenuItem("Mesh Renderer")) {
				MeshRenderer* meshRenderer = scene->NewComponent<MeshRenderer>(entity->uuid);
				meshRenderer->AddMaterial(AssetsManager::GetDefaultMaterial());
				meshRenderer->instanced = true;
			}

			if (ImGui::MenuItem("Rigid Body Dynamic")) {
				RigidBody* rigidBody = scene->NewComponent<RigidBody>(entity->uuid);
				rigidBody->mUuid = entity->uuid;
				ECS::RigidBodySystem::Init(scene, entity->uuid);
				ECS::ColliderSystem::InitCollider(scene, entity->uuid);
			}

			if (ImGui::MenuItem("Rigid Body No Gravity")) {
				RigidBody* rigidBody = scene->NewComponent<RigidBody>(entity->uuid);
				rigidBody->mUuid = entity->uuid;
				rigidBody->mUseGravity = false;
				ECS::ColliderSystem::InitCollider(scene, entity->uuid);
			}

			if (ImGui::MenuItem("Collider")) {
				Collider* collider = scene->NewComponent<Collider>(entity->uuid);
				ECS::ColliderSystem::InitCollider(scene, entity->uuid);
			}

			if (ImGui::MenuItem("Camera")) {
				Camera* camera = scene->NewComponent<Camera>(entity->uuid);
				camera->mUuid = entity->uuid;
			}

			if (ImGui::MenuItem("Text Renderer")) {
				Plaza::Drawing::UI::TextRenderer* textRenderer =
					scene->NewComponent<Plaza::Drawing::UI::TextRenderer>(entity->uuid);
				textRenderer->Init(Application::Get()->activeProject->mAssetPath.parent_path().string() + "/font.ttf");
				textRenderer->mUuid = entity->uuid;
			}

			if (ImGui::MenuItem("Audio Source")) {
				AudioSource* audioSource = scene->NewComponent<AudioSource>(entity->uuid);
				audioSource->mUuid = entity->uuid;
			}

			if (ImGui::MenuItem("Audio Listener")) {
				AudioListener* audioListener = scene->NewComponent<AudioListener>(entity->uuid);
				audioListener->mUuid = entity->uuid;
			}

			if (ImGui::MenuItem("Rename")) {
				entity->changingName = true;
				Scene::GetActiveScene()->GetEntity(entity->uuid)->changingName = true;
				HierarchyWindow::Item::firstFocus = true;
			}

			if (ImGui::BeginMenu("C++ Script")) {
				for (auto& [key, value] : AssetsManager::mScripts) {
					if (value->GetExtension() != ".h")
						continue;

					// TODO: FIX SCRIPTS WITH INCORRECT NAME AND REMOVE THE BELOW HACK USING THE ASSET
					Asset* asset = AssetsManager::GetAsset(key);
					if (ImGui::MenuItem(value->mAssetName.c_str())) {
						CppScriptComponent* component =
							Scene::GetActiveScene()->GetComponent<CppScriptComponent>(entity->uuid);
						if (!component) {
							component = scene->NewComponent<CppScriptComponent>(entity->uuid);
						}
						component->AddScriptNewInstance(scene, key);
					}
				}

				ImGui::EndMenu();
			}

			if (ImGui::MenuItem("Light")) {
				Light* light = scene->NewComponent<Light>(entity->uuid);
				light->mUuid = entity->uuid;
			}

			if (ImGui::MenuItem("Character Controller")) {
				CharacterController* characterController = scene->NewComponent<CharacterController>(entity->uuid);
				characterController->mUuid = entity->uuid;
			}

			if (ImGui::MenuItem("Animation")) {
				AnimationComponent* animationComponent = scene->NewComponent<AnimationComponent>(entity->uuid);
				animationComponent->mUuid = entity->uuid;
			}

			if (ImGui::MenuItem("Gui")) {
				GuiComponent* guiComponent = scene->NewComponent<GuiComponent>(entity->uuid);
				guiComponent->mUuid = entity->uuid;
			}

			ImGui::EndMenu();
		}
	}
} // namespace Plaza::Editor
