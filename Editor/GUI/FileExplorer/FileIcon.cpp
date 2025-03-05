#include "Engine/Core/PreCompiledHeaders.h"
#include "FileIcon.h"
#include "Editor/GUI/FileExplorer/FileExplorer.h"
#include "Editor/GUI/Utils/Filesystem.h"
#include "Editor/GUI/Popups/FileExplorerFilePopup.h"
#include "Engine/Core/Renderer/Renderer.h"
namespace Plaza {
	namespace Editor {
		std::map<std::string, IconTexture> Icon::textures = std::map<std::string, IconTexture>();

		float Editor::File::iconSize = 75.0f;
		float Editor::File::spacing = 5.0f;
		ImVec2 Editor::File::currentPos = ImVec2(-1.0f, 1.0f);
		bool Editor::File::firstFocus = false;
		std::string Editor::File::changingName = "";

		void Icon::LoadImageToImGuiTexture(const char* path, ImTextureID& outTextureID) {
			outTextureID = Application::Get()->mRenderer->LoadImGuiTexture(path)->GetImGuiTextureID();
		}
	} // namespace Editor
} // namespace Plaza
