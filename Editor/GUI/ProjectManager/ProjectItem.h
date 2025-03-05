#pragma once
#include "Editor/GUI/ProjectManager/ProjectManager.h"

namespace Plaza {
	namespace Editor {
		class ProjectManagerGui::ProjectItem {
		  public:
			std::string name;
			std::string path;
			void Setup();
			void OnClick();
			ProjectItem(std::string name, std::string path) : name(name), path(path) { Setup(); }
		};
	} // namespace Editor
} // namespace Plaza
