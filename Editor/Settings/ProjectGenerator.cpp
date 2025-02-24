#include "Engine/Core/PreCompiledHeaders.h"
#include "ProjectGenerator.h"
#include "Editor/GUI/Utils/Filesystem.h"
#include "Engine/Core/FilesManager/FilesManager.h"
#include <filesystem>
#include "Engine/Core/AssetsManager/AssetsManager.h"

namespace Plaza::Editor {
    void ProjectGenerator::PasteCmakeFile(const std::filesystem::path& directory) {
        std::filesystem::path cmakeFilePath = directory / "CMakeLists.txt";
        FilesManager::CreateFileWithData(cmakeFilePath, AssetsManager::GetEmbedResource<"Editor/DefaultAssets/NewProject/CMakeLists.txt">());
        std::filesystem::path cmakePresetsFilePath = directory / "CMakePresets.json";
        FilesManager::CreateFileWithData(cmakePresetsFilePath, AssetsManager::GetEmbedResource<"Editor/DefaultAssets/NewProject/CMakePresets.json">());
    }

    void ProjectGenerator::PasteGitIgnore(const std::filesystem::path& directory) {
        std::filesystem::path filePath = directory / ".gitignore";
        FilesManager::CreateFileWithData(filePath, AssetsManager::GetEmbedResource<"Editor/DefaultAssets/NewProject/.gitignore">());
    }

    void ProjectGenerator::PasteAllProjectFiles(const std::filesystem::path& directory) {
        ProjectGenerator::PasteCmakeFile(directory);
        ProjectGenerator::PasteGitIgnore(directory);
        std::filesystem::create_directory(directory / "bin");
    }

	void ProjectGenerator::GenerateSolution(const std::string& solutionName, const std::string& projectName, const std::string& outputDirectory) {
        // Specify the project name and Plaza root dir and also change the backslashes to forwardslesh
        const char* ProjectName = projectName.c_str();
        std::string PlazaRootDir = (Application::Get()->enginePath + "/../").c_str();
        for (size_t i = 0; i < PlazaRootDir.length(); ++i) {
            if (PlazaRootDir[i] == '/') {
                PlazaRootDir[i] = '/';
            }
        }

        // Read the content of default premake file
        std::string premakeDefaultFilePath = Application::Get()->editorPath + "/premakeDefaultProject.lua";
        std::ifstream sourceFile(premakeDefaultFilePath);
        if (!sourceFile.is_open()) {
            std::cerr << "Failed to open the default premake file." << std::endl;
        }

        std::string sourceContent((std::istreambuf_iterator<char>(sourceFile)),
            std::istreambuf_iterator<char>());
        sourceFile.close();

        // Open the destination file in append mode to preserve its content
        Editor::Utils::Filesystem::CreateNewFile(outputDirectory + "/premake5.lua");
        std::ofstream destinationFile(outputDirectory + "/premake5.lua", std::ios::app);
        if (destinationFile.is_open()) {
            // Write the project name and Plaza root dir
            std::string content = "local PlazaRootDir = \"" + PlazaRootDir + "\"\n" + "local ProjectName = \"" + ProjectName + "\"\n";
            destinationFile << content;
            std::cout << "Content written to the destination file." << std::endl;
        }
        else {
            std::cerr << "Failed to create or open the destination file." << std::endl;
        }

        // Write the content from the default premake file to destination file
        destinationFile << sourceContent;
        destinationFile.close();
        std::string premakeCommand = PlazaRootDir + "ThirdParty/premake/premake5.exe vs2022";

        // Execute the Premake command
        std::filesystem::current_path(outputDirectory);
        int result = system(premakeCommand.c_str());
    }

    void ProjectGenerator::GenerateProject(const std::string& projectName, const std::string& outputDirectory) {
        std::ofstream projectFile(outputDirectory + "/" + projectName + ".vcxproj");
        if (!projectFile.is_open()) {
            std::cerr << "Error opening project file." << std::endl;
            return;
        }

        projectFile << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
        projectFile << "<Project DefaultTargets=\"Build\" ToolsVersion=\"16.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n";
        projectFile << "    <ItemGroup Label=\"ProjectConfigurations\">\n";
        projectFile << "        <ProjectConfiguration Include=\"Debug|Win32\">\n";
        projectFile << "            <Configuration>Debug</Configuration>\n";
        projectFile << "            <Platform>Win32</Platform>\n";
        projectFile << "        </ProjectConfiguration>\n";
        projectFile << "    </ItemGroup>\n";
        projectFile << "    <PropertyGroup Label=\"Globals\">\n";
        projectFile << "        <ProjectGuid>{SOME_GUID}</ProjectGuid>\n";
        projectFile << "        <Keyword>Win32Proj</Keyword>\n";
        projectFile << "    </PropertyGroup>\n";
        projectFile << "    <Import Project=\"$(VCTargetsPath)/Microsoft.Cpp.Default.props\" />\n";
        projectFile << "    <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|Win32'\" Label=\"Configuration\">\n";
        projectFile << "        <ConfigurationType>Application</ConfigurationType>\n";
        projectFile << "        <UseDebugLibraries>true</UseDebugLibraries>\n";
        projectFile << "        <CharacterSet>Unicode</CharacterSet>\n";
        projectFile << "    </PropertyGroup>\n";
        projectFile << "    <Import Project=\"$(VCTargetsPath)/Microsoft.Cpp.props\" />\n";
        projectFile << "    <ImportGroup Label=\"ExtensionSettings\">\n";
        projectFile << "    </ImportGroup>\n";
        projectFile << "    <ImportGroup Label=\"Shared\">\n";
        projectFile << "    </ImportGroup>\n";
        projectFile << "    <ImportGroup Label=\"PropertySheets\" Condition=\"'$(Configuration)|$(Platform)'=='Debug|Win32'\">\n";
        projectFile << "    </ImportGroup>\n";
        projectFile << "    <PropertyGroup Label=\"UserMacros\" />\n";
        projectFile << "    <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|Win32'\">\n";
        projectFile << "    </PropertyGroup>\n";
        projectFile << "    <ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|Win32'\">\n";
        projectFile << "        <ClCompile>\n";
        projectFile << "            <PreprocessorDefinitions>WIN32;_DEBUG;_CONSOLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>\n";
        projectFile << "            <RuntimeLibrary>MultiThreadedDebug</RuntimeLibrary>\n";
        projectFile << "        </ClCompile>\n";
        projectFile << "        <Link>\n";
        projectFile << "            <SubSystem>Console</SubSystem>\n";
        projectFile << "        </Link>\n";
        projectFile << "    </ItemDefinitionGroup>\n";
        projectFile << "    <Import Project=\"$(VCTargetsPath)/Microsoft.Cpp.targets\" />\n";
        projectFile << "    <ImportGroup Label=\"ExtensionTargets\">\n";
        projectFile << "    </ImportGroup>\n";
        projectFile << "</Project>\n";
    }
}