#pragma once
#include "Engine/Core/Engine.h"
#include "Editor/Project.h"
#include <string>
#include <filesystem>

namespace Plaza {
	class PLAZA_API Scripting {
	  public:
		static void LoadProjectCppDll(Scene* scene, const Editor::Project& project);
		static void ReloadAllScripts(Scene* scene);
		static void UnloadAllScripts(Scene* scene);
		static void Update(Scene* scene);

		static std::filesystem::path CopyPasteDevelopmentLibraryFiles(const Editor::Project& project);
		static void PasteEngineLibToGameProject(const Editor::Project& project);
		static void UnloadCurrentLoadedCppDll();

	  private:
		static bool LoadCppDll(const std::filesystem::path& path);
#ifdef WIN32
		static inline HMODULE sCurrentLoadedCppDll;
#elif __linux__
		static inline void* sCurrentLoadedCppHandle;
#endif
		static inline int sReloadIndex = 0;
	};
} // namespace Plaza
