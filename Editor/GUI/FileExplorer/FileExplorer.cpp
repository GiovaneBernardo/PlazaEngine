#include "Engine/Core/PreCompiledHeaders.h"
#include "FileExplorer.h"
#include "Editor/GUI/Popups/FileExplorerPopup.h"
#include "Engine/Core/Standards.h"
#include "Editor/GUI/Style/EditorStyle.h"
#include "Editor/GUI/guiMain.h"
#include "Editor/GUI/Popups/FileExplorerFilePopup.h"
#include "Editor/GUI/Popups/FileExplorerPopup.h"
#include "Editor/GUI/Utils/Filesystem.h"
#include "Editor/GUI/FileExplorer/FileIcon.h"

#include "Editor/GUI/FileExplorer/File.h"
#include "Editor/GUI/FileExplorer/Files/BackFile.h"
#include "Editor/GUI/FileExplorer/Files/FolderFile.h"
#include "Editor/GUI/FileExplorer/Files/MaterialFile.h"

unsigned int startSelected = 0;
namespace Plaza {
	namespace Editor {
		std::string Gui::FileExplorer::currentDirectory = "";
		void Gui::FileExplorer::UpdateGui() {
			PLAZA_PROFILE_SECTION("Update File Explorer");
			if (Gui::canUpdateContent) {
				FileExplorer::UpdateContent(Editor::Gui::FileExplorer::currentDirectory);
				Gui::canUpdateContent = false;
			}

			ApplicationSizes& appSizes = *Application::Get()->appSizes;
			ApplicationSizes& lastAppSizes = *Application::Get()->lastAppSizes;
			Entity* selectedGameObject = Editor::selectedGameObject;

			// Set the window to be the content size + header size
			ImGuiWindowFlags  sceneWindowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove;
			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

			ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoNavFocus;
			//windowFlags |= ImGuiWindowFlags_NoScrollbar;
			if (ImGui::Begin("File Explorer", &Gui::isFileExplorerOpen, windowFlags)) {
				if (ImGui::IsWindowFocused())
					Application::Get()->focusedMenu = "File Explorer";
				if (ImGui::IsWindowHovered())
					Application::Get()->hoveredMenu = "File Explorer";
				File::currentPos = ImVec2(-1.0f, 1.0f);

				FileExplorer::breakFilesLoop = false;

				ImVec2 clipStart = ImGui::GetCursorScreenPos();
				ImVec2 clipEnd = ImVec2(clipStart.x + ImGui::GetWindowWidth(), clipStart.y + ImGui::GetWindowHeight());

				currentColumn = -1;
				int lastColumn = -1;
				lastY = 0.0f;

				float padding = 5.0f;
				float fileExplorerX = glm::max((float)ImGui::GetWindowSize().x, 75.0f + padding);
				ImVec2 fileSize = ImVec2(75 + padding, 75 + padding);

				float horizontalItemsPerColumn = glm::floor((float)(fileExplorerX / fileSize.x));
				int horizontalItemsPerRow = glm::floor(fileExplorerX / fileSize.x);
				ImGui::BeginGroup();
				// Create all the icons
				ImGuiListClipper clipper;
				int totalRows = glm::ceil(files.size() / horizontalItemsPerRow);
				clipper.Begin(totalRows + 1);
				clipper.ItemsHeight = fileSize.y;

				while (clipper.Step()) {
					for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
						ImGui::BeginGroup();
						for (int column = 0; column < horizontalItemsPerRow; ++column) {
							int fileIndex = row * horizontalItemsPerRow + column;
							if (fileIndex < files.size()) {
								FileExplorer::DrawFile(files[fileIndex].get());
								ImGui::SameLine();
								if (FileExplorer::breakFilesLoop) {
									FileExplorer::breakFilesLoop = false;
									break;
								}
							}
						}
						ImGui::EndGroup();
					}
				}

				Popup::FileExplorerPopup::Update();

				ImGui::EndGroup();
			}

			ImGui::End();
			ImGui::PopStyleColor();
		}
		
		/* Read all files in a directory and push them to the files vector */
		std::vector<std::unique_ptr<File>> Gui::FileExplorer::files = std::vector<std::unique_ptr<File>>();
		void Gui::FileExplorer::UpdateContent(std::string folderPath) {
			Editor::selectedFiles.clear();
			namespace fs = std::filesystem;
			files.clear();
			// Back Button
			std::string currentDirectory = Gui::FileExplorer::currentDirectory;
			const std::string& currentDirectoryPath = filesystem::path{ currentDirectory }.string();
			files.push_back(make_unique<BackFile>(".back", currentDirectory + "/asd.back", ".back"));

			// Loop through all files found and create an icon on the file explorer
			for (const auto& entry : fs::directory_iterator(folderPath)) {
				std::string filename = entry.path().filename().string();
				std::string extension = entry.path().extension().string();
				if (entry.path().stem().string() == filename)
					extension = filename;

				// Call the respective constructor
				if (fs::is_directory(entry.path()))
					files.emplace_back(make_unique<FolderFile>(filename, entry.path().string(), ""));
				else if (extension == Standards::materialExtName)
					files.emplace_back(make_unique<MaterialFile>(filename, entry.path().string(), extension));
				else
					files.emplace_back(make_unique<File>(filename, entry.path().string(), extension));

			}
		}

		void Gui::FileExplorer::DrawFile(File* file) {
			if (file->currentPos.x == -1.0f) {
				file->currentPos = ImGui::GetCursorScreenPos();
			}
			ImGui::SetCursorScreenPos(file->currentPos);

			ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoFocusOnAppearing;
			windowFlags |= ImGuiWindowFlags_NoScrollbar;

			if (ImGui::BeginChild(ImGui::GetID(file->name.c_str()), ImVec2(75, 75), false, windowFlags)) {
				if (lastY != ImGui::GetCursorPosY()) {
					lastY = ImGui::GetCursorPosY();
					currentColumn++;
				}

				if (ImGui::IsWindowFocused())
					Application::Get()->focusedMenu = "File Explorer";
				if (ImGui::IsWindowHovered())
					Application::Get()->hoveredMenu = "File Explorer";

				ImVec4 backgroundColor = sEditorStyle->hierarchyBackgroundColor;

				if (ImGui::IsWindowHovered() || Editor::selectedFiles.contains(file->directory)) {
					backgroundColor = sEditorStyle->treeNodeHoverBackgroundColor;
				}

				if (ImGui::BeginDragDropSource()) {
					File* filePtr = new File(*file);
					ImGui::Text(filePtr->name.c_str());
					ImGui::Text(filePtr->directory.c_str());
					ImGui::SetDragDropPayload(Gui::scenePayloadName.c_str(), &filePtr, sizeof(Editor::File&));
					ImGui::EndDragDropSource();
				}
				// Draw background
				ImDrawList* drawList = ImGui::GetWindowDrawList();
				ImVec2 size = ImGui::imVec2(ImGui::glmVec2(ImGui::GetWindowPos()) + glm::vec2(75, 75));
				drawList->AddRectFilled(ImGui::GetWindowPos(), size, ImGui::ColorConvertFloat4ToU32(backgroundColor));

				// Image
				ImGui::SetCursorPos(ImGui::imVec2(ImGui::glmVec2(ImGui::GetCursorPos()) + glm::vec2(75 / 2 - 25)));
				ImGui::Image(ImTextureID(file->textureId), ImVec2(50, 50));
				ImGui::SetCursorPos(ImGui::imVec2(ImGui::glmVec2(ImGui::GetCursorPos()) - glm::vec2(0, 10)));
				if (file->changingName == file->name) {
					if (file->firstFocus) {
						ImGui::SetKeyboardFocusHere();
						startSelected = std::filesystem::path{ file->name }.stem().string().length();
					}

					char buf[1024];
					strcpy(buf, file->name.c_str());
					if (ImGui::InputTextEx("##FileNameInput", "File Name", buf, 512, ImVec2(50, 20), ImGuiInputTextFlags_EnterReturnsTrue, nullptr, nullptr, startSelected)) {
						file->changingName = "";
						Utils::Filesystem::ChangeFileName(Editor::Gui::FileExplorer::currentDirectory + "/" + file->name, Editor::Gui::FileExplorer::currentDirectory + "/" + buf);
					}

					if (!ImGui::IsItemActive() && !file->firstFocus) {
						file->changingName = "";
						Utils::Filesystem::ChangeFileName(Editor::Gui::FileExplorer::currentDirectory + "/" + file->name, Editor::Gui::FileExplorer::currentDirectory + "/" + buf);
						file->name = buf;
						Gui::FileExplorer::UpdateContent(Editor::Gui::FileExplorer::currentDirectory);
					}

					if (file->firstFocus) {
						file->firstFocus = false;
					}
				}
				else {
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
					ImGui::TextWrapped(file->name.c_str());
					ImGui::SameLine();
					ImGui::Text("...");
					ImGui::PopStyleVar();
					ImGui::PopStyleVar();
				}
			}
			Editor::Popup::FileExplorerFilePopup::Update(file);
			ImGui::EndChild();

			// Open, or add the file to the selected files map when user clicked on a file
			if (ImGui::IsItemClicked()) {
				Editor::selectedGameObject = nullptr;
				// Clicked on a folder and is not holding ctrl
				if (ImGui::IsMouseDoubleClicked(0)) {
					// Handle double click on folders
					file->DoubleClick();
					//if (filesystem::is_directory(filesystem::path{ file->directory }) && glfwGetKey(Application::Get()->Window->glfwWindow, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS) {
					//	Editor::Gui::FileExplorer::currentDirectory = file->directory;
					//	Gui::FileExplorer::UpdateContent(Gui::FileExplorer::currentDirectory);
					//	Editor::selectedFiles.clear();
					//} // Handle double click on .cs files
					//else if (filesystem::path{ file->directory }.extension() == ".cs") {
					//	/* Get Devenv path */
					//	std::string getDevenvCommand = (Application::Get()->enginePath + "/vendor/vsWhere/vswhere -latest -requires Microsoft.Component.MSBuild -find Common7/IDE/devenv.exe");
					//	// Open a pipe to capture the command output
					//	FILE* pipe = _popen(getDevenvCommand.c_str(), "r");
					//	if (!pipe) {
					//		std::cerr << "Error: Unable to execute the command." << std::endl;
					//	}
					//	char buffer[1024];
					//	std::string devenvPath = "";
					//
					//	// Read the command output character by character
					//	int c;
					//	while ((c = fgetc(pipe)) != EOF) {
					//		// Filter out newline characters
					//		if (c != '\n' && c != '\r') {
					//			devenvPath += static_cast<char>(c);
					//		}
					//	}
					//	// Close the pipe
					//	_pclose(pipe);
					//
					//	/* "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\devenv.exe" /command "File.OpenFile" "Speed Runers.sln" scriptchola.cs */
					//	std::string projectPath = std::filesystem::path{ Application::Get()->activeProject->directory + "/" + Application::Get()->activeProject->name }.replace_extension(".sln").string();
					//	std::string scriptPath = file->directory;
					//	std::string openCsFileCommand = "\"\"" + devenvPath + "\" \"" + projectPath + "\" \"" + scriptPath + "\"\"";
					//	for (size_t i = 0; i < openCsFileCommand.length(); ++i) {
					//		if (openCsFileCommand[i] == '/') {
					//			openCsFileCommand[i] = '/';
					//		}
					//	}
					//	std::cout << "OpenCsFileCommand: " << openCsFileCommand.c_str() << "\n";
					//	//system(openCsFileCommand.c_str());
					//
					//	FILE* pipe2 = _popen(openCsFileCommand.c_str(), "r");
					//	if (!pipe2) {
					//		std::cerr << "Error: Unable to execute the command." << std::endl;
					//	}
					//}
				}
				else {
					Editor::selectedGameObject = 0;
					// Clicked on something and is pressing control
					if (glfwGetKey(Application::Get()->mWindow->glfwWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
						// Check if this file is already in the selected files, if yes, then remove it from the map
						if (Editor::selectedFiles.contains(file->name)) {
							Editor::selectedFiles.erase(file->name);
						}
						else {
							// Its not on the selected files map, so just add it
							Editor::selectedFiles.emplace(file->directory, new File(*file));
						}
					}
					else { // Clicked without control
						Editor::selectedFiles.clear();
						Editor::selectedFiles.emplace(file->directory, new File(*file));
					}
				}
			}



			file->currentPos.x += file->iconSize + file->spacing;
			if (file->currentPos.x + file->iconSize > ImGui::GetWindowPos().x + ImGui::GetWindowWidth()) {
				file->currentPos.x = ImGui::GetCursorScreenPos().x;
				file->currentPos.y += file->iconSize + file->spacing;
			}
		}
	}
}