#include "Engine/Core/PreCompiledHeaders.h"
#include "Editor.h"
#include "Editor/GUI/FileExplorer/File.h"
// Entity* Plaza::Editor::selectedGameObject = nullptr;
namespace Plaza {
	namespace Editor {
		// std::map<uint64_t, std::map<std::string, std::map<std::string, Field*>>> Editor::lastSavedScriptsFields =
		// std::map<uint64_t, std::map<std::string, std::map<std::string, Field*>>>();
		Entity* selectedGameObject = nullptr;
		std::map<std::string, File*> selectedFiles = std::map<std::string, File*>();
	} // namespace Editor
} // namespace Plaza
