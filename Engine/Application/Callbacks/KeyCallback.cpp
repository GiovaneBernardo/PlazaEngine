#include "Engine/Core/PreCompiledHeaders.h"
#include "CallbacksHeader.h"
#include "Engine/Application/Serializer/SceneSerializer.h"
#include "Engine/Core/Physics.h"
#include "Editor/DefaultAssets/DefaultAssets.h"

#include "Engine/Core/Skybox.h"
using namespace Plaza;

void ApplicationClass::Callbacks::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (Application->focusedMenu == "Scene") {

		if (key == GLFW_KEY_G && action == GLFW_PRESS)
			Application->Shadows->showDepth = !Application->Shadows->showDepth;

		if (key == GLFW_KEY_R && action == GLFW_PRESS)
			Application->shader = new Shader((Application->enginePath + "\\Shaders\\1.model_loadingVertex.glsl").c_str(), (Application->enginePath + "\\Shaders\\1.model_loadingFragment.glsl").c_str());

		if (key == GLFW_KEY_U && action == GLFW_PRESS)
			Application->activeCamera->Position = Plaza::Editor::selectedGameObject->GetComponent<Transform>()->GetWorldPosition();


		if (glfwGetKey(window, GLFW_KEY_INSERT) == GLFW_PRESS) {
			Application->Shadows->debugLayer += 1;
			if (Application->Shadows->debugLayer > Application->Shadows->shadowCascadeLevels.size()) {
				Application->Shadows->debugLayer = 0;
			}
		}

		if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS) {
			Application->shadowsDepthShader = new Shader((Application->enginePath + "\\Shaders\\shadows\\shadowsDepthVertex.glsl").c_str(), (Application->enginePath + "\\Shaders\\shadows\\shadowsDepthFragment.glsl").c_str(), (Application->enginePath + "\\Shaders\\shadows\\shadowsDepthGeometry.glsl").c_str());
		}

		// Play and Pause
		if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
			if (Application->runningScene)
				Scene::Stop();
			else
				Scene::Play();
		}

		if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
			int size = Application->activeScene->gameObjects.size();
			for (int i = size; i < size + 1; i++) {
				Entity* d = new Entity(std::to_string(Application->activeScene->entities.size()), Application->activeScene->mainSceneEntity);
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
				Mesh cubeMesh = Plaza::Mesh();//Plaza::Mesh::Cube();
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

		if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
			if (Application->activeCamera->isEditorCamera && Application->activeScene->cameraComponents.size() > 0)
				Application->activeCamera = &Application->activeScene->cameraComponents.begin()->second;
			else
				Application->activeCamera = Application->editorCamera;
		}

		if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
			Application->InitShaders();
		}
	}
}