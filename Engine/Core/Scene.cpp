#include "Engine/Core/PreCompiledHeaders.h"
#include "Scene.h"
#include "Engine/Core/Scripting/Mono.h"
#include "Engine/Components/Core/Entity.h"
#include "Editor/ScriptManager/ScriptManager.h"
#include "Engine/Core/Input/Input.h"
#include "Engine/Core/Input/Cursor.h"
#include "Engine/Core/Scripting/FieldManager.h"
#include "Editor/Filewatcher.h"
#include "Engine/Core/Physics.h"
#include <mutex>
#include "Engine/Core/AssetsManager/Loader/AssetsLoader.h"
#include "ThirdParty/mono/include/mono/metadata/mono-debug.h"
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

	void Scene::NewRuntimeScene(Scene* baseScene) {

	}

	Scene::Scene() {
		this->mAssetUuid = Plaza::UUID::NewUUID();
	}

	void Scene::Play() {
		bool scriptDllExists = std::filesystem::exists(Application::Get()->projectPath + "\\Binaries\\" + std::filesystem::path{ Application::Get()->activeProject->mAssetName }.stem().string() + ".dll");
		/* Get fields values */
		std::map<uint64_t, std::map<std::string, std::map<std::string, Field*>>> allFields;
		if (scriptDllExists) {
			allFields = FieldManager::GetAllScritpsFields(Scene::GetEditorScene());
			Editor::lastSavedScriptsFields = allFields;
		}
		/* Stop mono */
		/* Load the new domain */
		mono_set_assemblies_path("lib/mono");
		//mono_set_assemblies_path((Application::Get()->editorPath + "/lib/mono").c_str());
		if (Mono::mMonoRootDomain == nullptr)
			Mono::mMonoRootDomain = mono_jit_init("MyScriptRuntime");
		if (Mono::mMonoRootDomain == nullptr)
		{
			// Maybe log some error here
			return;
		}
		// Create an App Domain
		char appDomainName[] = "PlazaAppDomain";
		MonoDomain* newDomain = mono_domain_create_appdomain(appDomainName, nullptr);
		mono_domain_set(newDomain, true);

		Mono::ReloadAppDomain();
		Mono::mAppDomain = newDomain;

		mono_domain_set(Mono::mAppDomain, true);
		/* Copy the script dll */
		if (scriptDllExists) {
			std::string dllPath = (Application::Get()->projectPath + "\\Binaries\\" + std::filesystem::path{ Application::Get()->activeProject->mAssetName }.stem().string() + ".dll");
			std::string newPath = (Application::Get()->projectPath + "\\Binaries\\" + std::filesystem::path{ Application::Get()->activeProject->mAssetName }.stem().string() + "copy.dll");
			std::filesystem::copy_file(dllPath, newPath, filesystem::copy_options::overwrite_existing);
		}

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

		//Scene::GetActiveScene()->RecalculateAddedComponents();
		for (const uint64_t& uuid : SceneView<Collider>(scene)) {
			Collider& collider = *scene->GetComponent<Collider>(uuid);
			collider.lastScale = glm::vec3(0.0f);
			ECS::ColliderSystem::InitCollider(scene, collider.mUuid);
			//collider.UpdateShapeScale(Scene::GetActiveScene()->transformComponents.at(collider.mUuid).GetWorldScale());
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
		//// Init Rigid Bodies
		//for (const uint64_t& uuid : SceneView<Collider>(scene)) {
		//	Collider& collider = *scene->GetComponent<Collider>(uuid);
		//	value.Init();
		//}

		Scripting::LoadProjectCppDll(scene, *Application::Get()->activeProject);

		if (scriptDllExists) {
			Mono::OnStartAll(scene, false);
			//for (auto [key, value] : Scene::GetActiveScene()->csScriptComponents) {
			//	value.Init();
			//}
			FieldManager::ApplyAllScritpsFields(scene, allFields);
			for (const uint64_t& uuid : SceneView<CsScriptComponent>(scene)) {
				CsScriptComponent& csScript = *scene->GetComponent<CsScriptComponent>(uuid);
				for (auto& [className, classScript] : csScript.scriptClasses) {
					Mono::CallMethod(classScript->monoObject, classScript->onStartMethod);
				}
			}
		}

		for (const uint64_t& uuid : SceneView<Collider>(scene)) {
			Collider& collider = *scene->GetComponent<Collider>(uuid);
			ECS::ColliderSystem::UpdateShapeScale(scene, &collider, scene->GetComponent<TransformComponent>(uuid)->GetWorldScale());
			ECS::ColliderSystem::UpdatePose(&collider, scene->GetComponent<TransformComponent>(uuid));
		}

	}

	void Scene::Stop() {
		//mono_jit_cleanup(Mono::mAppDomain);
		/* Clear the queue on the main thread */
		std::lock_guard<std::mutex> lock(Editor::Filewatcher::queueMutex);
		while (!Editor::Filewatcher::taskQueue.empty()) {
			Editor::Filewatcher::taskQueue.pop();
		}


		for (const uint64_t& uuid : SceneView<AudioSource>(Scene::GetActiveScene())) {
			AudioSource& source = *Scene::GetActiveScene()->GetComponent<AudioSource>(uuid);
			source.Stop();
		}

		mono_set_assemblies_path("lib/mono");
		//mono_set_assemblies_path((Application::Get()->editorPath + "/lib/mono").c_str());
		if (Mono::mMonoRootDomain == nullptr)
			Mono::mMonoRootDomain = mono_jit_init("MyScriptRuntime");
		if (Mono::mMonoRootDomain == nullptr)
		{
			// Maybe log some error here
			return;
		}
		// Create an App Domain
		char appDomainName[] = "PlazaAppDomain";
		MonoDomain* newDomain = mono_domain_create_appdomain(appDomainName, nullptr);
		mono_domain_set(newDomain, true);
		Mono::ReloadAppDomain();
		Mono::mAppDomain = newDomain;

		mono_domain_set(Mono::mAppDomain, true);

		// Change active scene, update the selected object scene, delete runtime and set running to false.
		Editor::selectedGameObject = nullptr;
		//delete(Application::Get()->runtimeScene);
		Scene::sRuntimeScene.reset();

		Scene::SetActiveScene(Scene::GetEditorScene());
		Application::Get()->activeCamera = Application::Get()->editorCamera;



		bool scriptDllExists = std::filesystem::exists(Application::Get()->projectPath + "\\Binaries\\" + std::filesystem::path{ Application::Get()->activeProject->mAssetName }.stem().string() + ".dll");
		if (scriptDllExists) {
			Editor::ScriptManager::ReloadScriptsAssembly(Scene::GetActiveScene());
			FieldManager::ApplyAllScritpsFields(Scene::GetActiveScene(), Editor::lastSavedScriptsFields);
		}

		for (const uint64_t& uuid : SceneView<MeshRenderer>(Scene::GetEditorScene())) {
			MeshRenderer& meshRenderer = *Scene::GetEditorScene()->GetComponent<MeshRenderer>(uuid);
			if (meshRenderer.mMaterialsUuids.size() > 0 && meshRenderer.GetMesh()) {
				RenderGroup* newRenderGroup = new RenderGroup(meshRenderer.GetMesh(), meshRenderer.GetMaterials());
				meshRenderer.renderGroup = Scene::GetEditorScene()->AddRenderGroup(newRenderGroup);
			}
		}
	}
	void Scene::Pause() {

	}

	void Scene::RecalculateAddedComponents() {
		for (const uint64_t& uuid : SceneView<MeshRenderer>(this)) {
			MeshRenderer& component = *this->GetComponent<MeshRenderer>(uuid);
			component.renderGroup = this->AddRenderGroup(AssetsManager::GetMesh(component.mMeshUuid), component.GetMaterials());
		}

		this->mainSceneEntity = this->GetEntity(this->mainSceneEntityUuid);
		ECS::TransformSystem::UpdateSelfAndChildrenTransform(*this->GetComponent<TransformComponent>(mainSceneEntityUuid), nullptr, this, true, true);

		//for (auto& [componentUuid, component] : guiComponents) {
		//	for (auto& [key, value] : component.mGuiItems) {
		//		glm::mat4 parentTransform = component.HasGuiItem(value->mGuiParentUuid) ? component.GetGuiItem<GuiItem>(value->mGuiParentUuid)->mTransform : glm::mat4(1.0f);
		//		GuiItem::UpdateSelfAndChildrenTransform(value.get(), parentTransform);
		//	}
		//}
		//entitiesNames = std::unordered_map<std::string, std::unordered_set<uint64_t>>();
		//entitiesNames.reserve(entities.size());
		//for (auto& [key, value] : entities) {
		//	entitiesNames[value.name].insert(key);
		//}
	}

	void Scene::InitializeScenes() {
		sEditorScene = std::make_shared<Scene>();
		sRuntimeScene = std::make_shared<Scene>();
	}

	Scene* Scene::GetEditorScene() {
		return sEditorScene.get();
	}
	void Scene::SetEditorScene(std::shared_ptr<Scene> scene) {
		sEditorScene = scene;
	}
	void Scene::ClearEditorScene() {
		sEditorScene.reset();
		sEditorScene = std::make_shared<Scene>();
	}
	Scene* Scene::GetRuntimeScene() {
		return sRuntimeScene.get();
	}
	Scene* Scene::GetActiveScene() {
		return sActiveScene;
	}
	void Scene::SetActiveScene(Scene* scene) {
		sActiveScene = scene;
	}

	void Scene::Terminate() {
		sActiveScene = nullptr;
		sEditorScene.reset();
		sRuntimeScene.reset();
	}
}