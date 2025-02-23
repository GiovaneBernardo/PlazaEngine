#include "Engine/Core/PreCompiledHeaders.h"
#include "FilesManager.h"

#ifdef WIN32
#include <shellapi.h>
#elif _linux__
#include <cstdlib>
#endif

namespace Plaza {
    class FilesManager {
        void OpenFile(const std::filesystem::path& path) {
#ifdef WIN32
            ShellExecuteA(NULL, "open", path.c_str(), NULL, NULL, SW_SHOWDEFAULT);
#elif __linux__
            std::string command = "xdg-open" + path.string() + " &";
            std::system(command.c_str());
#endif

        }

        void OpenFileParentFolder(const std::filesystem::path& filePath) {
            FilesManager::OpenFolder(std::string("\"" + filePath.parent_path().string() + "\"").c_str());
        }

        void OpenFolder(const std::filesystem::path& path) {
#ifdef WIN32
            ShellExecuteA(NULL, "open", path.c_str(), NULL, NULL, SW_SHOWDEFAULT);
#elif __linux__
            std::string command = "xdg-open" + path.string() + " &";
            std::system(command.c_str());
#endif
        }

        void SaveFile(const std::filesystem::path& path, void* data, size_t size) {

        }
    };
}

/*
        if (!Editor::selectedFiles.contains(file->name)) {
            Editor::selectedFiles.clear();
            Editor::selectedFiles.emplace(file->directory, new File(*file));
        }

        if (ImGui::MenuItem("Open")) {
            ShellExecuteA(NULL, "open", std::string("\"" + file->directory + "\"").c_str(), NULL, NULL, SW_SHOWDEFAULT);
        }

        if (ImGui::MenuItem("Open in file explorer")) {
            ShellExecuteA(NULL, "open", std::string("\"" + std::filesystem::path{ file->directory }.parent_path().string() + "\"").c_str(), NULL, NULL, SW_SHOWDEFAULT);
        }

        if (ImGui::MenuItem("Rename")) {
            Editor::File::changingName = Editor::selectedFiles.begin()->second->name;
            Editor::File::firstFocus = true;
        }

        if (ImGui::MenuItem("Delete")) {
            for (const auto& [key, value] : Editor::selectedFiles) {
                Utils::Filesystem::DeleteFileF(value->directory);
            }
        }
*/