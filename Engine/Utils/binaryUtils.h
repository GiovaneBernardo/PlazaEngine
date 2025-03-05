#pragma once
#include <string>
#include <fstream>

namespace Plaza {
	namespace Utils {
		static void SaveStringAsBinary(std::ofstream& stream, std::string string) {
			uint32_t nameLength = static_cast<uint32_t>(string.length());
			stream.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
			stream.write(string.c_str(), nameLength);
		}
		static std::string ReadBinaryString(std::ifstream& stream) {
			std::string content;
			uint32_t nameLength;
			stream.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));
			content.resize(nameLength);
			stream.read(&content[0], nameLength);
			return content;
		}
	} // namespace Utils
} // namespace Plaza
