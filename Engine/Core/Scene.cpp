#include "Engine/Core/PreCompiledHeaders.h"
#include "Scene.h"
#include "Engine/Components/Core/Entity.h"
#include "Editor/ScriptManager/ScriptManager.h"
#include "Engine/Core/Input/Input.h"
#include "Engine/Core/Input/Cursor.h"
#include "Engine/Core/Scripting/FieldManager.h"
#include "Editor/Filewatcher.h"
#include "Engine/Core/Physics.h"
#include <mutex>
#include "Engine/Core/AssetsManager/Loader/AssetsLoader.h"
#include "Engine/Core/Scripting/Scripting.h"
#include "Engine/ECS/ECSManager.h"

namespace Plaza {
	std::shared_ptr<Scene> Scene::sEditorScene = nullptr;
	std::shared_ptr<Scene> Scene::sRuntimeScene = nullptr;
	Scene* Scene::sActiveScene = nullptr;
	void Scene::Copy(Scene* baseScene) {
		*this = *baseScene;
		this->mComponentPools.clear();
		for (ComponentPool* pool : baseScene->mComponentPools) {
			if (pool) {
				this->mComponentPools.push_back(new ComponentPool(*pool));
			}
			else
				this->mComponentPools.push_back(nullptr);
		}
		mainSceneEntity = this->GetEntity(baseScene->mainSceneEntityUuid);

		for (const uint64_t& uuid : SceneView<MeshRenderer>(this)) {
			MeshRenderer& meshRenderer = *this->GetComponent<MeshRenderer>(uuid);
			if (meshRenderer.mMaterialsUuids.size() > 0 && meshRenderer.GetMesh()) {
				RenderGroup* newRenderGroup = new RenderGroup(meshRenderer.GetMesh(), meshRenderer.GetMaterials());
				meshRenderer.renderGroup = AddRenderGroup(newRenderGroup);
			}
		}
	}

	void Scene::NewRuntimeScene(Scene* baseScene) {}

	Scene::Scene() {
		this->mAssetUuid = Plaza::UUID::NewUUID();
		mViewport = PlViewport(0.0f, 0.0f, Application::Get()->appSizes->sceneSize.x,
							   Application::Get()->appSizes->sceneSize.y, 0.0f, 1.0f);
	}

	void Scene::Play() {
		/* Restart physics */
		Physics::m_scene->release();
		Physics::InitPhysics();
		Physics::InitScene();

		Scene::sRuntimeScene.reset();
		Scene::sRuntimeScene = std::make_unique<Scene>();
		Scene::sRuntimeScene->InitMainEntity();

		Scene::sRuntimeScene->Copy(Scene::GetEditorScene());
		Scene* scene = Scene::GetRuntimeScene();

		Scene::SetActiveScene(Scene::GetRuntimeScene());
		Scene::sRuntimeScene->mRunning = true;

		for (const uint64_t& uuid : SceneView<Collider>(scene)) {
			Collider& collider = *scene->GetComponent<Collider>(uuid);
			collider.lastScale = glm::vec3(0.0f);
			ECS::ColliderSystem::InitCollider(scene, collider.mUuid);
			ECS::ColliderSystem::UpdatePose(&collider, scene->GetComponent<TransformComponent>(uuid));
		}
		for (const uint64_t& uuid : SceneView<RigidBody>(scene)) {
			RigidBody& rigidBody = *scene->GetComponent<RigidBody>(uuid);
			ECS::RigidBodySystem::Init(scene, uuid);
		}
#ifdef EDITOR_MODE
		ImGui::SetWindowFocus("Scene");
#endif
		int width, height;
		glfwGetWindowSize(Application::Get()->mWindow->glfwWindow, &width, &height);
		glfwSetCursorPos(Application::Get()->mWindow->glfwWindow, width / 2, height / 2);
		Input::Cursor::lastX = 0;
		Input::Cursor::lastY = 0;
		Input::Cursor::deltaX = 0;
		Input::Cursor::deltaY = 0;

		Scripting::LoadProjectCppDll(scene, *Application::Get()->activeProject);

		for (const uint64_t& uuid : SceneView<Collider>(scene)) {
			Collider& collider = *scene->GetComponent<Collider>(uuid);
			ECS::ColliderSystem::UpdateShapeScale(scene, &collider,
												  scene->GetComponent<TransformComponent>(uuid)->GetWorldScale());
			ECS::ColliderSystem::UpdatePose(&collider, scene->GetComponent<TransformComponent>(uuid));
		}

		ECS::TransformSystem::UpdateSelfAndChildrenTransform(
			*scene->GetComponent<TransformComponent>(scene->mainSceneEntityUuid), nullptr, scene);
	}

	void Scene::Stop() {
		/* Clear the queue on the main thread */
		std::lock_guard<std::mutex> lock(Editor::Filewatcher::queueMutex);
		while (!Editor::Filewatcher::taskQueue.empty()) {
			Editor::Filewatcher::taskQueue.pop();
		}

		for (const uint64_t& uuid : SceneView<AudioSource>(Scene::GetActiveScene())) {
			AudioSource& source = *Scene::GetActiveScene()->GetComponent<AudioSource>(uuid);
			source.Stop();
		}

		// Change active scene, update the selected object scene, delete runtime and set running to false.
		Editor::selectedGameObject = nullptr;
		// delete(Application::Get()->runtimeScene);
		Scene::sRuntimeScene.reset();

		Scene::SetActiveScene(Scene::GetEditorScene());
		Application::Get()->activeCamera = Application::Get()->editorCamera;

		for (const uint64_t& uuid : SceneView<MeshRenderer>(Scene::GetEditorScene())) {
			MeshRenderer& meshRenderer = *Scene::GetEditorScene()->GetComponent<MeshRenderer>(uuid);
			if (meshRenderer.mMaterialsUuids.size() > 0 && meshRenderer.GetMesh()) {
				RenderGroup* newRenderGroup = new RenderGroup(meshRenderer.GetMesh(), meshRenderer.GetMaterials());
				meshRenderer.renderGroup = Scene::GetEditorScene()->AddRenderGroup(newRenderGroup);
			}
		}
	}
	void Scene::Pause() {}

	void Scene::RecalculateAddedComponents() {
		for (const uint64_t& uuid : SceneView<MeshRenderer>(this)) {
			MeshRenderer& component = *this->GetComponent<MeshRenderer>(uuid);
			component.renderGroup =
				this->AddRenderGroup(AssetsManager::GetMesh(component.mMeshUuid), component.GetMaterials());
		}

		this->mainSceneEntity = this->GetEntity(this->mainSceneEntityUuid);
		ECS::TransformSystem::UpdateSelfAndChildrenTransform(
			*this->GetComponent<TransformComponent>(mainSceneEntityUuid), nullptr, this, true, true);

		// for (auto& [componentUuid, component] : guiComponents) {
		//	for (auto& [key, value] : component.mGuiItems) {
		//		glm::mat4 parentTransform = component.HasGuiItem(value->mGuiParentUuid) ?
		// component.GetGuiItem<GuiItem>(value->mGuiParentUuid)->mTransform : glm::mat4(1.0f);
		//		GuiItem::UpdateSelfAndChildrenTransform(value.get(), parentTransform);
		//	}
		// }
		// entitiesNames = std::unordered_map<std::string, std::unordered_set<uint64_t>>();
		// entitiesNames.reserve(entities.size());
		// for (auto& [key, value] : entities) {
		//	entitiesNames[value.name].insert(key);
		// }
	}

	void Scene::InitializeScenes() {
		sEditorScene = std::make_shared<Scene>();
		sRuntimeScene = std::make_shared<Scene>();
	}

	Scene* Scene::GetEditorScene() { return sEditorScene.get(); }
	void Scene::SetEditorScene(std::shared_ptr<Scene> scene) { sEditorScene = scene; }
	void Scene::ClearEditorScene() {
		sEditorScene.reset();
		sEditorScene = std::make_shared<Scene>();
	}
	Scene* Scene::GetRuntimeScene() { return sRuntimeScene.get(); }
	Scene* Scene::GetActiveScene() { return sActiveScene; }
	void Scene::SetActiveScene(Scene* scene) { sActiveScene = scene; }

	void Scene::Terminate() {
		sActiveScene = nullptr;
		sEditorScene.reset();
		sRuntimeScene.reset();
	}

	void Scene::RemoveEntity(uint64_t uuid) {
		// Remove all components related to entity
		for (auto& componentPool : mComponentPools) {
			if (componentPool->Get(uuid) != nullptr) {
				componentPool->Remove(uuid);
			}
		}

		// Remove entity reference on entities names
		auto it = entitiesNames.find(GetEntity(uuid)->name);
		if (it != entitiesNames.end()) {
			it->second.erase(std::find(it->second.begin(), it->second.end(), uuid));
		}

		// Remove entity
		entities.erase(entities.find(uuid));

	}
} // namespace Plaza
