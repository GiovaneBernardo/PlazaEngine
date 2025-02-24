#include "Engine/Core/PreCompiledHeaders.h"

#include "Editor/GUI/guiMain.h"

#include "Editor/GUI/Style/EditorStyle.h"
#include "Engine/Application/Callbacks/CallbacksHeader.h"
#include "Engine/Application/Window.h"
#include "Engine/Components/Core/Camera.h"
#include "Engine/Components/Core/Entity.h"
#include "Engine/Core/Renderer/Renderer.h"
#include "Engine/Core/Renderer/Vulkan/Renderer.h"
#include "Engine/Core/Scene.h"
#include "Engine/Core/Time.h"
#include "Engine/Editor/Editor.h"

#include <cstdlib>
#include <filesystem>

#include "Editor/Filewatcher.h"
#include "Editor/SessionCache/Cache.h"
#include "Engine/Components/Physics/RigidBody.h"
#include "Engine/Core/AssetsManager/Serializer/AssetsSerializer.h"
#include "Engine/Core/Audio/Audio.h"
#include "Engine/Core/Input/Input.h"
#include "Engine/Core/Physics.h"
#include "Engine/Core/Scripting/Scripting.h"
#include "Engine/ECS/ECSManager.h"

/// ---------------------------------------------------------------------

// std::list<Model> models;
#define DEFAULT_GRAPHICAL_API "Vulkan"

namespace Plaza {
Application *Application::sApplication = nullptr;
Application *Application::Get() {
  static Application *sApplication = new Application();
  return sApplication;
}

void Application::Init() {
  PL_CORE_INFO("Start");
  Application::Get();
  Application::Get()->CreateApplication();
  Application::Get()->Loop();
  Application::Get()->Terminate();
}

Plaza::Application::Application() {
  editorCamera = new Plaza::Camera(glm::vec3(0.0f, 0.0f, 5.0f));
  editorCamera->isEditorCamera = true;
  activeCamera = editorCamera;
}

void Application::CreateApplication() {
  PL_CORE_INFO("Creating Application");

  ECS::RegisterComponents();

  this->GetPaths();
  AssetsManager::Init();

  /* Create Renderer */
  switch (Application::Get()->mEditor->mSettings.mDefaultRendererAPI) {
  case RendererAPI::Vulkan:
    Application::Get()->mRenderer = new VulkanRenderer();
    Application::Get()->mRenderer->api =
        Application::Get()->mEditor->mSettings.mDefaultRendererAPI;
    break;
  }

  Application::Get()->mWindow->glfwWindow =
      Application::Get()->mWindow->InitGLFWWindow();

  this->GetAppSize();
#ifdef EDITOR_MODE
  this->CheckEditorCache();
#endif
  if (std::filesystem::exists(Application::Get()->enginePathAppData +
                              "Settings" + Standards::editorSettingsExtName))
    Application::Get()->mSettings =
        *AssetsSerializer::DeSerializeFile<EngineSettings>(
             Application::Get()->enginePathAppData + "Settings" +
                 Standards::editorSettingsExtName,
             Application::Get()->mSettings.mCommonSerializationMode)
             .get();
  this->SetDefaultSettings();
  Scene::InitializeScenes();
  mRenderer->Init();
  Audio::Init();
  Physics::Init();
  this->LoadProject();
}

void Application::GetPaths() {
  char *appdataValue;

#ifdef _WIN32
  appdataValue = std::getenv("APPDATA");
#elif __linux__
  appdataValue = std::getenv("HOME");
#endif
  std::filesystem::path currentPath(__FILE__);
  Application::Get()->dllPath =
      currentPath.parent_path().parent_path().parent_path().string() + "/dll";
  Application::Get()->enginePath =
      currentPath.parent_path().parent_path().string();
  Application::Get()->editorPath =
      currentPath.parent_path().parent_path().parent_path().string() +
      "/Editor";
  Application::Get()->enginePathAppData =
      std::string(appdataValue) + "/PlazaEngine/";

#if defined(_WIN32)
  char buffer[1024];
  GetModuleFileNameA(NULL, buffer, sizeof(buffer));
  Application::Get()->exeDirectory = std::filesystem::path(buffer);
#elif defined(__linux__)
  char buffer[PATH_MAX];
  ssize_t size = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
  if (size == -1)
    throw std::runtime_error("Failed to get executable path");
  buffer[size] = '\0';
  Application::Get()->exeDirectory = std::filesystem::canonical(buffer).string();
  Application::Get()->exeDirectory = std::filesystem::path{Application::Get()->exeDirectory}.parent_path().string();
#else
#error "Unsupported platform"
#endif
}

void Application::GetAppSize() {
#ifdef GAME_MODE
  // Set the scene size to be the entire screen
  int width, height;
  glfwGetWindowSize(Application::Get()->mWindow->glfwWindow, &width, &height);
  Application::Get()->appSizes->sceneSize = glm::vec2(width, height);
#else

#endif
}

/*

                SerializationMode mCommonSerializationMode =
   SerializationMode::SERIALIZE_JSON; SerializationMode
   mMetaDataSerializationMode = SerializationMode::SERIALIZE_JSON;
        SerializationMode mSceneSerializationMode =
   SerializationMode::SERIALIZE_JSON; SerializationMode
   mProjectSerializationMode = SerializationMode::SERIALIZE_JSON;
        SerializationMode mSettingsSerializationMode =
   SerializationMode::SERIALIZE_JSON; SerializationMode mModelSerializationMode
   = SerializationMode::SERIALIZE_BINARY; SerializationMode
   mAnimationSerializationMode = SerializationMode::SERIALIZE_BINARY;
*/

void Application::SetDefaultSettings() {
#ifdef EDITOR_MODE
  Application::GetEditorModeDefaultSettings(this->mSettings);

#elif GAME_MODE
  Application::GetGameModeDefaultSettings(this->mSettings);
#endif
}

void Application::GetEditorModeDefaultSettings(EngineSettings &settings) {
  settings.mCommonSerializationMode = SerializationMode::SERIALIZE_JSON;
  settings.mMetaDataSerializationMode = SerializationMode::SERIALIZE_JSON;
  settings.mSceneSerializationMode = SerializationMode::SERIALIZE_JSON;
  settings.mProjectSerializationMode = SerializationMode::SERIALIZE_JSON;
  settings.mSettingsSerializationMode = SerializationMode::SERIALIZE_JSON;
  settings.mModelSerializationMode = SerializationMode::SERIALIZE_BINARY;
  settings.mPrefabSerializationMode = SerializationMode::SERIALIZE_JSON;
  settings.mAnimationSerializationMode = SerializationMode::SERIALIZE_BINARY;
  settings.mMaterialSerializationMode = SerializationMode::SERIALIZE_JSON;
  settings.mRenderGraphSerializationMode = SerializationMode::SERIALIZE_JSON;
}

void Application::GetGameModeDefaultSettings(EngineSettings &settings) {
  settings.mCommonSerializationMode = SerializationMode::SERIALIZE_BINARY;
  settings.mMetaDataSerializationMode = SerializationMode::SERIALIZE_BINARY;
  settings.mSceneSerializationMode = SerializationMode::SERIALIZE_BINARY;
  settings.mProjectSerializationMode = SerializationMode::SERIALIZE_JSON;
  settings.mSettingsSerializationMode = SerializationMode::SERIALIZE_JSON;
  settings.mModelSerializationMode = SerializationMode::SERIALIZE_BINARY;
  settings.mPrefabSerializationMode = SerializationMode::SERIALIZE_BINARY;
  settings.mAnimationSerializationMode = SerializationMode::SERIALIZE_BINARY;
  settings.mMaterialSerializationMode = SerializationMode::SERIALIZE_BINARY;
  settings.mRenderGraphSerializationMode = SerializationMode::SERIALIZE_BINARY;
}

void Application::CheckEditorCache() {
  /* Check if the engine app data folder doesnt exists, if not, then create */
  if (!std::filesystem::is_directory(Application::Get()->enginePathAppData) &&
      Application::Get()->runningEditor) {
    std::filesystem::create_directory(Application::Get()->enginePathAppData);
    if (!std::filesystem::exists(Application::Get()->enginePathAppData +
                                 "/cache.yaml")) {
    }
  }
}

void Application::LoadProject() {
#ifdef GAME_MODE
  std::cout << "Loading Project \n";
  for (auto const &entry :
       std::filesystem::directory_iterator{Application::Get()->exeDirectory}) {
    if (entry.path().extension() == Standards::projectExtName) {
      Editor::Project::Load(entry.path().string());

      std::cout << "Starting Scene\n";
      Scene::Play();
      std::cout << "Scene Played \n";
      // FIX: Add Camera correctly again
      // if (Scene::GetActiveScene()->cameraComponents.size() > 0)
      //	Application::Get()->activeCamera =
      //&Scene::GetActiveScene()->cameraComponents.begin()->second; else
      //	Application::Get()->activeCamera =
      // Scene::GetActiveScene()->mainSceneEntity->AddComponent<Camera>(new
      // Camera()); Application::Get()->activeCamera =
      // Scene::GetActiveScene()->AddComponent<Camera>(Scene::GetActiveScene()->mainSceneEntity->uuid);
    }
  }

#else
  if (filesystem::exists(Application::Get()->enginePathAppData + "/cache" +
                         Standards::editorCacheExtName))
    Editor::Cache::Load();
  else {
    Application::Get()->runEngine = false;
    Application::Get()->runProjectManagerGui = true;
    Application::Get()->activeProject = std::make_unique<Editor::Project>();
    Editor::Cache::Serialize(Application::Get()->enginePathAppData + "/cache" +
                             Standards::editorCacheExtName);
    Application::Get()->focusedMenu = "ProjectManager";
  }
#endif
}

void Application::UpdateProjectManagerGui() {
  Application::Get()->projectManagerGui->Update();
}

void Application::Loop() {
  PL_CORE_INFO("Starting Loop");
  while (!glfwWindowShouldClose(Application::Get()->mWindow->glfwWindow)) {
    PLAZA_PROFILE_SECTION("Loop");
    // Run the Engine (Update Time, Shadows, Inputs, Buffers, Rendering, etc.)
    if (Application::Get()->runEngine) {
      Application::Get()->UpdateEngine();
    } // Run the Gui for selecting a project
    else if (Application::Get()->runProjectManagerGui) {
      Application::Get()->UpdateProjectManagerGui();
    }

    // GLFW
    {
      PLAZA_PROFILE_SECTION("Swap Buffers");
      glfwSwapBuffers(Application::Get()->mWindow->glfwWindow);
    }
    {
      PLAZA_PROFILE_SECTION("Poll Events");
      glfwPollEvents();
    }
  }
}

void Application::UpdateEngine() {
  PLAZA_PROFILE_SECTION("Update Engine");

  // Update time
  Time::Update();

  // Update Keyboard inputs
  Callbacks::processInput(Application::Get()->mWindow->glfwWindow);
  Input::Update();

  Application::Get()->mThreadsManager->UpdateFrameStartThread();

  // Update Audio Listener
  Audio::UpdateListener(Scene::GetActiveScene());

  // Update Filewatcher main thread
  Editor::Filewatcher::UpdateOnMainThread();

  // Update Animations
  for (auto &[key, value] : Scene::GetActiveScene()->mPlayingAnimations) {
    value->UpdateTime(Time::deltaTime);
  }

  /* Update Scripts */
  if (Scene::GetActiveScene()->mRunning) {
    Scripting::Update(Scene::GetActiveScene());
  }

  /* Update Physics */
  if (Scene::GetActiveScene()->mRunning) {
    PLAZA_PROFILE_SECTION("Update Physics");
    Physics::Advance(Time::deltaTime);
    Physics::Update(Scene::GetActiveScene());
  }

  // Update Camera Position and Rotation
  Application::Get()->activeCamera->Update(Scene::GetActiveScene());

  // Imgui New Frame (only if running editor)
#ifdef EDITOR_MODE
  Editor::Gui::NewFrame();
  Editor::Gui::Update();
#endif

  Time::drawCalls = 0;
  Time::addInstanceCalls = 0;
  Time::mUniqueTriangles = 0;
  Time::mTotalTriangles = 0;

  Application::Get()->mRenderer->Render(Scene::GetActiveScene());

  Application::Get()->mThreadsManager->UpdateFrameEndThread();

  // Update lastSizes
  Application::Get()->lastAppSizes = Application::Get()->appSizes;
  Input::isAnyKeyPressed = false;
  Application::Get()->mThreadsManager->mFrameEndThread->Update();
}

void Application::Terminate() {
  PL_CORE_INFO("Terminate");
  Scene::Terminate();
#ifdef EDITOR_MODE
  Editor::Gui::Delete();
#endif // !GAME_REL
  glfwTerminate();
}
} // namespace Plaza