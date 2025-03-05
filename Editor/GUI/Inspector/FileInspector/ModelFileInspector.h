#pragma once
#include "Engine/Core/PreCompiledHeaders.h"
#include "Editor/GUI/Utils/Utils.h"
#include "Engine/Application/Serializer/FileSerializer/FileSerializer.h"
#include "Engine/Core/ModelLoader/ModelLoader.h"
namespace Plaza::Editor {
	static class ModelFileInspector {
	  public:
		static Model* model;
		static File* lastFile;
		ModelFileInspector(File* file) {
			if (!model || file != lastFile) {
				model = ModelFileSerializer::DeSerialize(file->directory);
				if (model->uuid && AssetsManager::mMaterials.find(model->uuid) != AssetsManager::mMaterials.end())
					model = Application::Get()->.at(model->uuid).get();
			}
			ImGui::Text(file->directory.c_str());

			if (ImGui::Button("Apply")) {
				ModelFileSerializer::Serialize(file->directory, model);
			}

			lastFile = file;
		}
	};
} // namespace Plaza::Editor

inline Model* Plaza::Editor::ModelFileInspector::model = nullptr;
inline File* Plaza::Editor::ModelFileInspector::lastFile = nullptr;
