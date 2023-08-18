#include "Engine/Core/PreCompiledHeaders.h"
#include "CallbacksHeader.h"
#include "Engine/Application/Serializer/SceneSerializer.h"
#include "Engine/Core/Physics.h"
#include "Editor/DefaultAssets/DefaultAssets.h"
using namespace Engine;

void ApplicationClass::Callbacks::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (Application->focusedMenu == "Scene") {

		if (key == GLFW_KEY_G && action == GLFW_PRESS)
			Application->Shadows->showDepth = !Application->Shadows->showDepth;

		if (key == GLFW_KEY_R && action == GLFW_PRESS)
			Application->shader = new Shader((Application->enginePath + "\\Shaders\\1.model_loadingVertex.glsl").c_str(), (Application->enginePath + "\\Shaders\\1.model_loadingFragment.glsl").c_str());

		if (key == GLFW_KEY_U && action == GLFW_PRESS)
			Application->activeCamera->Position = Engine::Editor::selectedGameObject->GetComponent<Transform>()->worldPosition;


		if (glfwGetKey(window, GLFW_KEY_INSERT) == GLFW_PRESS) {
			Application->Shadows->debugLayer += 1;
			if (Application->Shadows->debugLayer > Application->Shadows->shadowCascadeLevels.size()) {
				Application->Shadows->debugLayer = 0;
			}
		}

		if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS) {
			Application->shadowsDepthShader = new Shader((Application->enginePath + "\\Shaders\\shadows\\shadowsDepthVertex.glsl").c_str(), (Application->enginePath + "\\Shaders\\shadows\\shadowsDepthFragment.glsl").c_str(), (Application->enginePath + "\\Shaders\\shadows\\shadowsDepthGeometry.glsl").c_str());
		}

		if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
			// Stop
			if (Application->runningScene) {
				Editor::selectedGameObject = nullptr;
				//if (Editor::selectedGameObject)
				//	Engine::Editor::Gui::changeSelectedGameObject(Application->editorScene->gameObjects.find(Editor::selectedGameObject->uuid));
				// Change active scene, update the selected object scene, delete runtime and set running to false.
				delete(Application->runtimeScene);
				Application->runningScene = false;
				Application->activeScene = Application->editorScene;
			} // Play
			else {
				// Create a new empty Scene, change active scene to runtime, copy the contents of editor scene into runtime scene and update the selected object scene
				Application->runtimeScene = new Scene();
				Application->runtimeScene = Scene::Copy(Application->runtimeScene, Application->editorScene);
				Application->activeScene = Application->runtimeScene;
				//if (Editor::selectedGameObject)
				//	Engine::Editor::Gui::changeSelectedGameObject(Application->activeScene->gameObjects.find(Editor::selectedGameObject->name));

				Application->runningScene = true;
			}
		}

		if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
			int size = Application->activeScene->gameObjects.size();
			for (int i = size; i < size + 1; i++) {
				GameObject* d = new GameObject(std::to_string(Application->activeScene->entities.size()), Application->activeScene->mainSceneEntity);
				//d->AddComponent(new Transform());



				std::random_device rd;
				std::mt19937 gen(rd());

				// Define the range for the random numbers (-20 to 20)
				int min = -20;
				int max = 20;
				std::uniform_int_distribution<int> distribution(min, max);
				Transform& test = *d->GetComponent<Transform>();
				d->GetComponent<Transform>()->relativePosition = glm::vec3(distribution(gen), distribution(gen), distribution(gen)) + Application->activeCamera->Position;
				d->GetComponent<Transform>()->UpdateChildrenTransform();
				Mesh cubeMesh = Engine::Mesh();//Engine::Mesh::Cube();
				cubeMesh.material.diffuse.rgba = glm::vec4(0.8f, 0.3f, 0.3f, 1.0f);
				cubeMesh.material.specular = Texture();
				cubeMesh.material.specular.rgba = glm::vec4(0.8f, 0.3f, 0.3f, 1.0f);
				MeshRenderer* meshRenderer = new MeshRenderer(cubeMesh);
				meshRenderer->instanced = true;
				//meshRenderer->mesh = std::make_unique<Mesh>(cubeMesh);
				//Editor::DefaultModels::Init();
				meshRenderer->mesh = Editor::DefaultModels::Cube();
				//MeshSerializer::Serialize(Application->activeProject->directory + "\\teste.yaml", *cubeMesh);
				d->AddComponent<MeshRenderer>(meshRenderer);
				RigidBody* rigidBody = new RigidBody(d->uuid, Application->runningScene);
				rigidBody->uuid = d->uuid;
				d->AddComponent<RigidBody>(rigidBody);
			}
		}


		if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
			Application->shader = new Shader((Application->enginePath + "\\Shaders\\1.model_loadingVertex.glsl").c_str(), (Application->enginePath + "\\Shaders\\1.model_loadingFragment.glsl").c_str());
			Application->pickingShader = new Shader((Application->enginePath + "\\Shaders\\picking\\pickingVertex.glsl").c_str(), (Application->enginePath + "\\Shaders\\picking\\pickingFragment.glsl").c_str());
			Application->outlineShader = new Shader((Application->enginePath + "\\Shaders\\outlining\\outliningVertex.glsl").c_str(), (Application->enginePath + "\\Shaders\\outlining\\outliningFragment.glsl").c_str());
			Application->outlineBlurShader = new Shader((Application->enginePath + "\\Shaders\\blur\\blurVertex.glsl").c_str(), (Application->enginePath + "\\Shaders\\blur\\blurFragment.glsl").c_str());
			Application->combiningShader = new Shader((Application->enginePath + "\\Shaders\\combining\\combiningVertex.glsl").c_str(), (Application->enginePath + "\\Shaders\\combining\\combiningFragment.glsl").c_str());
			Application->edgeDetectionShader = new Shader((Application->enginePath + "\\Shaders\\edgeDetection\\edgeDetectionVertex.glsl").c_str(), (Application->enginePath + "\\Shaders\\edgeDetection\\edgeDetectionFragment.glsl").c_str());
			Application->singleColorShader = new Shader((Application->enginePath + "\\Shaders\\singleColor\\singleColorVertex.glsl").c_str(), (Application->enginePath + "\\Shaders\\singleColor\\singleColorFragment.glsl").c_str());
			Application->shadowsDepthShader = new Shader((Application->enginePath + "\\Shaders\\shadows\\shadowsDepthVertex.glsl").c_str(), (Application->enginePath + "\\Shaders\\shadows\\shadowsDepthFragment.glsl").c_str(), (Application->enginePath + "\\Shaders\\shadows\\shadowsDepthGeometry.glsl").c_str());
			Application->debugDepthShader = new Shader((Application->enginePath + "\\Shaders\\debug\\debugDepthVertex.glsl").c_str(), (Application->enginePath + "\\Shaders\\debug\\debugDepthFragment.glsl").c_str());
			Application->hdrShader = new Shader((Application->enginePath + "\\Shaders\\hdr\\hdrVertex.glsl").c_str(), (Application->enginePath + "\\Shaders\\hdr\\hdrFragment.glsl").c_str());
		}
	}
}