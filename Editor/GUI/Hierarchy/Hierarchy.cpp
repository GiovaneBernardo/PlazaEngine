#include "Engine/Core/PreCompiledHeaders.h"
#include "Editor/GUI/Hierarchy/Hierarchy.h"
#include "Engine/Editor/Editor.h"
#include "Editor/GUI/guiMain.h"
#include "Editor/GUI/Hierarchy/HierarchyPopup.h"
#include "Editor/DefaultAssets/DefaultAssets.h"

#include "Editor/GUI/Popups/NewEntityPopup.h"
#include "Engine/Core/Scripting/Mono.h"
#include "Engine/Core/Scripting/CppScriptFactory.h"
#include "Engine/Components/Rendering/Material.h"
#include "Engine/Core/RenderGroup.h"
#include "Engine/Application/Callbacks/CallbacksHeader.h"


namespace Plaza::Editor {
	void HierarchyWindow::Init() {
		auto onKeyPressLambda = [this](int key, int scancode, int action, int mods) {
			this->OnKeyPress(key, scancode, action, mods);
			};
		Callbacks::AddFunctionToKeyCallback({ onKeyPressLambda, GuiLayer::ASSETS_IMPORTER });
	}

	void HierarchyWindow::Update(Scene* scene) {
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
		HierarchyPopup::Update(scene);

		if (Editor::selectedGameObject)
			Editor::selectedFiles.clear();

		// Create the main collapser
		ImGui::SetCursorPosX(0);

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
		HierarchyWindow::Item::NewItem(*Scene::GetActiveScene()->GetEntity(Scene::GetActiveScene()->mainSceneEntity->uuid), selectedGameObject, mScene);
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();


		//Gui::curHierarchySize = ImGui::glmVec2(ImGui::GetWindowSize());
		ImGui::End();
	}

	void HierarchyWindow::OnKeyPress(int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_C && GLFW_PRESS && mods == GLFW_MOD_CONTROL) {
			std::cout << "coopy \n";
		}
	}


	bool HierarchyWindow::Item::firstFocus = false;
	float inputTextWidth = 0;
	void HierarchyWindow::Item::NewItem(Entity& entity, Entity*& selectedGameObject, Scene* scene) {
		const float height = 9.0f;
		// Push the entity id, to prevent it to collpases all the treenodes with same id
		ImGui::PushID(entity.uuid);
		// Start the treenode before the component selectable, but only assign its values after creating the button

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f)); // Remove the padding of the window
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, editorStyle.treeNodeActiveBackgroundColor);

		bool itemIsSelectedObject = false;
		if (selectedGameObject && entity.uuid == selectedGameObject->uuid) itemIsSelectedObject = true;

		if (itemIsSelectedObject) {// Selected backgroundde
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, editorStyle.selectedTreeNodeBackgroundColor);
			ImGui::PushStyleColor(ImGuiCol_Header, editorStyle.selectedTreeNodeBackgroundColor);
		}
		else {// Not selected background
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, editorStyle.treeNodeHoverBackgroundColor);
			ImGui::PushStyleColor(ImGuiCol_Header, editorStyle.treeNodeBackgroundColor);
		}



		bool treeNodeOpen = false;

		ImGuiStyle& style = ImGui::GetStyle();



		style.IndentSpacing = 15.0f;
		//ImGui::SetCursorPosX(ImGui::GetCursorPosX());
		float indentSpacing = ImGui::GetStyle().IndentSpacing;
		const int depth = 1.0f;

		//ImGui::Indent(3.0f);
		//ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetTreeNodeToLabelSpacing() + 3.0f); // Decrease the indentation spacing

		if (!entity.changingName) {
			if (entity.childrenUuid.size() > 0) {
				treeNodeOpen = ImGui::TreeNodeEx(entity.name.c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanFullWidth);
			}
			else {

				//ImGui::Selectable(entity->name.c_str(), ImGuiTreeNodeFlags_Framed);
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetTreeNodeToLabelSpacing());
				//ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
				treeNodeOpen = ImGui::TreeNodeEx(entity.name.c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth);
			}
		}
		else {
			treeNodeOpen = true;
		}

		bool nameChanged = false;
		if (entity.changingName) {
			char buf[512];
			strcpy_s(buf, entity.name.c_str());
			if (firstFocus) {
				ImGui::SetKeyboardFocusHere();
			}
			float currentIndent = ImGui::GetCursorPosX();
			if (Scene::GetActiveScene()->GetEntity(entity.parentUuid)->childrenUuid.size() <= 0)
				ImGui::SetCursorPosX(currentIndent + 0);
			else
				ImGui::SetCursorPosX(currentIndent + 20);

			ImGui::SetNextWindowSize(ImVec2(50.0f, height));
			if (ImGui::InputTextEx("##EntityNameInput", "Name", buf, 512, ImVec2(inputTextWidth, 0), ImGuiInputTextFlags_EnterReturnsTrue)) {
				entity.Rename(buf);
				entity.changingName = false;
				nameChanged = true;
				Gui::changeSelectedGameObject(Scene::GetActiveScene()->GetEntity(entity.uuid));
			}

			if (!ImGui::IsItemActive() && !firstFocus) {
				entity.Rename(buf);
				entity.changingName = false;
				nameChanged = true;
				Gui::changeSelectedGameObject(Scene::GetActiveScene()->GetEntity(entity.uuid));
			}

			if (firstFocus) {
				firstFocus = false;
			}

			inputTextWidth = ImGui::CalcTextSize(buf).x + 30;
		}

		//ImGui::SetWindowPos(ImVec2(ImGui::GetWindowPos().x - 1.0f, ImGui::GetWindowPos().y));

		// Get the start and end position of the header
		ImVec2 treeNodeMin = ImGui::GetItemRectMin();
		ImVec2 treeNodeMax = ImGui::GetItemRectMax();
		ImVec2 treeNodePos = ImVec2(treeNodeMin.x, treeNodeMin.y);
		//ImGui::PopStyleVar(); // IndentSpacing

		ImGui::PopStyleColor(); // Header Active Background (Active is when I click on it)
		ImGui::PopStyleColor(); // Header Hovered Background
		ImGui::PopStyleColor(); // Header Selected Background

		// Change the selected entity if user clicked on the selectable
		if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
			Gui::changeSelectedGameObject(Scene::GetActiveScene()->GetEntity(entity.uuid));
		//Plaza::Editor::selectedGameObject = entity;


		if (ImGui::IsItemVisible()) {
			if (entity.parentUuid && ImGui::BeginDragDropSource()) {
				ImGui::SetDragDropPayload(payloadName.c_str(), &entity, sizeof(Entity*));

				ImVec2 mousePos = ImGui::GetMousePos();
				ImVec2 treeNodePos = treeNodeMin;
				ImVec2 treeNodeSize = ImVec2(treeNodeMax.x - treeNodeMin.x, treeNodeMax.y - treeNodeMin.y);

				ImVec2 treeNodeCenter = ImVec2(treeNodePos.x, treeNodePos.y + treeNodeSize.y / 2);
				ImGui::EndDragDropSource();
			}

			HierarchyWindow::Item::HierarchyDragDrop(entity, &entity, treeNodeMin, treeNodeMax, scene);
		}
		bool treePop = !entity.changingName && !nameChanged;
		if (ImGui::IsItemHovered() || ImGui::IsPopupOpen("ItemPopup")) {
			HierarchyWindow::Item::ItemPopup(entity, scene);
		}

		if (ImGui::IsPopupOpen("ItemPopup")) {
			Gui::changeSelectedGameObject(Scene::GetActiveScene()->GetEntity(entity.uuid));
		}

		if (treeNodeOpen)
		{
			for (uint64_t child : entity.childrenUuid)
			{
				HierarchyWindow::Item::NewItem(*Scene::GetActiveScene()->GetEntity(child), selectedGameObject, scene);
			}
			if (treePop)
				ImGui::TreePop();
		}
		//ImGui::Unindent(indentSpacing * depth);
		ImGui::PopStyleVar();

		ImGui::PopID();
	};

	void HierarchyWindow::Item::ItemPopup(Entity& entity, Scene* scene) {
		if (ImGui::BeginPopupContextWindow("ItemPopup"))
		{
			if (ImGui::BeginMenu("Add Component"))
			{
				if (ImGui::MenuItem("Mesh Renderer"))
				{
					MeshRenderer* meshRenderer = scene->AddComponent<MeshRenderer>(entity.uuid);
					meshRenderer->mMaterials.push_back(AssetsManager::GetDefaultMaterial());
					meshRenderer->instanced = true;
				}

				if (ImGui::MenuItem("Rigid Body Dynamic"))
				{
					RigidBody* rigidBody = scene->AddComponent<RigidBody>(entity.uuid);
					rigidBody->mUuid = entity.uuid;
					//Collider* collider = new Collider(entity.uuid);
					//entity.AddComponent<Collider>(collider);
					//entity.GetComponent<Collider>()->Init();
				}

				if (ImGui::MenuItem("Rigid Body Non Dynamic"))
				{
					RigidBody* rigidBody = scene->AddComponent<RigidBody>(entity.uuid);
					rigidBody->dynamic = false;
					rigidBody->mUuid = entity.uuid;
				}

				if (ImGui::MenuItem("Collider"))
				{
					Collider* collider = scene->AddComponent<Collider>(entity.uuid);
				}

				if (ImGui::MenuItem("Camera"))
				{
					Camera* camera = scene->AddComponent<Camera>(entity.uuid);
					camera->mUuid = entity.uuid;
				}

				if (ImGui::MenuItem("Text Renderer"))
				{
					Plaza::Drawing::UI::TextRenderer* textRenderer = scene->AddComponent<Plaza::Drawing::UI::TextRenderer>(entity.uuid);
					textRenderer->Init(Application::Get()->activeProject->mAssetPath.parent_path().string() + "\\font.ttf");
					textRenderer->mUuid = entity.uuid;
				}

				if (ImGui::MenuItem("Audio Source"))
				{
					AudioSource* audioSource = scene->AddComponent<AudioSource>(entity.uuid);
					audioSource->mUuid = entity.uuid;
				}

				if (ImGui::MenuItem("Audio Listener"))
				{
					AudioListener* audioListener = scene->AddComponent<AudioListener>(entity.uuid);
					audioListener->mUuid = entity.uuid;
				}

				if (ImGui::MenuItem("Rename")) {
					entity.changingName = true;
					Scene::GetActiveScene()->GetEntity(entity.uuid)->changingName = true;
					HierarchyWindow::Item::firstFocus = true;
				}

				if (ImGui::BeginMenu("C++ Script"))
				{
					for (auto& [key, value] : AssetsManager::mScripts) {
						if (value->GetExtension() != ".h")
							continue;

						//TODO: FIX SCRIPTS WITH INCORRECT NAME AND REMOVE THE BELOW HACK USING THE ASSET
						Asset* asset = AssetsManager::GetAsset(key);
						if (ImGui::MenuItem(value->mAssetName.c_str())) {
							CppScriptComponent* component = Scene::GetActiveScene()->GetComponent<CppScriptComponent>(entity.uuid);
							if (!component) {
								component = scene->AddComponent<CppScriptComponent>(entity.uuid);
							}
							CppScript* script = ScriptFactory::CreateScript(std::filesystem::path(value->mAssetName).stem().string());
							if (!script) {
								PL_CORE_ERROR("Added Script is a nullptr");
								continue;
							}
							script->mAssetUuid = value->mAssetUuid;
							component->AddScript(script);
							if (Application::Get()->runningScene) {
								script->OnStart();
							}
							//entity.AddComponent<CppScriptComponent>(component);
						}
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("C# Script"))
				{
					for (auto& [key, value] : AssetsManager::mScripts) {
						if (value->GetExtension() != ".cs")
							continue;
						//TODO: FIX SCRIPTS WITH INCORRECT NAME AND REMOVE THE BELOW HACK USING THE ASSET
						Asset* asset = AssetsManager::GetAsset(key);
						if (ImGui::MenuItem(value->mAssetName.c_str())) {
							CsScriptComponent* script = scene->AddComponent<CsScriptComponent>(entity.uuid);
							script->mScriptUuid = value->mAssetUuid;
							//std::string csFileName = filesystem::path{ key }.replace_extension(".cs").string();
							script->Init();
							//Application::Get()->activeProject->scripts.at(csFileName).entitiesUsingThisScript.emplace(entity.uuid);
							if (Application::Get()->runningScene) {
								for (auto& [key, value] : script->scriptClasses) {
									Mono::OnStart(value->monoObject);
								}
							}
						}
					}

					ImGui::EndMenu();
				}

				if (ImGui::MenuItem("Light"))
				{
					Light* light = scene->AddComponent<Light>(entity.uuid);
					light->mUuid = entity.uuid;
				}

				if (ImGui::MenuItem("Character Controller"))
				{
					CharacterController* characterController = scene->AddComponent<CharacterController>(entity.uuid);
					characterController->mUuid = entity.uuid;
				}

				if (ImGui::MenuItem("Animation"))
				{
					AnimationComponent* animationComponent = scene->AddComponent<AnimationComponent>(entity.uuid);
					animationComponent->mUuid = entity.uuid;
				}

				if (ImGui::MenuItem("Gui"))
				{
					GuiComponent* guiComponent = scene->AddComponent<GuiComponent>(entity.uuid);
					guiComponent->mUuid = entity.uuid;
				}

				ImGui::EndMenu();
			}

			Popup::NewEntityPopup::Init(&entity, scene);



			ImGui::EndPopup();
		}
	}
};
