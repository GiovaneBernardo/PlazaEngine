#pragma once
namespace Plaza::Editor {
	class EditorSettingsSerializer {
	  public:
		static void Serialize(std::string filePath = "");
		static void DeSerialize(std::string filePath = "");
	};
} // namespace Plaza::Editor
