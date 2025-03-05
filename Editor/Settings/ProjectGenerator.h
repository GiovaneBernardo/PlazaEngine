#pragma once
namespace Plaza::Editor {
	class ProjectGenerator {
	  public:
		static void PasteCmakeFile(const std::filesystem::path& directory);
		static void PasteGitIgnore(const std::filesystem::path& directory);
		static void PasteAllProjectFiles(const std::filesystem::path& directory);

		static void GenerateSolution(const std::string& solutionName, const std::string& projectName,
									 const std::string& outputDirectory);
		static void GenerateProject(const std::string& projectName, const std::string& outputDirectory);
	};
} // namespace Plaza::Editor
