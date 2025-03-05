#include "Engine/Core/PreCompiledHeaders.h"
#include "ProjectManager.h"

#include "Editor/GUI/MenuBar.h"
#include "Editor/GUI/ProjectManager/ProjectItem.h"
#include "Editor/GUI/Style/EditorStyle.h"
#include "Editor/GUI/FileExplorer/FileExplorer.h"
#include "Editor/Settings/ProjectGenerator.h"
#include "Editor/SessionCache/Cache.h"
#include <ThirdParty/imgui/imgui_impl_opengl3.h>
#include "Editor/DefaultAssets/DefaultAssets.h"
#include "Engine/Core/Renderer/Renderer.h"
#include "Engine/Core/AssetsManager/Serializer/AssetsSerializer.h"

namespace Plaza {
	namespace Editor {
		void ProjectManagerGui::ProjectManagerContent::UpdateContent(ProjectManagerGui& projectManagerGui) {
			projectManagerGui.NewProjectButton();
			ProjectManagerGui::SetupProjectsTreeNode();
		}
		/// <summary>
		/// Update the content of dockspace to show the New Project window
		/// </summary>
		/// <param name="projectManagerGui"></param>
		void ProjectManagerGui::NewProjectContent::UpdateContent(ProjectManagerGui& projectManagerGui) {
			ImGuiWindowFlags containerFlags =
				ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNavFocus;
			if (ImGui::Begin("New Project Container", nullptr, containerFlags)) {
				// Make the: input text, create project button and cancel button
				static char name[256] = "";
				ImGui::InputText("##InputText", name, IM_ARRAYSIZE(name));
				if (ImGui::Button("Create Project") && std::string(name) != "") {
					Application::Get()->activeProject->mAssetName = std::string(name);
					Application::Get()->activeProject->mAssetPath =
						Application::Get()->activeProject->mAssetPath.string() + "/" + name + Standards::projectExtName;

					Application::Get()->runEngine = true;
					Application::Get()->runProjectManagerGui = false;

					// Create the main project settings file
					Gui::FileExplorer::currentDirectory =
						Application::Get()->activeProject->mAssetPath.parent_path().string();
					AssetsSerializer::SerializeFile<Project>(*Application::Get()->activeProject,
															 Application::Get()->activeProject->mAssetPath.string(),
															 Application::Get()->mSettings.mProjectSerializationMode);
					// ProjectSerializer::Serialize(Application::Get()->activeProject->mAssetPath.parent_path().string()
					// + "/" + Application::Get()->activeProject->mAssetName + Standards::projectExtName);

					// Create visual studio solution and project
					const std::string solutionName = Application::Get()->activeProject->mAssetName;
					const std::string projectName = Application::Get()->activeProject->mAssetName;
					const std::string outputDirectory =
						Application::Get()->activeProject->mAssetPath.parent_path().string(); // Specify the desired
																							  // output directory
					ProjectGenerator::GenerateSolution(solutionName, projectName, outputDirectory);
					ProjectGenerator::GenerateProject(projectName, outputDirectory);
					ProjectGenerator::PasteAllProjectFiles(outputDirectory);

					// Create scripts holder file
					// ScriptManagerSerializer::Create(Application::Get()->activeProject->directory + "/Scripts" +
					// Standards::scriptConfigExtName);
					// ScriptManagerSerializer::DeSerialize(Application::Get()->activeProject->directory + "/Scripts" +
					// Standards::scriptConfigExtName);

					// Update the file explorer content
					Editor::Gui::FileExplorer::UpdateContent(
						Application::Get()->activeProject->mAssetPath.parent_path().string());

					Application::Get()->projectPath =
						Application::Get()->activeProject->mAssetPath.parent_path().string();
					Cache::Serialize(Application::Get()->enginePathAppData + "/cache" + Standards::editorCacheExtName);

					Project::Load(Application::Get()->activeProject->mAssetPath.string());

					// Load Default Models
					// Editor::DefaultModels::Init();
				}
				if (ImGui::Button("Cancel")) {
					projectManagerGui.currentContent = new ProjectManagerContent();
				}

				ImGui::End();
			}
		}

		void ProjectManagerGui::Update() {
			ProjectManagerGui::SetupDockspace();
			Application::Get()->mRenderer->UpdateProjectManager();
		}

		void ProjectManagerGui::SetupDockspace() {
			// Imgui New Frame
			Gui::NewFrame();

			ApplicationSizes& appSizes = *Application::Get()->appSizes;
			ApplicationSizes& lastAppSizes = *Application::Get()->lastAppSizes;

			// ImGui::PushStyleColor(ImGuiCol_WindowBg, editorStyle.hierarchyBackgroundColor);

			//	ImGuiWindowFlags  windowFlags = ImGuiWindowFlags_MenuBar;

			ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar;
			windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
						   ImGuiWindowFlags_NoMove;
			windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
			ImGui::SetNextWindowSize(ImGui::imVec2(appSizes.appSize));

			ImGui::PushStyleColor(ImGuiCol_WindowBg, Gui::sEditorStyle->hierarchyBackgroundColor);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
			bool showProjectManager = true;
			if (ImGui::Begin("Main DockSpace", &showProjectManager, windowFlags)) {
				// Create the dockspace
				ImGuiID dockspace_id = ImGui::GetID("Main DockSpace");
				ImGuiDockNodeFlags dockspace_flags =
					ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_NoDockingInCentralNode;
				dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
				ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

				Gui::MainMenuBar::Begin(); // Title bar

				this->currentContent->UpdateContent(*this);

				ImGui::End();
			}
			ImGui::PopStyleColor();
			ImGui::PopStyleVar();
			ImGui::Render();
		}

		void ProjectManagerGui::SetupProjectsTreeNode() {
			ImGuiWindowFlags containerFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
											  ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoNavFocus;
			if (ImGui::Begin("Projects Container", nullptr, containerFlags)) {
				for (unsigned int i = 0; i < 10; i++) {
					std::unique_ptr<ProjectManagerGui::ProjectItem> guiItem =
						std::make_unique<ProjectManagerGui::ProjectItem>("nomeFoda" + std::to_string(i),
																		 "caminho/caminho2/caminho3/caminho4");
				}
			}
			ImGui::End();
		}

		void ProjectManagerGui::NewProjectButton() {
			ImGuiWindowFlags containerFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
											  ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoNavFocus |
											  ImGuiWindowFlags_AlwaysAutoResize;
			ImGui::SetNextWindowPos(ImVec2(800, 500));
			if (ImGui::Begin("Buttons Container", nullptr, containerFlags)) {
				bool newProjectButton = ImGui::Button("New Project", ImVec2(500, 100));

				if (newProjectButton) {
					ProjectManagerGui::NewProjectClick();
				}

				ProjectManagerGui::OpenProjectButton();
			}
			ImGui::End();
		}

		void ProjectManagerGui::OpenProjectButton() {
			bool openProjectButton = ImGui::Button("Open Project", ImVec2(500, 100));
			if (openProjectButton) {
				ProjectManagerGui::OpenProjectClick();
			}
		}
	} // namespace Editor
} // namespace Plaza
