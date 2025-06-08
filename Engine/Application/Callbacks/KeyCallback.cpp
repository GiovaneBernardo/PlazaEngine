#include "Engine/Core/PreCompiledHeaders.h"
#include "CallbacksHeader.h"
#include "Engine/Core/Physics.h"
#include "Editor/DefaultAssets/DefaultAssets.h"

#include "Editor/GUI/guiMain.h"
#include "Engine/Core/Input/Input.h"
#include "Engine/Application/FileDialog/FileDialog.h"
#include "Engine/Core/Scene.h"
#include "Editor/GUI/NodeEditors/RenderGraphEditor.h"
#include "Editor/ScriptManager/ScriptManager.h"
#include "Engine/Core/AssetsManager/Serializer/AssetsSerializer.h"
#include "Engine/ECS/ECSManager.h"
#include "Engine/stb_image.h"

uint64_t lastUuid;

static unsigned int cascadeIndexDebug = 2;

glm::vec3 randomVec3() {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_real_distribution<float> dis(-1000.0f, 1000.0f);

	return glm::vec3(dis(gen), dis(gen), dis(gen));
}

Plaza::Entity* NewEntity(std::string name, Plaza::Entity* parent, Plaza::Mesh* mesh, bool instanced = true,
						 bool addToScene = true, Plaza::Scene* scene = nullptr) {
	Plaza::Entity* obj = scene->NewEntity(name, parent); // new Entity(name, parent, addToScene);
	Plaza::ECS::TransformSystem::UpdateSelfAndChildrenTransform(
		*scene->GetComponent<Plaza::TransformComponent>(obj->uuid), nullptr, scene, true);
	Plaza::MeshRenderer* meshRenderer = scene->NewComponent<Plaza::MeshRenderer>(
		obj->uuid); // new MeshRenderer(mesh, { AssetsManager::GetDefaultMaterial() }, true);
	meshRenderer->ChangeMesh(mesh);
	meshRenderer->AddMaterial(Plaza::AssetsManager::GetDefaultMaterial());
	meshRenderer->instanced = true;

	// meshRenderer->mesh = new Mesh(*mesh);
	// meshRenderer->mMaterials.push_back(AssetsManager::GetDefaultMaterial());
	// RenderGroup* newRenderGroup = new RenderGroup(meshRenderer->mesh, meshRenderer->mMaterials);
	// meshRenderer->renderGroup = Scene::GetActiveScene()->AddRenderGroup(newRenderGroup);
	// meshRenderer->renderGroup->material = make_shared<Material>(*AssetsManager::GetDefaultMaterial());
	Plaza::Editor::selectedGameObject = obj;

	return obj;
}
std::vector<unsigned char> read_fileg(const std::string& filename) {
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file: " + filename);
	}

	std::streamsize file_size = file.tellg();
	if (file_size <= 0) {
		throw std::runtime_error("File is empty or size could not be determined: " + filename);
	}

	std::vector<unsigned char> buffer(static_cast<size_t>(file_size));
	file.seekg(0, std::ios::beg);
	file.read(reinterpret_cast<char*>(buffer.data()), file_size);

	if (!file) {
		throw std::runtime_error("Failed to read file: " + filename);
	}

	return buffer;
}

namespace Plaza {
	void Callbacks::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		// FIX: Get a proper scene instead of the active scene
		Scene* scene = Scene::GetActiveScene();
		for (CallbackFunction callbackFunction : sOnKeyPressFunctions) {
			bool isLayerFocused = Editor::Gui::sFocusedLayer == callbackFunction.layerToExecute;
			if (isLayerFocused)
				callbackFunction.function;
		}

		for (auto& tool : Editor::Gui::sEditorTools) {
			tool.second->OnKeyPress(key, scancode, action, mods);
		}

		if (Application::Get()->focusedMenu == "Scene" || Application::Get()->focusedMenu == "Editor") {
			if (key == GLFW_KEY_F5 && action == GLFW_PRESS) {
				Editor::ScriptManager::ReloadScriptsAssembly(scene);
			}
		}

		if (Application::Get()->focusedMenu == "Scene")
			Input::isAnyKeyPressed = true;
		if (Application::Get()->focusedMenu == "Editor") {
			if (key == GLFW_KEY_K && action == GLFW_PRESS) {
				for (int i = 0; i < 100; ++i) {
					Application::Get()->mRenderer->UpdateMainProgressBar(i / 100.0f);
					std::this_thread::sleep_for(std::chrono::milliseconds(3));
				}
			}

			if (key == GLFW_KEY_F && action == GLFW_PRESS) {
				if (Editor::selectedGameObject) {
					uint64_t newUuid =
						ECS::EntitySystem::Instantiate(Scene::GetActiveScene(), Editor::selectedGameObject->uuid);
					if (newUuid)
						ECS::TransformSystem::UpdateSelfAndChildrenTransform(
							*scene->GetComponent<TransformComponent>(newUuid), nullptr, scene);
					Editor::selectedGameObject = Scene::GetActiveScene()->GetEntity(newUuid);
				}
			}

			if (key == GLFW_KEY_T && action == GLFW_PRESS) {
				for (int i = 0; i < 32000; ++i) {
					Entity* entity = NewEntity("Cube", nullptr, Editor::DefaultModels::Cube(), true, true, scene);
					TransformComponent* transform = scene->GetComponent<TransformComponent>(entity->uuid);
					Plaza::ECS::TransformSystem::SetLocalPosition(*transform, scene, randomVec3());
					Plaza::Collider* collider = scene->NewComponent<Plaza::Collider>(entity->uuid);
					Plaza::ECS::ColliderSystem::CreateShape(collider, transform,
															Plaza::ColliderShape::ColliderShapeEnum::BOX);
				}
			}

			VulkanRenderer* vulkanRenderer = (VulkanRenderer*)Application::Get()->mRenderer;
			if (key == GLFW_KEY_H && action == GLFW_PRESS) {
				int width = 0;
				int height = 0;

				GLFWimage images[1] = {GLFWimage()};
				// images[0].pixels = static_cast<unsigned char*>(stbi_load(std::string(Application::Get()->editorPath +
				// "/Images/Other/PlazaEngineLogo.png").c_str(), &images[0].width, &images[0].height, &channels, 4));
				// //rgba channels

				int channels = 0;

				auto datae = read_fileg(
					std::string(FilesManager::sEditorFolder.string() + "/Images/Other/PlazaEngineLogo.png").c_str());

				// images[0].pixels = static_cast<unsigned char*>(stbi_load_from_memory(datae.data(), datae.size(),
				// &width, &height, &channels, 4));
				images[0].pixels =
					stbi_load(std::string(FilesManager::sEditorFolder.string() + "/Images/Other/PlazaEngineLogo.png").c_str(),
							  &width, &height, &channels, 4);

				// images[0].pixels = static_cast<unsigned char*>(data);
				images[0].width = width;
				images[0].height = height;

				glfwSetWindowIcon(window, 1, images);
			}
			if (key == GLFW_KEY_L && action == GLFW_PRESS) {
				cascadeIndexDebug++;
				if (cascadeIndexDebug > 8)
					cascadeIndexDebug = 0;
			}
			if (key == GLFW_KEY_G && action == GLFW_PRESS)
				Application::Get()->showCascadeLevels = !Application::Get()->showCascadeLevels;

			if (key == GLFW_KEY_N && action == GLFW_PRESS) {
				VulkanRenderGraph* graph =
					new VulkanRenderGraph(*AssetsSerializer::DeSerializeFile<VulkanRenderGraph>(
											   FileDialog::OpenFileDialog(Standards::plazaRenderGraph.c_str()),
											   Application::Get()->mSettings.mRenderGraphSerializationMode)
											   .get());
				Application::Get()->mEditor->mGui.mRenderGraphEditor->LoadRenderGraphNodes(graph);
			}
			// if (key == GLFW_KEY_G && action == GLFW_PRESS)
			//	Scene::GetActiveScene()->entities[Editor::selectedGameObject->uuid].RemoveComponent<RigidBody>();

			if (key == GLFW_KEY_U && action == GLFW_PRESS)
				Application::Get()->activeCamera->Position =
					scene->GetComponent<TransformComponent>(Plaza::Editor::selectedGameObject->uuid)
						->GetWorldPosition();

			if (key == GLFW_KEY_END && action == GLFW_PRESS) {
			}
			if (key == GLFW_KEY_HOME && action == GLFW_PRESS) {
			}

			// Play and Pause
			if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
				if (scene->mRunning)
					Scene::Stop();
				else
					Scene::Play();
			}
		}

		if (Application::Get()->focusedMenu == "Editor" || Application::Get()->focusedMenu == "Hierarchy") {
			if (glfwGetKey(window, GLFW_KEY_DELETE) == GLFW_PRESS && Editor::selectedGameObject) {
				uint64_t uuid = Editor::selectedGameObject->uuid;
				ECS::EntitySystem::Delete(scene, Editor::selectedGameObject->uuid);
				Editor::selectedGameObject = nullptr;
			}
		}

		if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
			Editor::Gui::sceneViewUsingEditorCamera = !Editor::Gui::sceneViewUsingEditorCamera;
		}

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && Application::Get()->focusedMenu == "Scene") {
#ifdef EDITOR_MODE
			ImGui::SetWindowFocus("Editor");
#endif
		}

#ifdef EDITOR_MODE
		if (glfwGetKey(window, GLFW_KEY_PAGE_UP))
			glfwSetInputMode(Application::Get()->mWindow->glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		else if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN))
			glfwSetInputMode(Application::Get()->mWindow->glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
#endif
	}
} // namespace Plaza
