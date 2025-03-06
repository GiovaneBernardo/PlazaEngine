#pragma once
#include "Editor/GUI/FileExplorer/File.h"
#include "Editor/GUI/guiMain.h"
#include "Engine/Core/AssetsManager/AssetsType.h"

namespace Plaza {
	namespace Editor {
		class Gui::FileExplorer {
		  public:
			static inline std::vector<AssetType> sAssetTypesOrder{
				AssetType::SCENE,
				AssetType::PREFAB,
				AssetType::SCRIPT,
				AssetType::SHADERS,
				AssetType::ANIMATION,
				AssetType::MATERIAL,
				AssetType::AUDIO,
				AssetType::TEXTURE,
				AssetType::MODEL,
				AssetType::MESH,
				AssetType::DLL,
				AssetType::METADATA,
				AssetType::UNKNOWN,
				AssetType::NONE
			};

			static inline int currentColumn = -1;
			static inline float lastY = 0.0f;
			static inline bool breakFilesLoop = false;
			static std::vector<std::unique_ptr<File>> files;
			static std::string currentDirectory;
			static void Init();
			static void UpdateGui();
			static void DrawFile(File* file);
			/// <summary>
			/// Read the project directory to get the present files
			/// </summary>
			static void UpdateContent(std::string folderPath);

		  private:
			static void OrderFiles(std::vector<std::unique_ptr<File>>& files);
			static int GetTypeOrderIndex(const AssetType& type);
		};
	} // namespace Editor
} // namespace Plaza
