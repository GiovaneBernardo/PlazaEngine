#pragma once
#include <vector>
#include <string>

namespace Plaza {
	class FilesManager {
	  public:
		static std::filesystem::path CopyPasteFile(const std::filesystem::path& from, const std::filesystem::path& to,
												   bool override = true);
		static std::filesystem::path CreateFileCopy(const std::filesystem::path& from, bool override = true);
		static void CreateNewDirectory(const std::filesystem::path& path);

		static std::filesystem::path GetValidPath(const std::filesystem::path& path);
		static int GetNumberOfSameNameFiles(const std::filesystem::path& path, const std::string& fileName);

		static bool PathExists(const std::filesystem::path& path);
		static bool PathIsDirectory(const std::filesystem::path& path);
		static bool PathMustExist(const std::filesystem::path& path);

		static void CreateFileWithData(const std::filesystem::path& path, const char* data);

		static void OpenFile(const std::filesystem::path& path);
		static void OpenFileParentFolder(const std::filesystem::path& filePath);
		static void OpenFolder(const std::filesystem::path& path);
		static void SaveFile(const std::filesystem::path& path, void* data, size_t size);
	};
} // namespace Plaza
