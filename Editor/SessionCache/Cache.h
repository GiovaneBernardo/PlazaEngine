#pragma once
namespace Plaza::Editor {
	class Cache {
	  public:
		std::string mLastOpenProjectPath = "";
		std::string mLastOpenProjectName = "";
		static void Serialize(const std::string filePath);
		static Cache DeSerialize(const std::string filePath);

		static void Load();

		template <class Archive> void serialize(Archive& archive) {
			archive(mLastOpenProjectPath, mLastOpenProjectName);
		}
	};
} // namespace Plaza::Editor
