#pragma once
#include "Engine/Core/PreCompiledHeaders.h"
#include "Editor/GUI/Utils/Utils.h"

namespace Plaza::Editor {
	// Serialize all lines into a file again
	static bool SerializeFileLines(std::string filePath, vector<std::string> lines) {
		std::ofstream file(filePath);
		if (file.is_open()) {
			for (const auto& line : lines) {
				file << line << '\n';
			}
			file.close();
			return true;
		}
		else {
			std::cout << "Unable to save file." << std::endl;
			return false;
		}
	}

	static void TextEditor(File* file) {
		static vector<std::string> lines;
		static std::string text;
		static File* lastFile;
		ImGui::Text(file->directory.c_str());
		if (lines.size() <= 0 || file != lastFile) {
			lines.clear();
			std::string path = file->directory;
			std::ifstream file(path); // Replace "example.txt" with your file path

			if (file.is_open()) {
				std::string line;
				while (std::getline(file, line)) {
					lines.push_back(line);
				}
				file.close();
			}
			else {
				std::cout << "Unable to open file." << std::endl;
			}
		}

		if (ImGui::Button("Apply")) {
			SerializeFileLines(file->directory, lines);
		}

		/* Make each line editable with imgui input text*/
		// if (lines.size() > 0) {
		//	for (int i = 0; i < lines.size(); ++i) {
		//		char buffer[2048];
		//		strncpy_s(buffer, lines[i].c_str(), sizeof(buffer));
		//		if (ImGui::InputText(("##Line" + std::to_string(i)).c_str(), buffer, sizeof(buffer))) {
		//			lines[i] = buffer;
		//		}
		//	}
		// }

		if (ImGui::Button("Apply")) {
			SerializeFileLines(file->directory, lines);
		}

		lastFile = file;
	}
}