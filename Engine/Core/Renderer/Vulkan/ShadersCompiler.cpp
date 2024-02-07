#include "Engine/Core/PreCompiledHeaders.h"
#include "ShadersCompiler.h"

namespace Plaza {
	std::string VulkanShadersCompiler::mGlslcExePath = "";
    std::string VulkanShadersCompiler::mDefaultOutDirectory = "";
    std::string VulkanShadersCompiler::Compile(std::string shadersPath, std::string outDirectory) {
        std::string shadersName = std::filesystem::path{ shadersPath }.filename().string();
        std::string command = "\" " "\"" + mGlslcExePath + "\" \"" + shadersPath + "\" -o \"" + outDirectory + shadersName + ".spv\"" " \"";

        int result = system(command.c_str());
        if (result == 0) {
            std::cout << "Shader compilation successful." << std::endl;
        }
        else {
            std::cerr << "Shader compilation failed." << std::endl;
        }
        return outDirectory + shadersName + ".spv";
    }

}