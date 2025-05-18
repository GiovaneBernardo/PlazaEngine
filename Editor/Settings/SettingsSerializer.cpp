#include "Engine/Core/PreCompiledHeaders.h"
#include "SettingsSerializer.h"
#include "EditorSettings.h"
namespace Plaza::Editor {
	void EditorSettingsSerializer::Serialize(std::string filePath) {
		if (filePath.empty()) {
			filePath = FilesManager::sEngineSettingsFolder.string() + "/Settings.yaml";
		}
	}

	void EditorSettingsSerializer::DeSerialize(std::string filePath) {
		if (filePath.empty()) {
			filePath = FilesManager::sEngineSettingsFolder.string() + "/Settings.yaml";
		}
		// this->ReapplyAllSettings();
	}
} // namespace Plaza::Editor
