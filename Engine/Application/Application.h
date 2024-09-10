#pragma once
//#include "Engine/Core/PreCompiledHeaders.h"
#include "Engine/Application/ApplicationSizes.h"
#include "Engine/Components/Core/Camera.h"
#include "Engine/Application/Application.h"

#include "Engine/Application/EditorCamera.h"
#include "Engine/Shaders/Shader.h"
#include "Editor/GUI/guiMain.h"
#include "Engine/Application/Window.h"
#include "Engine/Editor/Editor.h"
#include "Editor/GUI/ProjectManager/ProjectManager.h"
//#include "Engine/Application/Callbacks/CallbacksHeader.h"
#include "Engine/Core/Scene.h"
#include "Editor/Project.h"
#include "Engine/Core/Engine.h"
#include "Engine/Core/FrameBuffer.h"

#include "Engine/Core/Renderer/Renderer.h"
#include "Engine/Core/Renderer/Vulkan/Renderer.h"
#include "Engine/Core/AssetsManager/AssetsManager.h"
#include "Engine/Threads/ThreadManager.h"
#include "Editor/Editor.h"

namespace Plaza {
	class Camera;
	class Window;
	class Application {
	public:
		void CreateApplication();
		void GetPaths();
		void GetAppSize();
		void CheckEditorCache();
		void LoadProject();

		void UpdateEngine();
		void UpdateProjectManagerGui();

		void Loop();
		void Terminate();

		AssetsManager* mAssetsManager;
		Renderer* mRenderer = nullptr;
		RendererAPI mRendererAPI;
		ThreadsManager* mThreadsManager = new ThreadsManager();
		bool showCascadeLevels = false;

		Scene* editorScene = new Scene();
		Scene* runtimeScene = new Scene();
		Scene* activeScene = editorScene;
		bool runningScene = false;
		int drawCalls = 0;
		bool runningEditor = true;

		std::string exeDirectory;
		std::string projectPath;
		std::string dllPath;
		std::string enginePath;
		std::string editorPath;
		std::string enginePathAppData;
		Editor::Project* activeProject = nullptr;

		std::string focusedMenu = "Scene";
		std::string hoveredMenu = "Scene";

		bool copyingScene = false;

		Camera* editorCamera;
		Camera* activeCamera;
		Application();
		ApplicationSizes* appSizes = new ApplicationSizes();
		ApplicationSizes* lastAppSizes = appSizes;
		Plaza::Editor::ProjectManagerGui* projectManagerGui = new Plaza::Editor::ProjectManagerGui();

		bool runProjectManagerGui = true;
		bool runEngine = true;

		EngineClass* engine = new EngineClass();
		Window* mWindow = new Window();
		Editor::EditorClass* mEditor = new Editor::EditorClass();

		static void Init() {
			sApplication = new Application();
		}
		static Application* Get() {
			return sApplication;
		}

	private:
		static Application* sApplication;
	};
}
