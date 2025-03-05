#pragma once
#include "Engine/Core/PreCompiledHeaders.h"
#include "FilesManager.h"
#include <regex>

namespace Plaza {
	std::filesystem::path FilesManager::CopyPasteFile(const std::filesystem::path& from,
													  const std::filesystem::path& to, bool override) {
		if (!FilesManager::PathExists(from))
			return "";

		const std::filesystem::path& finalTo = override ? to : FilesManager::GetValidPath(to);
		std::filesystem::copy_file(from, finalTo,
								   override ? std::filesystem::copy_options::overwrite_existing |
												  std::filesystem::copy_options::recursive
											: std::filesystem::copy_options::recursive);
		return finalTo;
	}

	std::filesystem::path FilesManager::CreateFileCopy(const std::filesystem::path& from, bool override) {
		std::filesystem::path newDestinationPath = from;
		newDestinationPath =
			newDestinationPath.parent_path() / (from.stem().string() + "Copy" + from.extension().string());
		return FilesManager::CopyPasteFile(from, newDestinationPath, override);
	}

	void FilesManager::CreateNewDirectory(const std::filesystem::path& path) {
		if (!FilesManager::PathExists(path))
			std::filesystem::create_directory(path);
	}

	std::filesystem::path FilesManager::GetValidPath(const std::filesystem::path& path) {
		int filesWithSameNameCount = FilesManager::GetNumberOfSameNameFiles(path.parent_path(), path.stem().string());
		if (filesWithSameNameCount == 0)
			return path;
		else {
			std::string newFileName =
				path.stem().string() + "(" + std::to_string(filesWithSameNameCount) + ")" + path.extension().string();
			return path.parent_path() / newFileName;
		}
	}

	int FilesManager::GetNumberOfSameNameFiles(const std::filesystem::path& path, const std::string& fileName) {
		std::filesystem::path directory(path);
		int count = 0;

		for (const auto& entry : std::filesystem::directory_iterator(directory)) {
			std::string entryName = entry.path().stem().string();
			if (entryName == fileName || (entryName.starts_with(fileName) && entryName.ends_with(")"))) {
				count++;
			}
		}

		return count;
	}

	bool FilesManager::PathExists(const std::filesystem::path& path) { return std::filesystem::exists(path); }

	bool FilesManager::PathIsDirectory(const std::filesystem::path& path) {
		return std::filesystem::is_directory(path);
	}

	bool FilesManager::PathMustExist(const std::filesystem::path& path) {
		if (!FilesManager::PathExists(path)) {
			PL_CORE_CRITICAL("Path does not exists");
			return false;
		}
		return true;
	}

	void FilesManager::CreateFileWithData(const std::filesystem::path& path, const char* data) {
		std::ofstream file(path);
		if (file.is_open()) {
			std::string content(data);
			content = std::regex_replace(content, std::regex("\r\n"), "\n");
			file << content;
			file.close();
		}
		else {
			PL_CORE_ERROR("Failed to open file");
		}
	}

	void FilesManager::OpenFile(const std::filesystem::path& path) {
#ifdef WIN32
		ShellExecuteA(NULL, "open", path.c_str(), NULL, NULL, SW_SHOWDEFAULT);
#elif __linux__
		std::string command = "xdg-open" + path.string() + " &";
		std::system(command.c_str());
#endif
	}

	void FilesManager::OpenFileParentFolder(const std::filesystem::path& filePath) {
		FilesManager::OpenFolder(std::string("\"" + filePath.parent_path().string() + "\"").c_str());
	}

	void FilesManager::OpenFolder(const std::filesystem::path& path) {
#ifdef WIN32
		ShellExecuteA(NULL, "open", path.c_str(), NULL, NULL, SW_SHOWDEFAULT);
#elif __linux__
		std::string command = "xdg-open" + path.string() + " &";
		std::system(command.c_str());
#endif
	}

	void FilesManager::SaveFile(const std::filesystem::path& path, void* data, size_t size) {}
} // namespace Plaza
