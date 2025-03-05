#pragma once
#include "Engine/Core/PreCompiledHeaders.h"
#include "ComponentsInspector.h"
#include "Engine/Components/Audio/AudioSource.h"
#include "Editor/GUI/Utils/Utils.h"
#include "Engine/Application/FileDialog/FileDialog.h"
#include "Engine/Core/Audio/Audio.h"
#include "Editor/GUI/Utils/DataVisualizer.h"
#include "Editor/GUI/FileExplorer/FileExplorer.h"
#include "Engine/Core/Scene.h"

namespace Plaza::Editor {
	void ComponentsInspector::AudioSourceInspector(Scene* scene, Entity* entity) {
		AudioSource* component = scene->GetComponent<AudioSource>(entity->uuid);
		if (Utils::ComponentInspectorHeader(component, "Audio Source")) {
			ImGui::PushID("AudioSourceInspector");

			ImGui::BeginTable("AudioSourceSettings", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg);

			if (Utils::AddTableButton("Select File", nullptr)) {
				// std::string filePath = FileDialog::OpenFileDialog(".mp3");
				Asset* asset = AssetsManager::GetAssetOrImport(FileDialog::OpenFileDialog(".mp3"), {},
															   Editor::Gui::FileExplorer::currentDirectory);
				if (asset) {
					Audio::Init();

					component->mAudioAssetUuid = asset->mAssetUuid;
					if (asset)
						component->LoadFile(asset->mAssetPath.string());
				}
				// component->LoadFile(filePath);
			}

			if (Utils::AddTableButton("Play Sound", nullptr)) {
				component->Play();
			}
			if (Utils::AddTableButton("Stop Sound", nullptr)) {
				component->Stop();
			}

			if (Utils::AddTableButton("Gain", nullptr)) {
				component->Play();
			}
			Utils::AddTableFloat("Pitch", &component->mPitch);

			// Utils::AddTableInt("Y", &mSettings.y);
			// Utils::AddTableInt("Z", &mSettings.z);
			// Utils::AddTableInt("Radius", &mSettings.radius);
			// Utils::AddTableFloat("Intensity", &mSettings.intensity);
			// Utils::AddTableInt("Repeat Interval", &mSettings.repeatInterval);
			ImGui::EndTable();

			if (ImGui::DragFloat("Gain", &component->mGain)) {
				component->SetGain(component->mGain);
			}

			if (ImGui::DragFloat("Pitch", &component->mPitch)) {
				component->SetPitch(component->mPitch);
			}

			if (ImGui::Checkbox("Spatial", &component->mSpatial)) {
				component->SetSpatial(component->mSpatial);
			}
			if (ImGui::Checkbox("Loop", &component->mLoop)) {
				component->SetLoop(component->mLoop);
			}

			ImGui::PopID();
		}
	}
} // namespace Plaza::Editor
