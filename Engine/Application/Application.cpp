#include <glad/glad.h>
#include <glad/glad.c>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/trigonometric.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <stb/stb_image.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <iostream>
#include <random>
#include <unordered_map>
#include <fileSystem>
#include <fileSystem/fileSystem.h>


#include "Engine/Application/Application.h"
#include "Engine/GUI/guiMain.h"

#include "Engine/Application/PickingTexture.h"
#include "Engine/Components/Core/Camera.h"
#include "Engine/Components/Core/Mesh.h"
#include "Engine/Components/Core/GameObject.h"
#include "Engine/Shaders/Shader.h"

#include "Engine/GUI/Style/EditorStyle.h"
#include "Engine/Core/Skybox.h"
#include "Engine/Core/Time.h"
#include "Engine/Core/Renderer.h"
#include "Engine/Editor/Editor.h"
#include "Engine/Application/Window.h"
#include "Engine/Application/Callbacks/CallbacksHeader.h"

using namespace Engine;
/// ---------------------------------------------------------------------
Engine::ApplicationSizes Engine::Application::appSizes;
Engine::ApplicationSizes Engine::Application::lastAppSizes;
Engine::Camera Engine::Application::editorCamera = Engine::Camera::Camera(glm::vec3(0.0f, 0.0f, 5.0f));
Engine::Camera& Engine::Application::activeCamera = Engine::Application::editorCamera;

unsigned int Application::textureColorbuffer = 0;
unsigned int Application::frameBuffer = 0;
unsigned int Application::rbo = 0;

unsigned int Application::blurVAO = 0;
unsigned int Application::blurVBO = 0;

unsigned int Application::verticalBlurColorBuffer = 0;
unsigned int Application::horizontalBlurColorBuffer = 0;

Shader* Application::shader = nullptr;
Shader* Application::pickingShader = nullptr;
Shader* Application::outlineShader = nullptr;
Shader* Application::outlineBlurShader = nullptr;
Shader* Skybox::skyboxShader = nullptr;
GLFWwindow* Application::window = nullptr;

//std::list<Model> models;

EditorStyle editorStyle;

PickingTexture* Application::pickingTexture = nullptr;

using namespace Editor;

void Engine::Application::CreateApplication() {
	// Initialize GLFW (Window)
	Application::window = Application::Window::InitGLFWWindow();
	// Initialize OpenGL, Shaders and Skybox
	InitOpenGL();
	// Initialize Shaders
	Application::shader = new Shader("C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\1.model_loading.vs", "C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\1.model_loading.fs");
	Application::pickingShader = new Shader("C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\1.model_loading.vs", "C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\objectPickingFragment.glsl");
	Application::outlineShader = new Shader("C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\outlining\\outliningVertex.glsl", "C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\outlining\\outliningFragment.glsl");
	Application::outlineBlurShader = new Shader("C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\blur\\blurVertex.glsl", "C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\blur\\blurFragment.glsl");
	Skybox::skyboxShader = new Shader("C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\skybox\\skyboxVertex.glsl", "C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\skybox\\skyboxFragment.glsl");

	InitBlur();


	//Application::InitSkybox();
	Skybox::Init();

	//Initialize ImGui
	Gui::Init(window);
}

void Engine::Application::Loop() {
	while (!glfwWindowShouldClose(Application::window)) {
		// Update time
		Time::Update();
		float currentFrame = static_cast<float>(glfwGetTime());

		// Update Buffers
		if (appSizes.sceneSize != lastAppSizes.sceneSize || appSizes.sceneStart != lastAppSizes.sceneStart) {
			Application::updateBuffers(textureColorbuffer, rbo);
			pickingTexture->updateSize(appSizes.sceneSize);
		}

		// Update inputs
		Callbacks::processInput(window);
		glDisable(GL_BLEND);

		pickingTexture->enableWriting();
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// Render with Picking Shader
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);
		Renderer::Render(*Application::pickingShader);
		pickingTexture->disableWriting();
		glEnable(GL_BLEND);

		// Imgui New Frame
		Gui::NewFrame();

		// Clear buffers
		glBindFramebuffer(GL_FRAMEBUFFER, Application::frameBuffer);
		glClearColor(0.1f, 0.3f, 0.4f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// Draw GameObjects
		Renderer::Render(*Application::shader);



		//  Draw Outline
		if (Editor::selectedGameObject != nullptr)
			Renderer::RenderOutline(*Application::outlineShader);

		// Update Skybox
		//Skybox::Update();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// Update ImGui
		Gui::Update();

		// Update last frame

		Time::lastFrame = currentFrame;

		// GLFW
		glfwSwapBuffers(window);
		glfwPollEvents();

		// Update lastSizes
		Application::lastAppSizes = Application::appSizes;
	}
}

void Engine::Application::Terminate() {
	Skybox::Terminate();
	Gui::Delete();
	glfwTerminate();
}

void Engine::Application::InitBlur() {
	float vertices[] = {
		// positions          // colors           // texture coords
		 1.0f,  1.0, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
		 1.0, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
		-1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
		-1.0f,  1.0, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
	};
	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};

	unsigned int& VAO = Engine::Application::blurVAO;
	unsigned int& VBO = Engine::Application::blurVBO;
	unsigned int EBO = 0;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

}

void Engine::Application::InitOpenGL() {
	ApplicationSizes& appSizes = Engine::Application::appSizes;

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	// Init some starting shaders
	Engine::Application::shader = new Shader("C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\1.model_loading.vs", "C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\1.model_loading.fs");
	Engine::Application::pickingShader = new Shader("C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\1.model_loading.vs", "C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\objectPickingFragment.glsl");
	Engine::Application::outlineShader = new Shader("C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\outlining\\outliningVertex.glsl", "C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\outlining\\outliningFragment.glsl");
	Engine::Application::outlineBlurShader = new Shader("C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\blur\\blurVertex.glsl", "C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Shaders\\blur\\blurFragment.glsl");

	// Create buffers
#pragma region Framebuffer
	unsigned int& frameBuffer = Application::frameBuffer;
	//unsigned int& textureColorbuffer = Application::textureColorbuffer;
	glGenFramebuffers(1, &Application::frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, Application::frameBuffer);
	// create a color attachment texture
	glGenTextures(1, &Application::textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, Application::textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, appSizes.sceneSize.x, appSizes.sceneSize.y, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Application::textureColorbuffer, 0);

	// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
	glGenRenderbuffers(1, &Application::rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, Application::rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, appSizes.sceneSize.x, appSizes.sceneSize.y); // use a single renderbuffer object for both a depth AND stencil buffer.
	glViewport(0, 0, appSizes.sceneSize.x, appSizes.sceneSize.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, Application::rbo); // now actually attach it

	GLenum attachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_RED_INTEGER };
	glDrawBuffers(3, attachments);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, appSizes.sceneSize.x, appSizes.sceneSize.y);

	pickingTexture = new PickingTexture();
	pickingTexture->init(appSizes.sceneSize.x, appSizes.sceneSize.y);
	glViewport(0, 0, appSizes.sceneSize.x, appSizes.sceneSize.y);
#pragma endregion

	Engine::Application::shader->use();
	Engine::Application::shader->setInt("texture1", 0);
}




/* Updates ? ------------------------------------------------------------------- MUST REVIEW IT*/
void Engine::Application::updateBuffers(GLuint textureColorBuffer, GLuint rbo) {
	{
		ApplicationSizes& appSizes = Engine::Application::appSizes;
		glBindTexture(GL_TEXTURE_2D, Engine::Application::textureColorbuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, appSizes.sceneSize.x, appSizes.sceneSize.y, 0, GL_RGB, GL_FLOAT, nullptr);

		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, appSizes.sceneSize.x, appSizes.sceneSize.y);

		glViewport(appSizes.sceneStart.x, appSizes.sceneStart.y, appSizes.sceneSize.x, appSizes.sceneSize.y);
	}
}
