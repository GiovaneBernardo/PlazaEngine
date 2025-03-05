#pragma once

namespace Plaza {
	namespace Editor {
		class IconTexture {
		  public:
			ImTextureID id;
			string extension; // The extension that will have this image
			IconTexture(ImTextureID newId, string newExtension) : id(newId), extension(newExtension){};
		};

		class Icon {
		  public:
			static std::map<std::string, IconTexture> textures;
			static void Init() {
				ImTextureID imageTextureID = 0;
				std::filesystem::path currentPath(__FILE__);
				std::string projectDirectory = currentPath.parent_path().parent_path().parent_path().string();
				std::cout << projectDirectory << std::endl;
				// Folder Icon
				LoadImageToImGuiTexture((projectDirectory + "/Images/FileIcons/folderIcon.png").c_str(),
										imageTextureID);
				textures.emplace("", IconTexture(imageTextureID, ""));

				// Icon for unsupported extensions
				LoadImageToImGuiTexture((projectDirectory + "/Images/FileIcons/blankFileIcon.png").c_str(),
										imageTextureID);
				textures.emplace(".notFound", IconTexture(imageTextureID, ".notFound"));

				// Back button Icon
				LoadImageToImGuiTexture((projectDirectory + "/Images/FileIcons/backArrowIcon.png").c_str(),
										imageTextureID);
				textures.emplace(".back", IconTexture(imageTextureID, ".back"));

				// Yaml Icon
				LoadImageToImGuiTexture((projectDirectory + "/Images/FileIcons/yamlIcon.png").c_str(), imageTextureID);
				textures.emplace(".yaml", IconTexture(imageTextureID, ".yaml"));

				// Obj Icon
				LoadImageToImGuiTexture((projectDirectory + "/Images/FileIcons/objIcon.png").c_str(), imageTextureID);
				textures.emplace(".obj", IconTexture(imageTextureID, ".obj"));

				// Model Icon
				LoadImageToImGuiTexture((projectDirectory + "/Images/FileIcons/modelIcon.png").c_str(), imageTextureID);
				textures.emplace(Standards::modelExtName, IconTexture(imageTextureID, Standards::modelExtName));

				// Scene Icon
				LoadImageToImGuiTexture((projectDirectory + "/Images/FileIcons/sceneIcon.png").c_str(), imageTextureID);
				textures.emplace(Standards::sceneExtName, IconTexture(imageTextureID, Standards::sceneExtName));
			}

			static void LoadImageToImGuiTexture(const char* path, ImTextureID& outTextureID);
		};
	} // namespace Editor
} // namespace Plaza
