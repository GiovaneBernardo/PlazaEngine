#include "Engine/Core/PreCompiledHeaders.h"
#include "ShadersCompiler.h"

namespace Plaza {
	std::string VulkanShadersCompiler::mGlslcExePath = "";
	std::string VulkanShadersCompiler::mDefaultOutDirectory = "";
	std::string VulkanShadersCompiler::Compile(std::string shadersPath, std::string outDirectory) {
		if (!std::filesystem::exists(std::filesystem::path{outDirectory}))
			std::filesystem::create_directory(outDirectory);
		std::string shadersName = std::filesystem::path{shadersPath}.filename().string();

#ifdef EDITOR_MODE
		std::string command = mGlslcExePath + " \"" + shadersPath + "\" -o \"" + outDirectory + shadersName + ".spv\"";
		int result = system(command.c_str());
		if (result == 0) {
			std::cout << "Shader compilation successful." << std::endl;
		}
		else {
			std::cerr << "Shader compilation failed." << std::endl;
		}
#endif
		return outDirectory + shadersName + ".spv";
	}

} // namespace Plaza
