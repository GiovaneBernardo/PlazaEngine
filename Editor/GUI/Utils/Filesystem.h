#pragma once
#include <string>
#include <filesystem>
#include "Editor/GUI/FileExplorer/FileExplorer.h"
namespace Plaza::Editor::Utils {
	class Filesystem {
	  public:
		/// <summary>
		/// If theres already a file with this name, includes a (number), so its always unique
		/// </summary>
		/// <param name="filePath">Full path to the desired file</param>
		/// <returns></returns>
		static std::string GetUnrepeatedName(std::string filePath) {
			std::filesystem::path path(filePath);
			if (!std::filesystem::exists(path))
				return path.filename().string();

			int counter = 0;
			std::string baseName = path.stem().string();
			std::string extension = path.extension().string();

			while (true) {
				std::string newFilename = baseName + " (" + std::to_string(counter) + ")" + extension;
				if (!std::filesystem::exists(path.parent_path().string() + "/" + newFilename))
					return newFilename;
				counter++;
			}
		}

		static std::string GetUnrepeatedPath(std::string filePath) {
			std::filesystem::path path(filePath);
			return path.parent_path().string() + "/" + GetUnrepeatedName(filePath);
		}

		static std::string CreateFolder(std::string fullPath) {
			std::string name = GetUnrepeatedPath(fullPath);
			std::filesystem::create_directory(name);
			return name;
		}

		static void CreateFolder(std::string path, std::string name) { CreateFolder(path + "/" + name); }

		static std::ofstream GetFileStream(std::string fullPath) {
			std::ofstream file(fullPath);
			return file;
		}

		static std::ofstream CreateNewFileStream(std::string fullPath) {
			const std::string name = GetUnrepeatedPath(fullPath);
			std::ofstream file(name);
			return file;
		}

		static std::string CreateNewFile(std::string fullPath) {
			const std::string name = GetUnrepeatedPath(fullPath);
			std::ofstream file(name);
			return name;
		}

		static void CreateNewFile(std::string path, std::string name) { CreateNewFile(path + "/" + name); }

		static void CreateNewFile(std::string path, std::string name, std::string extension) {
			CreateNewFile(path + "/" + name + "." + extension);
		}

		static void ChangeFileName(std::string fullPath, std::string newName) {
			if (filesystem::exists(fullPath)) {
				filesystem::path oldPath(fullPath);
				filesystem::path newPath = oldPath;
				newPath.replace_filename(newName);
				filesystem::rename(oldPath, newPath);
			}
		}

		static void DeleteFileF(std::string fullPath) {
			filesystem::path path(fullPath);
			if (filesystem::exists(path)) {
				std::filesystem::remove_all(path);
				Gui::canUpdateContent = true;
				// Gui::FileExplorer::UpdateContent(Gui::FileExplorer::currentDirectory);
			}
		}

		static void ChangeFolderName() {}
	};
} // namespace Plaza::Editor::Utils
