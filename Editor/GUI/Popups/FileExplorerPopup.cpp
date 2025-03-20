#include "Engine/Core/PreCompiledHeaders.h"
#include "FileExplorerPopup.h"
#include "Editor/GUI/Utils/Filesystem.h"
#include "Editor/ScriptManager/ScriptManager.h"
#include "Editor/GUI/FileExplorer/File.h"
#include "Engine/Core/AssetsManager/Metadata/Metadata.h"
#include "Engine/Core/AssetsManager/AssetsReader.h"
#include "Engine/Core/Scripting/CppScript.h"
#include "Engine/Core/Scripting/Lua/LuaScript.h"
#include "Engine/Core/AssetsManager/Serializer/AssetsSerializer.h"

namespace Plaza::Editor {
	void Popup::FileExplorerPopup::UpdateContent() {
		if (ImGui::BeginMenu("Create")) {
			if (ImGui::MenuItem("Folder")) {
				Editor::File::changingName =
					Utils::Filesystem::CreateFolder(Gui::FileExplorer::currentDirectory + "/Unnamed");
				Gui::FileExplorer::UpdateContent(Gui::FileExplorer::currentDirectory);
				Editor::File::changingName = std::filesystem::path{Editor::File::changingName}.filename().string();
				// Editor::File::changingName = "Unnamed";
				Editor::File::firstFocus = true;
			}

			if (ImGui::BeginMenu("Script")) {
				if (ImGui::MenuItem("C++ Script")) {
					Editor::File::changingName =
						Utils::Filesystem::CreateNewFile(Gui::FileExplorer::currentDirectory + "/Unnamed" + ".h");
					Editor::File::changingName = std::filesystem::path{Editor::File::changingName}.filename().string();
					Editor::File::firstFocus = true;
					CppScript* script = new CppScript();
					script->mAssetUuid = Plaza::UUID::NewUUID();
					script->mAssetName = Editor::File::changingName;

					script->mAssetPath = Gui::FileExplorer::currentDirectory + "/" + Editor::File::changingName;

					Metadata::CreateMetadataFile(script);

					std::filesystem::path metaDataPath = script->mAssetPath;
					metaDataPath.replace_extension(Standards::metadataExtName);
					AssetsReader::ReadAssetAtPath(metaDataPath);
				}

				if (ImGui::MenuItem("Lua Script")) {
					Editor::File::changingName =
						Utils::Filesystem::CreateNewFile(Gui::FileExplorer::currentDirectory + "/Unnamed" + ".lua");
					Editor::File::changingName = std::filesystem::path{Editor::File::changingName}.filename().string();
					Editor::File::firstFocus = true;
					LuaScript* script = new LuaScript();
					script->mAssetUuid = Plaza::UUID::NewUUID();
					script->mAssetName = Editor::File::changingName;

					script->mAssetPath = Gui::FileExplorer::currentDirectory + "/" + Editor::File::changingName;

					Metadata::CreateMetadataFile(script);

					std::filesystem::path metaDataPath = script->mAssetPath;
					metaDataPath.replace_extension(Standards::metadataExtName);
					AssetsReader::ReadAssetAtPath(metaDataPath);
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Rendering")) {
				if (ImGui::MenuItem("Material")) {
					Editor::File::changingName = Utils::Filesystem::CreateNewFile(
						Gui::FileExplorer::currentDirectory + "/Unnamed" + Standards::materialExtName);
					Editor::File::changingName = std::filesystem::path{Editor::File::changingName}.filename().string();
					Editor::File::firstFocus = true;
					Material* material = new Material();
					material->mAssetUuid = Plaza::UUID::NewUUID();
					// AssetsManager::mMaterials.emplace(material.uuid, std::make_shared<Material>(material));
					AssetsManager::AddMaterial(material);
					std::string path = Gui::FileExplorer::currentDirectory + "/" + Editor::File::changingName;
					AssetsManager::NewAsset<Material>(material->mAssetUuid, path);
					AssetsSerializer::SerializeMaterial(material, path,
														Application::Get()->mSettings.mMaterialSerializationMode);
					// MaterialFileSerializer::Serialize(path, material);
				}

				if (ImGui::MenuItem("Shaders")) {
					Editor::File::changingName =
						Utils::Filesystem::CreateNewFile(Gui::FileExplorer::currentDirectory + "/Unnamed" + ".vert");
					Editor::File::changingName = std::filesystem::path{Editor::File::changingName}.filename().string();
					Editor::File::firstFocus = true;
					Asset* shadersAsset = new Asset();
					shadersAsset->mAssetUuid = Plaza::UUID::NewUUID();
					shadersAsset->mAssetName = Editor::File::changingName;

					shadersAsset->mAssetPath = Gui::FileExplorer::currentDirectory + "/" + Editor::File::changingName;

					Metadata::CreateMetadataFile(shadersAsset);

					std::filesystem::path metaDataPath = shadersAsset->mAssetPath;
					metaDataPath.replace_extension(Standards::metadataExtName);
					AssetsReader::ReadAssetAtPath(metaDataPath);
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}
	}
	void Popup::FileExplorerPopup::Update() {
		if (ImGui::BeginPopupContextWindow("FileExplorerPopup")) {
			UpdateContent();
			ImGui::EndPopup();
		}
	}
} // namespace Plaza::Editor
