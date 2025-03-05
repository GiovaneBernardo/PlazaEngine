#include "Engine/Core/PreCompiledHeaders.h"
#include "SettingsSerializer.h"
#include "EditorSettings.h"
namespace Plaza::Editor {
	void EditorSettingsSerializer::Serialize(std::string filePath) {
		if (filePath.empty()) {
			filePath = Application::Get()->enginePathAppData + "Settings.yaml";
		}
	}

	void EditorSettingsSerializer::DeSerialize(std::string filePath) {
		if (filePath.empty()) {
			filePath = Application::Get()->enginePathAppData + "Settings.yaml";
		}
		// this->ReapplyAllSettings();
	}
} // namespace Plaza::Editor
