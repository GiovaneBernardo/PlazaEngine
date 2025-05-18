#include "Engine/Core/PreCompiledHeaders.h"
#include "MenuBar.h"
#include "Engine/Application/FileDialog/FileDialog.h"
#include "Engine/Core/Standards.h"
#include "Editor/SessionCache/Cache.h"
#include "Engine/Core/AssetsManager/Loader/AssetsLoader.h"
#include "Engine/Core/AssetsManager/AssetsReader.h"
#include "Editor/Settings/ProjectGenerator.h"
#include "Engine/Core/Scene.h"
#include "Engine/Core/AssetsManager/Serializer/AssetsSerializer.h"

namespace Plaza {
	namespace Editor {
		void CopyDirectory(const std::filesystem::path& source, const std::filesystem::path& destination) {
			try {
				if (!std::filesystem::exists(destination)) {
					std::filesystem::create_directories(destination);
				}

				for (const auto& entry : std::filesystem::recursive_directory_iterator(source)) {
					const auto& path = entry.path();
					auto relativePath = std::filesystem::relative(path, source);
					std::filesystem::path destPath = destination / relativePath;

					try {
						if (std::filesystem::is_directory(path)) {
							std::filesystem::create_directories(destPath);
						}
						else if (std::filesystem::is_regular_file(path)) {
							std::filesystem::copy_file(path, destPath,
													   std::filesystem::copy_options::overwrite_existing);
							std::cout << "Copied: " << path << " to " << destPath << '\n';
						}
					} catch (const std::filesystem::filesystem_error& e) {
						std::cerr << "Skipped file (in use or inaccessible): " << path << "\n";
					}
				}
			} catch (const std::filesystem::filesystem_error& e) {
				std::cerr << "Error: " << e.what() << '\n';
			}
		}

		void Gui::MainMenuBar::Begin() {
			// MenuBar / TitleBar
			ImGui::BeginMainMenuBar();
			if (ImGui::BeginMenu("File")) {
				if (ImGui::Button("Build")) {
					std::string devEnv =
						" \"C:/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/msbuild.exe\" ";
					std::string outPath = "\"" + Application::Get()->projectPath + "/../" +
										  Application::Get()->activeProject->mAssetName + "build//\"";
					std::string command = "\"" + devEnv + "\"" + FilesManager::sEngineFolder.string() +
										  "/OpenGLEngine.sln\" /p:Configuration=GameRel /t:Build "
										  "/p:PROJECT_NAME=YourMacroValue /p:OutDir=" +
										  outPath + "\"";
					std::cout << command << std::endl;
					system(command.c_str());

					std::string newMainFolderPath = Application::Get()->projectPath + "/../" +
													Application::Get()->activeProject->mAssetName + "build/";

					/* Copy assets and scripts into Content Folder inside build folder */
					CopyDirectory(Application::Get()->projectPath, newMainFolderPath);

					EngineSettings oldSettings = Application::Get()->mSettings;
					EngineSettings newSettings;
					Application::GetGameModeDefaultSettings(newSettings);

					Application::Get()->mSettings = newSettings;

					/* Iterate over all copied assets and convert them to binary */
					for (auto [key, asset] : AssetsManager::mAssets) {
						std::string oldPath = asset->mAssetPath.string();
						asset->mAssetPath = asset->mAssetPath.string().replace(
							0, Application::Get()->projectPath.size(), newMainFolderPath);

						bool assetContainsMetaData = asset->GetExtension() != Standards::metadataExtName &&
													 AssetsManager::AssetContainsMetaData(asset);
						if (assetContainsMetaData) {
							std::filesystem::path oldMetadataPath = oldPath;
							oldMetadataPath.replace_extension(Standards::metadataExtName);
							std::shared_ptr<Metadata::MetadataStructure> metaDataContent =
								AssetsSerializer::DeSerializeFile<Metadata::MetadataStructure>(
									AssetsManager::GetAssetMetaDataPath(oldMetadataPath).string(),
									oldSettings.mMetaDataSerializationMode);
							asset->mAssetPath.replace_extension(Standards::metadataExtName);
							AssetsSerializer::SerializeFile<Metadata::MetadataStructure>(
								*metaDataContent.get(), asset->mAssetPath.string(),
								newSettings.mMetaDataSerializationMode);
						}
						else {
							if (asset->GetExtension() == Standards::sceneExtName) {
								Asset oldAsset = *asset;
								// asset = new Scene(*AssetsSerializer::DeSerializeFile<Scene>(oldPath,
								// oldSettings.mSceneSerializationMode).get());
								AssetsSerializer::SerializeFile<Scene>(
									*static_cast<Scene*>(AssetsSerializer::DeSerializeFile<Scene>(
															 oldPath, oldSettings.mSceneSerializationMode)
															 .get()),
									oldAsset.mAssetPath.string(),
									Application::Get()->mSettings.mSceneSerializationMode);
							}
							else
								AssetsSerializer::SerializeAssetByExtension(asset);
						}
						asset->mAssetPath = oldPath;
					}

					Application::Get()->mSettings = oldSettings;
				}

				if (ImGui::Button("Return to Project Menu")) {
					Editor::selectedGameObject = nullptr;
					Application::Get()->runEngine = false;
					Application::Get()->runProjectManagerGui = true;
					Application::Get()->activeProject = std::make_unique<Project>();
					Cache::Serialize(FilesManager::sEngineSettingsFolder.string() + "/cache.yaml");
					Application::Get()->focusedMenu = "ProjectManager";
				}

				if (ImGui::Button("New Scene")) {
					std::string newPath =
						FileDialog::SaveFileDialog(("Engine (*.%s)", Standards::sceneExtName).c_str());
					if (!newPath.empty()) {
						Editor::selectedGameObject = nullptr;
						Scene::SetEditorScene(*new std::shared_ptr<Scene>(AssetsManager::NewAsset<Scene>(newPath)));
						Scene::SetActiveScene(Scene::GetEditorScene());
						Scene::GetActiveScene()->mainSceneEntity = new Entity("Scene");
						Scene::GetActiveScene()->mainSceneEntity->parentUuid =
							Scene::GetActiveScene()->mainSceneEntity->uuid;

						AssetsSerializer::SerializeFile<Scene>(*Scene::GetEditorScene(), newPath,
															   Application::Get()->mSettings.mSceneSerializationMode);
						Application::Get()->activeProject->mLastSceneUuid = Scene::GetEditorScene()->mAssetUuid;
						AssetsSerializer::SerializeFile<Project>(
							*Application::Get()->activeProject, Application::Get()->activeProject->mAssetPath.string(),
							Application::Get()->mSettings.mProjectSerializationMode);
						AssetsLoader::LoadScene(AssetsSerializer::DeSerializeFile<Asset>(
													newPath, Application::Get()->mSettings.mSceneSerializationMode)
													.get(),
												Application::Get()->mSettings.mSceneSerializationMode);
					}
				}
				if (ImGui::Button("Save Scene")) {
					if (!Scene::GetActiveScene()->mAssetPath.empty() &&
						std::filesystem::exists(Scene::GetActiveScene()->mAssetPath)) {
						AssetsSerializer::SerializeFile(
							*Scene::GetActiveScene(),
							AssetsManager::GetAsset(Scene::GetActiveScene()->mAssetUuid)->mAssetPath.string(),
							Application::Get()->mSettings.mSceneSerializationMode);
						Application::Get()->activeProject->mLastSceneUuid = Scene::GetActiveScene()->mAssetUuid;
						AssetsSerializer::SerializeFile<Project>(
							*Application::Get()->activeProject, Application::Get()->activeProject->mAssetPath.string(),
							Application::Get()->mSettings.mProjectSerializationMode);
					}
				}
				if (ImGui::Button("Save Scene As...")) {
					std::string path = FileDialog::SaveFileDialog(("Engine (*.%s)", Standards::sceneExtName).c_str());
					if (!path.empty()) {
						Asset* asset = AssetsManager::GetAsset(Scene::GetActiveScene()->mAssetUuid);
						if (!asset) {
							AssetsSerializer::SerializeFile<Scene>(
								*Scene::GetActiveScene(), path, Application::Get()->mSettings.mSceneSerializationMode);
							asset = AssetsReader::ReadAssetAtPath(path);
						}
						bool sceneFileIsEditorScene = path == asset->mAssetPath.string();

						Scene::GetActiveScene()->mAssetPath = path;
						Scene::GetActiveScene()->mAssetName = std::filesystem::path{path}.filename().string();
						AssetsSerializer::SerializeFile(*Scene::GetActiveScene(), path,
														Application::Get()->mSettings.mSceneSerializationMode);

						Application::Get()->activeProject->mLastSceneUuid = Scene::GetActiveScene()->mAssetUuid;
						AssetsSerializer::SerializeFile<Project>(
							*Application::Get()->activeProject, Application::Get()->activeProject->mAssetPath.string(),
							Application::Get()->mSettings.mProjectSerializationMode);

						AssetsLoader::LoadScene(AssetsSerializer::DeSerializeFile<Asset>(
													path, Application::Get()->mSettings.mSceneSerializationMode)
													.get(),
												Application::Get()->mSettings.mSceneSerializationMode);
					}
				}
				if (ImGui::Button("Open Scene")) {
					std::string path = FileDialog::OpenFileDialog(("Engine (*.%s)", Standards::sceneExtName).c_str());
					if (std::filesystem::exists(path)) {
						Scene::SetEditorScene(AssetsSerializer::DeSerializeFile<Scene>(
							path, Application::Get()->mSettings.mSceneSerializationMode));
						Scene::SetActiveScene(Scene::GetEditorScene());
						Scene::GetActiveScene()->RecalculateAddedComponents();
					}
				}

				if (ImGui::BeginMenu("Project")) {
					if (ImGui::BeginMenu("Update Project Files")) {
						if (ImGui::MenuItem("Update All"))
							ProjectGenerator::PasteAllProjectFiles(
								Application::Get()->activeProject->mAssetPath.parent_path());
						if (ImGui::MenuItem("Update CMake file"))
							ProjectGenerator::PasteCmakeFile(
								Application::Get()->activeProject->mAssetPath.parent_path());
						if (ImGui::MenuItem("Update Git Ignore"))
							ProjectGenerator::PasteGitIgnore(
								Application::Get()->activeProject->mAssetPath.parent_path());
						ImGui::EndMenu();
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}

			// Create an invisible margin to align the buttons to the right
			float spacingX = Application::Get()->appSizes->appSize.x - 150;
			ImGui::Dummy(ImVec2(spacingX, 0));

			// Create the right corner buttons
			ImGui::SameLine();
			ImVec2 buttonSize = ImVec2(25, 25);
			if (ImGui::Button("-", buttonSize)) {
				glfwIconifyWindow(Application::Get()->mWindow->glfwWindow);
			}
			ImGui::SameLine();
			if (ImGui::Button("□", buttonSize)) {
				glfwMaximizeWindow(Application::Get()->mWindow->glfwWindow);
			}
			ImGui::SameLine();
			if (ImGui::Button("X", buttonSize)) {
				glfwSetWindowShouldClose(Application::Get()->mWindow->glfwWindow, true);
			}

			ImGui::EndMainMenuBar();
		}
	} // namespace Editor
} // namespace Plaza
