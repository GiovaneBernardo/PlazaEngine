#pragma once
#include "Engine/Core/Standards.h"
#include "Editor/GUI/Style/EditorStyle.h"
#include "Editor/GUI/guiMain.h"
#include "Editor/GUI/FileExplorer/FileIcon.h"
// #include "Engine/Core/Renderer/Renderer.h"

namespace Plaza {
	namespace Editor {
		class File {
		  public:
			File(std::string name, std::string directory, std::string extension)
				: name(name), directory(directory), extension(extension) {
				const auto& it = Icon::textures.find(extension);
				if (it != Icon::textures.end()) {
					this->textureId = it->second.id;
				}
				else
					this->textureId = Icon::textures.at(".notFound").id;
			}
			File(const File&) = default;
			~File() = default;

			static float iconSize;
			static float spacing;
			static glm::vec2 size;
			static ImVec2 currentPos;
			static bool firstFocus;
			static std::string changingName;

			std::string name;
			std::string directory;
			std::string extension;
			ImTextureID textureId;
			bool temporary = false;

			virtual void DoubleClick();
			virtual void Delete() {}
			virtual void Rename(std::string oldPath, std::string newPath) {}
			virtual void Move(std::string oldPath, std::string newPath) {}
			virtual void Copy() {}
			virtual void Paste() {}
			virtual void Popup() {}
			virtual void DragDrop() {}
		};
	} // namespace Editor
} // namespace Plaza
