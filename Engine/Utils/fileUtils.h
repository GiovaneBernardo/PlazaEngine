#pragma once
#include <string>
#include <vector>
#include <filesystem>
namespace Plaza {
	namespace FileUtils {
		bool startsWith(const std::string& str, const std::string& prefix) {
			return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
		}

		bool endsWith(const std::string& str, const std::string& suffix) {
			return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
		}

		int GetNumberOfSameNameFiles(std::string path, std::string name) {
			std::filesystem::path directory(path);
			int count = 0;

			for (const auto& entry : std::filesystem::directory_iterator(directory)) {
				std::string asd = entry.path().string();
				std::string fileName = entry.path().filename().string();
				if (fileName == name || (startsWith(fileName, name) && endsWith(fileName, ")"))) {
					count++;
				}
			}

			return count;
		}
	} // namespace FileUtils
} // namespace Plaza
