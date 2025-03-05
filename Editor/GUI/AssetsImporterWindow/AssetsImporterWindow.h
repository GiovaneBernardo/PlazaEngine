#pragma once
#include "Editor/GUI/GuiWindow.h"
#include "Engine/Core/AssetsManager/Importer/AssetsImporter.h"

namespace Plaza {
	namespace Editor {
		class AssetsImporterWindow : public GuiWindow {
		  public:
			AssetsImporterWindow(GuiLayer layer, bool startOpen = true) : GuiWindow(layer, startOpen) {}

			void Init() override;
			void Update(Scene* scene) override;

			void OpenAssetsImporter(std::string filePath, std::string outPath) {
				mFileToImport = filePath;
				outPath = mFileToImportOut;
				this->SetOpen(true);
			}

		  private:
			Plaza::AssetsImporterSettings settings{};

			std::string mFileToImport = "";
			std::string mFileToImportOut = "";
			// AssetsImporterSettings mSettings{};
			void OnKeyPress(int key, int scancode, int action, int mods) override;

			void ModelImporterMenu();
			void TextureImporterMenu();
		};
	} // namespace Editor
} // namespace Plaza
