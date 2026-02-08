#pragma once
#include "Engine/Core/PreCompiledHeaders.h"
#include "SceneFile.h"
#include "Editor/GUI/FileExplorer/FileExplorer.h"
#include "Engine/Application/FileDialog/FileDialog.h"
#include "Engine/Core/Scene.h"
#include "Engine/Core/AssetsManager/Serializer/AssetsSerializer.h"

namespace Plaza {
	namespace Editor {
		void SceneFile::DoubleClick() {
			std::string path = this->directory;
			if (std::filesystem::exists(path)) {
				Scene::SetEditorScene(AssetsSerializer::DeSerializeFile<Scene>(
					path, Application::Get()->mSettings.mSceneSerializationMode));
				Scene::SetActiveScene(Scene::GetEditorScene());
				Scene::GetActiveScene()->RecalculateAddedComponents();
			}
		}

		void SceneFile::Delete() {}

		void SceneFile::Rename(std::string oldPath, std::string newPath) {}

		void SceneFile::Move(std::string oldPath, std::string newPath) {}

		void SceneFile::Copy() {}

		void SceneFile::Paste() {}

		void SceneFile::Popup() {

			if (ImGui::MenuItem("Load Scene")) {
				std::string path = this->directory;
				if (std::filesystem::exists(path)) {
					Scene::SetEditorScene(AssetsSerializer::DeSerializeFile<Scene>(
						path, Application::Get()->mSettings.mSceneSerializationMode));
					Scene::SetActiveScene(Scene::GetEditorScene());
					Scene::GetActiveScene()->RecalculateAddedComponents();
				}
			}

			if (ImGui::MenuItem("Set as default scene")) {
				Application::Get()->activeProject->mLastSceneUuid = AssetsManager::GetAsset(directory)->mAssetUuid;
				AssetsSerializer::SerializeFile(*Application::Get()->activeProject,
												Application::Get()->activeProject->mAssetPath.string(),
												Application::Get()->mSettings.mProjectSerializationMode);
			}
		}
	} // namespace Editor
} // namespace Plaza
