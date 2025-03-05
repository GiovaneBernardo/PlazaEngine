#pragma once

namespace Plaza {
	namespace Editor {
		class ProjectManagerGui {
		  public:
			class ProjectItem;
			void Init();
			void Update();

			void SetupDockspace();
			void SetupMenuBar();
			static void SetupProjectsTreeNode();

			void NewProjectButton();
			void NewProjectClick();
			void OpenProjectButton();
			void OpenProjectClick();
			class ProjectManagerContent {
			  public:
				/// <summary>
				/// Update the content of dockspace to show the Projects and the button to create a new project
				/// </summary>
				/// <param name="projectManagerGui"></param>
				virtual void UpdateContent(ProjectManagerGui& projectManagerGui);
			};

			class NewProjectContent : public ProjectManagerContent {
			  public:
				/// <summary>
				/// Update the content of dockspace to show the New Project window
				/// </summary>
				/// <param name="projectManagerGui"></param>
				void UpdateContent(ProjectManagerGui& projectManagerGui) override;
			};

			ProjectManagerContent* currentContent;

			ProjectManagerGui() { currentContent = new ProjectManagerContent(); }
		};
	} // namespace Editor
} // namespace Plaza
