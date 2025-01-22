#pragma once
#include "Engine/Application/ApplicationSizes.h"
#include "Engine/Components/Core/Camera.h"

#include "Engine/Application/EditorCamera.h"
#include "Engine/Application/Window.h"
#include "Engine/Editor/Editor.h"
#include "Editor/GUI/ProjectManager/ProjectManager.h"
#include "Editor/Project.h"
#include "Engine/Core/Engine.h"

#include "Engine/Core/AssetsManager/AssetsManager.h"
#include "Engine/Threads/ThreadManager.h"
#include "Editor/Editor.h"
#include "EngineSettings.h"

namespace Plaza {
	class Camera;
	class Window;
	class Renderer;
	class PLAZA_API Application {
	public:
		void CreateApplication();
		void GetPaths();
		void GetAppSize();
		void SetDefaultSettings();
		static void GetEditorModeDefaultSettings(EngineSettings& settings);
		static void GetGameModeDefaultSettings(EngineSettings& settings);
		void CheckEditorCache();
		void LoadProject();

		void UpdateEngine();
		void UpdateProjectManagerGui();

		void Loop();
		void Terminate();

		EngineSettings mSettings = EngineSettings();
		AssetsManager* mAssetsManager;
		Renderer* mRenderer = nullptr;
		RendererAPI mRendererAPI;
		ThreadsManager* mThreadsManager = new ThreadsManager();
		bool showCascadeLevels = false;

		Scene* editorScene = nullptr;
		Scene* runtimeScene = nullptr;
		Scene* activeScene = nullptr;
		int drawCalls = 0;
		bool runningEditor = true;
		int mComponentCounter = 0;

		std::string exeDirectory;
		std::string projectPath;
		std::string dllPath;
		std::string enginePath;
		std::string editorPath;
		std::string enginePathAppData;
		std::unique_ptr<Editor::Project> activeProject = nullptr;

		std::string focusedMenu = "Scene";
		std::string hoveredMenu = "Scene";

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

		Editor::EditorClass* GetEditor() { return mEditor; }
		Editor::EditorClass* mEditor = new Editor::EditorClass();

		static void Init();
		static Application* Get();

	private:
		static Application* sApplication;
	};
}
