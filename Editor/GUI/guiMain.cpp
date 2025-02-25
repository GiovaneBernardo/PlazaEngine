#include "Engine/Core/PreCompiledHeaders.h"

#include "guiMain.h"
#include "Editor/GUI/Utils/DataVisualizer.h"


#include "Engine/Components/Core/Entity.h"
#include "Editor/GUI/Hierarchy/Hierarchy.h"
#include "Editor/GUI/Inspector.h"
#include "Editor/GUI/gizmo.h"
#include "Editor/GUI/TransformOverlay.h"
#include "Engine/Components/Core/Entity.h"
#include "Engine/Application/ApplicationSizes.h"
#include "Engine/Editor/Editor.h"
//#include "EntryPoint.h"
#include "Editor/GUI/Inspector.h"
#include "Editor/GUI/FpsCounter.h"
#include "Editor/GUI/MenuBar.h"
#include "Editor/GUI/FileExplorer/FileExplorer.h"

#include "Editor/GUI/Hierarchy/HierarchyPopup.h"
#include "Editor/GUI/Utils/ImageLoader.h"
#include "Editor/ScriptManager/ScriptManager.h"
#include "Engine/Core/Input/Input.h"
#include "Engine/Core/Input/Cursor.h"

#include <ThirdParty/imgui/imgui_impl_opengl3.h>
#include "Engine/Core/AssetsManager/AssetsManager.h"
#include "Engine/Core/AssetsManager/Loader/AssetsLoader.h"
#include "Editor/DragDrop/DropFileScene.h"
#include "Editor/EditorTools/TerrainEditorTool.h"

#include "Editor/GUI/Hierarchy/Hierarchy.h"
#include "Editor/GUI/AssetsImporterWindow/AssetsImporterWindow.h"
#include "Editor/GUI/Console/Console.h"
#include "Editor/GUI/NodeEditors/RenderGraphEditor.h"
#include "Editor/GUI/Style/EditorStyle.h"

////   #include "ThirdParty/imgui/imgui_impl_vulkan.h"
//#include "Engine/Application/Application.h" //

//
//bool ImGuizmo::IsDrawing = false;


//using namespace Editor;

glm::vec2 curHierarchySize;
glm::vec2 curSceneSize;
glm::vec2 curInspectorSize;

bool Plaza::Editor::Gui::sceneViewUsingEditorCamera = false;
bool Plaza::Editor::Gui::isHierarchyOpen = true;
bool Plaza::Editor::Gui::isSceneOpen = true;
bool Plaza::Editor::Gui::isInspectorOpen = true;
bool Plaza::Editor::Gui::isFileExplorerOpen = true;
bool Plaza::Editor::Gui::canUpdateContent = false;

bool windowVisible = true;
// Update ImGui Windows
namespace Plaza {
	namespace Editor {
		FpsCounter* fpsCounter;
		class Hierarchy;
		string Gui::scenePayloadName = "scenePayloadName";

		ImTextureID Gui::playPauseButtonImageId;
		void Gui::Update() {
			PLAZA_PROFILE_SECTION("ImGui Update");
			ImGuiIO& io = ImGui::GetIO();
			io.DeltaTime = Time::deltaTime;
			io.DisplaySize = ImVec2(Application::Get()->appSizes->appSize.x, Application::Get()->appSizes->appSize.y);
			Input::SetFocusedMenuCheck("Editor");
			Gui::setupDockspace(Scene::GetActiveScene(), Application::Get()->mWindow->glfwWindow, Application::Get()->activeCamera);
			//else if (Application::Get()->mRenderer->api == RendererAPI::Vulkan)
			//	Application::Get()->mRenderer->UpdateGUI();
			//ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), (VulkanRenderer)(Application::Get()->mRenderer)->);
		}

		void Gui::NewFrame() {
			PLAZA_PROFILE_SECTION("ImGui New Frame");
			if (Application::Get()->mRenderer->api == RendererAPI::Vulkan) {
				Application::Get()->mRenderer->NewFrameGUI();
			}
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
		}

		void Gui::Init(GLFWwindow* window) {
			sEditorStyle = new EditorStyle();

			mMainContext = ImGui::CreateContext();
			mMainProgressBarContext = ImGui::CreateContext();

			ImGui::SetCurrentContext(mMainContext);
			CommonGuiInit(window, sEditorStyle, true);

			Icon::Init();
			FpsCounter* fpsCounter = new FpsCounter();
			// Load Icons
			playPauseButtonImageId = Utils::LoadImageToImGuiTexture(std::string(Application::Get()->editorPath + "/Images/Other/playPauseButton.png").c_str());
			sEditorTools.emplace(EditorTool::ToolType::TERRAIN_EDITOR, std::make_unique<TerrainEditorTool>());

			ImGui::SetCurrentContext(mMainProgressBarContext);
			CommonGuiInit(window, sEditorStyle, false);
			mMainProgressBarContext->IO = mMainContext->IO;
			mMainProgressBarContext->PlatformIO = mMainContext->PlatformIO;

			Application::Get()->mEditor->mGui.Gui::mHierarchy = new HierarchyWindow(GuiLayer::HIERARCHY, true);
			Application::Get()->mEditor->mGui.Gui::mAssetsImporter = new AssetsImporterWindow(GuiLayer::ASSETS_IMPORTER, false);
			Application::Get()->mEditor->mGui.Gui::mConsole = new Console(GuiLayer::CONSOLE, false);
			Application::Get()->mEditor->mGui.Gui::mRenderGraphEditor = new RenderGraphEditor(GuiLayer::RENDER_GRAPH_EDITOR, false);

			Application::Get()->mEditor->mGui.mRenderGraphEditor->Init();

			ImGui::SetCurrentContext(mMainContext);
		}

		void Gui::CommonGuiInit(GLFWwindow* window, EditorStyle* editorStyle, bool initializePlazaImGuiPools) {
			ImGui::StyleColorsDark();
			ImGuiIO& io = ImGui::GetIO();
			(void)io;

			const char* iniFilePath = new char[(Application::Get()->enginePath + "/imgui.ini").size() + 1];
			strcpy(const_cast<char*>(iniFilePath), (Application::Get()->enginePath + "/imgui.ini").c_str());
			ImGui::GetIO().IniFilename = iniFilePath;
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
			if (Application::Get()->mRenderer->api == RendererAPI::Vulkan)
			{
				if (initializePlazaImGuiPools) {
					ImGui_ImplGlfw_InitForVulkan(window, true);
					Application::Get()->mRenderer->InitGUI();
				}
				else {
					ImGui_ImplGlfw_InitForVulkan(window, false);
					VulkanRenderer::GetRenderer()->InitVulkanImGui();
				}
			}
			//ImGui_ImplGlfw_InitForVulkan(Application::Get()->Window->glfwWindow, true);
		//C:/Users/Giovane/Desktop/Workspace 2023/OpenGL/OpenGLEngine/Engine/Font/Poppins-Regular.ttf
			io.Fonts->AddFontFromFileTTF((Application::Get()->enginePath + "/Font/Poppins-Regular.ttf").c_str(), 18);
			io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts | ImGuiConfigFlags_DpiEnableScaleViewports;

#pragma region ImGui Style
			auto& colors = ImGui::GetStyle().Colors;
			colors[ImGuiCol_WindowBg] = editorStyle->backgroundColor;
			colors[ImGuiCol_Header] = editorStyle->CollapsingHeaderBackgroundColor;
			colors[ImGuiCol_HeaderHovered] = editorStyle->treeNodeHoverBackgroundColor;
			colors[ImGuiCol_HeaderActive] = editorStyle->treeNodeActiveBackgroundColor;
			colors[ImGuiCol_TabActive] = editorStyle->tabActiveColor;
			colors[ImGuiCol_Tab] = ImVec4(0.295f, 0.295f, 0.295f, 1.0f);
			colors[ImGuiCol_TitleBgActive] = ImVec4(0.255f, 0.255f, 0.255f, 1.0f);
			colors[ImGuiCol_TabUnfocused] = ImVec4(0.10f, 0.10f, 0.10f, 1.0f);
			colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.33f, 0.33f, 0.33f, 1.0f);
			colors[ImGuiCol_FrameBg] = ImVec4(0.30f, 0.30f, 0.30f, 1.0f);
			colors[ImGuiCol_Button] = ImVec4(0.30f, 0.30f, 0.30f, 1.0f);

			ImGui::GetStyle().FrameRounding = 0.0f;
			ImGui::GetStyle().TabRounding = 0.0f;
			ImGui::GetStyle().WindowPadding = ImVec2(0.0f, 0.0f);
			ImGui::GetStyle().FramePadding = ImVec2(2.0f, 2.0f);
			//ImGui::GetStyle().FrameBorderSize = 2.0f;
			ImGui::GetStyle().WindowMenuButtonPosition = ImGuiDir_::ImGuiDir_Right;
#pragma endregion

			ImGui::GetStyle().ScrollbarSize = 9.0f;
		}

		void Gui::Delete() {
			// Clean up ImGui
			//ImGui_ImplGlfw_Shutdown();
			//ImGui::DestroyContext();

		}

		void Gui::changeSelectedGameObject(Entity* newSelectedGameObject) {
			selectedGameObject = newSelectedGameObject;
		}
		void Gui::setupDockspace(Scene* scene, GLFWwindow* window, Camera* camera) {
			PLAZA_PROFILE_SECTION("Setup Dockspace");
			Gui::sRenderingTextureViewer = false;

			ApplicationSizes& appSizes = *Application::Get()->appSizes;
			ApplicationSizes& lastAppSizes = *Application::Get()->lastAppSizes;

			//	ImGuiWindowFlags  windowFlags = ImGuiWindowFlags_MenuBar;

			ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar;
			windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
			ImGui::SetNextWindowSize(ImGui::imVec2(appSizes.appSize));
			//			windowFlags |= ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoTitleBar;
				//		windowFlags |= ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiConfigFlags_DockingEnable | ImGuiWindowFlags_NoScrollbar;

			bool showDockspace = true;
			ImGui::Begin("Main DockSpace", &showDockspace, windowFlags);

			ImGui::GetIO().MouseDelta.x = Input::Cursor::deltaX;
			ImGui::GetIO().MouseDelta.y = Input::Cursor::deltaY;

			//ImGui::SetWindowPos(ImVec2(0, 0));

			// Submit the DockSpace
			ImGuiIO& io = ImGui::GetIO();
			ImGuiID dockspace_id = ImGui::GetID("Main DockSpace");
			ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
			dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
			dockspace_flags |= ImGuiDockNodeFlags_NoDockingInCentralNode;

			// Submit the DockSpace
			ImGui::DockSpace(dockspace_id, ImVec2(0, 0), dockspace_flags);
			Gui::MainMenuBar::Begin();

			//Gui::beginHierarchyView(gameFrameBuffer);
			Application::Get()->mEditor->mGui.mHierarchy->mScene = Scene::GetActiveScene();
			Application::Get()->mEditor->mGui.mHierarchy->Update(Scene::GetActiveScene());

			Gui::beginScene(scene, *Application::Get()->activeCamera);

			Gui::beginAssetsViewer(scene, *Application::Get()->activeCamera);

			Gui::beginProfiler(scene);

			Gui::beginInspector(scene, *camera);

			Gui::beginImageInspector(scene, *camera);

			//ImGui::ShowDemoWindow();

			FileExplorer::UpdateGui();
			Application::Get()->mEditor->mGui.mConsole->Update(scene);
			//Application::Get()->mEditor->mGui.mRenderGraphEditor->Update(scene);
			Gui::beginEditor(scene, *Application::Get()->activeCamera);
			ImGui::End();


			Overlay::beginTransformOverlay(*Application::Get()->activeCamera);
			fpsCounter->Update();
			// Update the sizes after resizing

		   //Gui::UpdateSizes();
			if (canUpdateContent) {
				//    Gui::FileExplorer::UpdateContent(Gui::FileExplorer::currentDirectory);
				canUpdateContent = false;
			}

			//if (isAssetImporterOpen) {
			//	Gui::AssetsImporter::Update();
			//}

			Application::Get()->mEditor->mGui.mAssetsImporter->Update(scene);


			for (auto& editorTool : Gui::sEditorTools) {
				editorTool.second->UpdateGui();
			}


			//if (ImGui::Begin("Asset Importer", &isAssetImporterOpen)) {
			//	ImGui::End();
			//}

			if (!sRenderingTextureViewer) {

			}
		}


		// Create the scene view
		inline void Gui::beginScene(Scene* scene, Camera& camera) {
			PLAZA_PROFILE_SECTION("Begin Scene");
			ApplicationSizes& appSizes = *Application::Get()->appSizes;
			ApplicationSizes& lastAppSizes = *Application::Get()->lastAppSizes;
			Entity* selectedGameObject = Editor::selectedGameObject;

			// Set the window to be the content size + header size
			ImGuiWindowFlags  sceneWindowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove;

			ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoNavFocus;
			windowFlags |= ImGuiWindowFlags_NoScrollbar;

			/*			windowFlags |= ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoTitleBar;
			windowFlags |= ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiConfigFlags_DockingEnable | ImGuiWindowFlags_NoScrollbar;*/

			ImGui::SetNextWindowSize(ImVec2(appSizes.sceneSize.x, appSizes.sceneSize.y));
			if (ImGui::Begin("Scene", &Gui::isSceneOpen, windowFlags)) {
				if (sceneViewUsingEditorCamera)
					Application::Get()->activeCamera = Application::Get()->editorCamera;
				else {
					// FIX: Make active camera depdendant on the scene
					//if (Application::Get()->activeCamera->isEditorCamera && Scene::GetActiveScene()->cameraComponents.size() > 0)
					//	Application::Get()->activeCamera = &Scene::GetActiveScene()->cameraComponents.begin()->second;
				}
			}


			if (ImGui::IsWindowFocused())
			{
				if (Application::Get()->focusedMenu != "Scene") {
					if (Input::Cursor::show)
						glfwSetInputMode(Application::Get()->mWindow->glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
					else
						glfwSetInputMode(Application::Get()->mWindow->glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				}

				Application::Get()->focusedMenu = "Scene";
			}
			if (ImGui::IsWindowHovered())
				Application::Get()->hoveredMenu = "Scene";

			ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x / 2, 25));

			ImGuiStyle& style = ImGui::GetStyle();

			// Change the background color of the button
			if (scene->mRunning) {
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
			}
			else {
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
			}
			if (isSceneOpen) {
				if (ImGui::ImageButton("PlayPauseButton", ImTextureID(playPauseButtonImageId), ImVec2(25, 25))) {
					if (scene->mRunning)
						Scene::Stop();
					else
						Scene::Play();
				}
				ImGui::SameLine();
				if (ImGui::Button("Reload C# Script Assembly")) {
					ScriptManager::ReloadScriptsAssembly(scene);
				}
				ImGui::SameLine();
				if (ImGui::Checkbox("Editor Camera", &sceneViewUsingEditorCamera)) {
					if (sceneViewUsingEditorCamera)
						Application::Get()->activeCamera = Application::Get()->editorCamera;
					else {
						// FIX: Make active camera depdendant on the scene
						//if (Application::Get()->activeCamera->isEditorCamera && Scene::GetActiveScene()->cameraComponents.size() > 0)
						//	Application::Get()->activeCamera = &Scene::GetActiveScene()->cameraComponents.begin()->second;
					}
				}
			}
			ImGui::PopStyleColor();
			if (isSceneOpen) {

				ImVec2 uv0(0, 0);
				ImVec2 uv1(1, 1);
				appSizes.sceneImageStart = ImGui::glmVec2(ImGui::GetCursorScreenPos());
				ImGui::Image(ImTextureID(Application::Get()->mRenderer->GetFrameImage()), ImGui::imVec2(appSizes.sceneSize), uv0, uv1);
				//ImGui::Image(ImTextureID(Application::Get()->textureColorbuffer), ImGui::imVec2(appSizes.sceneSize), uv0, uv1);

				//	appSizes.sceneStart = ImGui::glmVec2(ImGui::GetWindowPos());
				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(scenePayloadName.c_str())) {
						if (payload->DataSize == sizeof(Editor::File)) {
							File* file = *static_cast<File**>(payload->Data);
							DropFileScene::Drop(file);
							//if (file->extension == Standards::modelExtName) {
							//if (file->extension == Standards::modelExtName) {
							//	//file->directory, file->name
							//
							//	ModelLoader::LoadImportedModelToScene(ModelSerializer::ReadUUID(file->directory), file->directory);
							//}
							delete(file);
						}
					}
					ImGui::EndDragDropTarget();
				}
			}
			ImGui::End();
		}

		// Create the Editor view
		inline void Gui::beginEditor(Scene* scene, Camera& camera) {
			PLAZA_PROFILE_SECTION("Begin Editor");
			ApplicationSizes& appSizes = *Application::Get()->appSizes;
			ApplicationSizes& lastAppSizes = *Application::Get()->lastAppSizes;
			Entity* selectedGameObject = Editor::selectedGameObject;

			// Set the window to be the content size + header size
			ImGuiWindowFlags  sceneWindowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove;

			ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoNavFocus;
			windowFlags |= ImGuiWindowFlags_NoScrollbar;

			/*			windowFlags |= ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoTitleBar;
			windowFlags |= ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiConfigFlags_DockingEnable | ImGuiWindowFlags_NoScrollbar;*/

			ImGui::SetNextWindowSize(ImVec2(appSizes.sceneSize.x, appSizes.sceneSize.y));
			// Get a reference to the current ImGui style
			ImGuiStyle& style = ImGui::GetStyle();

			// Adjust padding and margin sizes
			style.WindowPadding = ImVec2(0.0f, 0.0f);  // Change window padding

			if (ImGui::Begin("Editor", &Gui::isSceneOpen, windowFlags)) {
				Application::Get()->activeCamera = Application::Get()->editorCamera;
			};
			if (ImGui::IsWindowFocused())
			{
				if (Application::Get()->focusedMenu != "Editor") {
					glfwSetInputMode(Application::Get()->mWindow->glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				}
				Application::Get()->focusedMenu = "Editor";
				Application::Get()->activeCamera = Application::Get()->editorCamera;
			}
			if (ImGui::IsWindowHovered())
				Application::Get()->hoveredMenu = "Editor";

			ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x / 2, 25));

			// Change the background color of the button
			if (scene->mRunning) {
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
			}
			else {
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
			}

			if (ImGui::ImageButton("PlayPauseButton", ImTextureID(playPauseButtonImageId), ImVec2(25, 25))) {
				if (scene->mRunning)
					Scene::Stop();
				else
					Scene::Play();
			}
			ImGui::SameLine();
			if (ImGui::Button("Reload C# Script Assembly")) {
				ScriptManager::ReloadScriptsAssembly(scene);
			}
			ImGui::PopStyleColor();

			ImVec2 uv0(0, 0); // bottom-left corner
			ImVec2 uv1(1, 1); // top-right corner
			appSizes.sceneImageStart = ImGui::glmVec2(ImGui::GetCursorScreenPos());

			ImGui::Image(mShowSelectedImageInEditorView && Gui::mSelectedImageInspector ? Gui::mSelectedImageInspector->mTextureID : ImTextureID(Application::Get()->mRenderer->GetFrameImage()), ImGui::imVec2(appSizes.sceneSize), uv0, uv1);

			// Show the gizmo if there's a selected entity
			std::map<std::string, File*> files = Editor::selectedFiles;
			selectedGameObject = Editor::selectedGameObject;
			if (selectedGameObject && scene->GetComponent<TransformComponent>(selectedGameObject->uuid) != nullptr && selectedGameObject->parentUuid != 0) {
				ImGuizmoHelper::IsDrawing = true;
				Editor::Gizmo::Draw(scene, selectedGameObject, camera);
			}
			else {
				ImGuizmoHelper::IsDrawing = false;
			}

			//	appSizes.sceneStart = ImGui::glmVec2(ImGui::GetWindowPos());
			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(scenePayloadName.c_str())) {
					if (payload->DataSize == sizeof(Editor::File)) {
						File* file = *static_cast<File**>(payload->Data);
						DropFileScene::Drop(file);
						//if (file->extension == Standards::modelExtName) {
						//if (file->extension == Standards::modelExtName) {
						//	AssetsLoader::LoadAsset(AssetsManager::GetAsset(file->directory));
						//}
						delete(file);
					}
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::End();
		}

		void Gui::beginHierarchyView(Scene* scene) {
			PLAZA_PROFILE_SECTION("Begin Hierarchy");
			ApplicationSizes& appSizes = *Application::Get()->appSizes;
			ApplicationSizes& lastAppSizes = *Application::Get()->lastAppSizes;
			Entity* selectedGameObject = Editor::selectedGameObject;
			//ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGuiWindowFlags  sceneWindowFlags = ImGuiWindowFlags_NoFocusOnAppearing | ImGuiConfigFlags_DockingEnable | ImGuiWindowFlags_HorizontalScrollbar | ImGuiScrollFlags_NoScrollParent;

			//ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // Set window background to red//
			ImGui::SetNextWindowSize(ImVec2(appSizes.hierarchySize.x, appSizes.hierarchySize.y));
			ImGui::Begin("Hierarchy", &Gui::isHierarchyOpen, sceneWindowFlags);
			if (ImGui::IsWindowFocused())
				Application::Get()->focusedMenu = "Hierarchy";
			if (ImGui::IsWindowHovered()) {
				Application::Get()->hoveredMenu = "Hierarchy";
			}

			appSizes.hierarchySize.x = ImGui::GetWindowSize().x;
			HierarchyPopup::Update(scene, Editor::selectedGameObject);

			if (Editor::selectedGameObject)
				Editor::selectedFiles.clear();

			// Create the main collapser
			ImGui::SetCursorPosX(0);

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
			HierarchyWindow::Item::NewItem(*Scene::GetActiveScene()->GetEntity(Scene::GetActiveScene()->mainSceneEntity->uuid), selectedGameObject, scene);
			ImGui::PopStyleVar();
			ImGui::PopStyleVar();


			curHierarchySize = ImGui::glmVec2(ImGui::GetWindowSize());
			ImGui::End();
		}

		void Gui::beginInspector(Scene* scene, Camera camera) {
			PLAZA_PROFILE_SECTION("Begin Inspector");
			ApplicationSizes& appSizes = *Application::Get()->appSizes;
			ApplicationSizes& lastAppSizes = *Application::Get()->lastAppSizes;
			Entity* selectedGameObject = Editor::selectedGameObject;

			//ImGui::SetCursorPosY(appSizes.appHeaderSize);
			ImGui::SetNextWindowSize(ImGui::imVec2(appSizes.inspectorSize), ImGuiCond_Always);
			//ImGui::SetNextWindowPos(ImVec2(appSizes.appSize.x - appSizes.inspectorSize.x, 0));

			ImGuiWindowFlags sceneWindowFlags = ImGuiWindowFlags_NoFocusOnAppearing | ImGuiConfigFlags_DockingEnable;
			ImGuiWindowFlags  inspectorWindowFlags = ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing;
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f)); // Remove the padding of the window
			//ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // Set window background to red//
			ImGui::SetNextWindowSize(ImGui::imVec2(appSizes.inspectorSize));
			bool visibleInspector = ImGui::Begin("Inspector", &Gui::isInspectorOpen, sceneWindowFlags);
			if (ImGui::IsWindowFocused())
				Application::Get()->focusedMenu = "Inspector";
			if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
				Application::Get()->hoveredMenu = "Inspector";


			//// Handle size changes
			//const ImVec2& CurSize = ImGui::GetWindowSize();
			//if (!ImGui::Compare(CurSize, ImGui::imVec2(lastAppSizes.inspectorSize))) {
			//	//appSizes.inspectorSize = ImGui::glmVec2(CurSize);
			//	lastAppSizes.inspectorSize = appSizes.inspectorSize;
			//	ImGui::SetWindowSize(ImGui::imVec2(appSizes.inspectorSize), ImGuiCond_Always);
			//	//ImGui::SetWindowPos(ImVec2(appSizes.appSize.x - appSizes.inspectorSize.x, 0));
			//}

			if (visibleInspector && Editor::selectedFiles.size() > 0) {
				Editor::Inspector::FileInspector::CreateInspector();
			}
			else if (visibleInspector && selectedGameObject) {
				if (selectedGameObject->parentUuid)
					Inspector::ComponentInspector::UpdateComponents();
				Editor::Inspector::ComponentInspector::CreateInspector(scene);
				ImGui::Text(std::to_string(selectedGameObject->uuid).c_str());
			}


			curInspectorSize = ImGui::glmVec2(ImGui::GetWindowSize());
			ImGui::End();
			ImGui::PopStyleVar();
		}

		void Gui::beginImageInspector(Scene* scene, Camera camera) {
			PLAZA_PROFILE_SECTION("Begin Image Inspector");
			ApplicationSizes& appSizes = *Application::Get()->appSizes;
			ApplicationSizes& lastAppSizes = *Application::Get()->lastAppSizes;
			Entity* selectedGameObject = Editor::selectedGameObject;

			ImGui::SetNextWindowSize(ImGui::imVec2(appSizes.inspectorSize), ImGuiCond_Always);

			ImGuiWindowFlags sceneWindowFlags = ImGuiWindowFlags_NoFocusOnAppearing | ImGuiConfigFlags_DockingEnable;
			ImGuiWindowFlags  inspectorWindowFlags = ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing;
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f)); // Remove the padding of the window
			//ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // Set window background to red//
			ImGui::SetNextWindowSize(ImGui::imVec2(appSizes.inspectorSize));
			ImGui::Begin("Image Inspector", &Gui::isInspectorOpen, sceneWindowFlags);
			if (ImGui::IsWindowFocused())
				Application::Get()->focusedMenu = "ImageInspector";
			if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
				Application::Get()->hoveredMenu = "ImageInspector";


			ImVec2 uv0(0, mFlipY ? 0 : 1); // bottom-left corner
			ImVec2 uv1(1, mFlipY ? 1 : 0); // top-right corner
			appSizes.sceneImageStart = ImGui::glmVec2(ImGui::GetCursorScreenPos());

			Gui::imageSize = ImVec2(200 * (Application::Get()->appSizes->sceneSize.x / Application::Get()->appSizes->sceneSize.y), 200);
			ImGui::Checkbox("Show All Images", &mImageInspectorShowAllImages);
			ImGui::Checkbox("Show image in editor view", &mShowSelectedImageInEditorView);
			ImGui::Checkbox("Flip Y", &mFlipY);

			if (!Gui::mImageInspectorShowAllImages && Gui::mSelectedImageInspector) {
				ImGui::Image(Application::Get()->mRenderer->GetTrackedImageID(Gui::mSelectedImageInspector), Gui::imageSize, uv0, uv1);
				int tempMipLevel = static_cast<int>(Gui::mSelectedImageInspector->mTrackerSetting.mMipLevel);
				if (ImGui::DragInt("Mip", &tempMipLevel, 1.0f, 0, 255)) {
					Gui::mSelectedImageInspector->mTrackerSetting.mMipLevel = tempMipLevel;
					Gui::mSelectedImageInspector->mRecalculateView = true;
				}

				int tempLayerLevel = static_cast<int>(Gui::mSelectedImageInspector->mTrackerSetting.mLayerLevel);
				if (ImGui::DragInt("Layer", &tempLayerLevel, 1.0f, 0, Gui::mSelectedImageInspector->mTextureInfo.mLayersCount)) {
					Gui::mSelectedImageInspector->mTrackerSetting.mLayerLevel = tempLayerLevel;
					Gui::mSelectedImageInspector->mRecalculateView = true;
				}
			}

			Gui::sRenderingTextureViewer = true;
			for (unsigned int i = 0; i < Application::Get()->mRenderer->mTrackedImages.size(); ++i) {
				if (Gui::mImageInspectorShowAllImages) {
					ImGui::Image(Application::Get()->mRenderer->GetTrackedImageID(Application::Get()->mRenderer->GetTrackedImage(i)), Gui::imageSize, uv0, uv1);
				}
				else
				{
					if (ImGui::Button(Application::Get()->mRenderer->mTrackedImages[i]->name.c_str())) {
						Gui::mSelectedImageInspector = Application::Get()->mRenderer->GetTrackedImage(i);
					}
				}
			}

			curInspectorSize = ImGui::glmVec2(ImGui::GetWindowSize());
			ImGui::End();
			ImGui::PopStyleVar();
		}

		void Gui::beginAssetsViewer(Scene* scene, Camera camera) {
			PLAZA_PROFILE_SECTION("Begin Assets Viewer");
			ApplicationSizes& appSizes = *Application::Get()->appSizes;
			ApplicationSizes& lastAppSizes = *Application::Get()->lastAppSizes;
			Entity* selectedGameObject = Editor::selectedGameObject;

			// Set the window to be the content size + header size
			ImGuiWindowFlags  sceneWindowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove;

			ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoNavFocus;
			windowFlags |= ImGuiWindowFlags_NoScrollbar;

			ImGui::SetNextWindowSize(ImVec2(appSizes.sceneSize.x, appSizes.sceneSize.y));

			ImGuiStyle& style = ImGui::GetStyle();

			// Adjust padding and margin sizes
			style.WindowPadding = ImVec2(0.0f, 0.0f);  // Change window padding
			if (ImGui::Begin("Assets Viewer", &Gui::isSceneOpen, windowFlags)) {
				int itemsCount = AssetsManager::mAssets.size();
				ImGuiListClipper clipper;
				clipper.Begin(itemsCount);
				while (clipper.Step()) {
					for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
						auto& [key, value] = *std::next(std::begin(AssetsManager::mAssets), i);

						if (ImGui::TreeNodeEx(value->mAssetPath.filename().string().c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
							ImGui::Text("Uuid: ");
							ImGui::SameLine();
							ImGui::Text(std::to_string(value->mAssetUuid).c_str());

							ImGui::Text("Path: ");
							ImGui::SameLine();
							ImGui::Text(value->mAssetPath.string().c_str());

							ImGui::Text("Extension: ");
							ImGui::SameLine();
							ImGui::Text(value->GetExtension().c_str());


							ImGui::TreePop();
						}
					}
				}

				clipper.End();
			};
			if (ImGui::IsWindowFocused())
			{
				if (Application::Get()->focusedMenu != "AssetsViewer") {
					glfwSetInputMode(Application::Get()->mWindow->glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				}
				Application::Get()->focusedMenu = "AssetsViewer";
			}
			if (ImGui::IsWindowHovered())
				Application::Get()->hoveredMenu = "AssetsViewer";

			ImGui::End();
		}

		void Gui::beginProfiler(Scene* scene) {
			PLAZA_PROFILE_SECTION("Begin Profiler");
			ApplicationSizes& appSizes = *Application::Get()->appSizes;
			ApplicationSizes& lastAppSizes = *Application::Get()->lastAppSizes;
			Entity* selectedGameObject = Editor::selectedGameObject;

			// Set the window to be the content size + header size
			ImGuiWindowFlags  sceneWindowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove;

			ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoNavFocus;
			windowFlags |= ImGuiWindowFlags_NoScrollbar;

			ImGui::SetNextWindowSize(ImVec2(appSizes.sceneSize.x, appSizes.sceneSize.y));

			ImGuiStyle& style = ImGui::GetStyle();

			// Adjust padding and margin sizes
			style.WindowPadding = ImVec2(0.0f, 0.0f);  // Change window padding
			if (ImGui::Begin("Profiler", &Gui::isSceneOpen, windowFlags)) {
				for (const auto& [key, value] : Profiler::sProfilers) {
					if (ImGui::TreeNode(key.c_str())) {
						value->OrderSections();

						ImGui::Text("Total Time: ");
						ImGui::SameLine();
						ImGui::Text(std::string(std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(value->GetNanoSeconds()).count()) + " Milliseconds").c_str());

						for (const SectionProfiler& section : value->mSections) {
							ImGui::Text(std::string(section.mName + " " + std::to_string(section.mDurationNanoSeconds / 1000 / 100) + " Milliseconds").c_str());
						}
						ImGui::TreePop();
					}
				}
			};
			if (ImGui::IsWindowFocused())
			{
				if (Application::Get()->focusedMenu != "Profiler") {
					glfwSetInputMode(Application::Get()->mWindow->glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				}
				Application::Get()->focusedMenu = "Profiler";
			}
			if (ImGui::IsWindowHovered())
				Application::Get()->hoveredMenu = "Profiler";

			ImGui::End();
		}

		void Gui::beginMainProgressBar(float percentage) {
			PLAZA_PROFILE_SECTION("Begin Main Progress Bar");
			ApplicationSizes& appSizes = *Application::Get()->appSizes;
			ApplicationSizes& lastAppSizes = *Application::Get()->lastAppSizes;
			Entity* selectedGameObject = Editor::selectedGameObject;

			// Set the window to be the content size + header size
			ImGuiWindowFlags  sceneWindowFlags = ImGuiWindowFlags_MenuBar;

			ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse;
			windowFlags |= ImGuiWindowFlags_NoScrollbar;

			ImGui::SetNextWindowSize(ImVec2(appSizes.sceneSize.x, appSizes.sceneSize.y));

			ImGuiStyle& style = ImGui::GetStyle();

			// Adjust padding and margin sizes
			style.WindowPadding = ImVec2(0.0f, 0.0f);  // Change window padding
			if (ImGui::Begin("Progress Bar", &Gui::isSceneOpen, windowFlags)) {
				ImGui::ProgressBar(percentage);
			}
			ImGui::End();
		}

		void Gui::OpenAssetImporterContext(std::string filePath) {
			isAssetImporterOpen = true;
			mAssetToImportPath = filePath;
		}

		// Update appSizes
		void Gui::UpdateSizes() {
			ApplicationSizes& appSizes = *Application::Get()->appSizes;
			appSizes.hierarchySize = glm::abs(curHierarchySize);
			appSizes.inspectorSize = glm::abs(curInspectorSize);
			//appSizes.sceneSize = curSceneSize;
		}
	};
}

//glm::vec2 ImGui::vec2() {
//	return glm::vec2;
//}