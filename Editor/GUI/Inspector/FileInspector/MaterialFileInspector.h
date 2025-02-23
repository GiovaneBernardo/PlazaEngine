#pragma once
#include "Engine/Core/PreCompiledHeaders.h"
#include "Engine/Components/Rendering/Material.h"
#include "Editor/GUI/Utils/Utils.h"
#include "Engine/Core/Renderer/Renderer.h"
#include "Engine/Application/FileDialog/FileDialog.h"
#include "Editor/GUI/FileExplorer/File.h"
#include "Editor/GUI/Utils/DataVisualizer.h"
#include "Engine/Core/AssetsManager/Serializer/AssetsSerializer.h"

namespace Plaza::Editor {
		std::shared_ptr<Texture> LoadTextureButton(std::string outDirectory = "") {
			Asset* asset = AssetsManager::GetAssetOrImport(FileDialog::OpenFileDialog(".jpeg"), {}, outDirectory);

			if (asset)
				return std::shared_ptr<Texture>(Application::Get()->mRenderer->LoadTexture(asset->mAssetPath.string(), asset->mAssetUuid));
			else
				return std::shared_ptr<Texture>(AssetsManager::mTextures.find(1)->second);
		}

		static void MaterialFileInspector(Material* material) {
			//if (!material || file->directory != lastFile->directory) {
			//	if (AssetsManager::GetAsset(file->directory))
			//	{
			//		auto materialIt = AssetsManager::mMaterials.find(AssetsManager::GetAsset(file->directory)->mAssetUuid);
			//		if (materialIt != AssetsManager::mMaterials.end())
			//			material = materialIt->second.get();
			//	}
			//}

			if (!material)
			{
				ImGui::Text("Material not found");
				return;
			}

			//ImGui::Text(file->directory.c_str());
			ImGui::Text(material->mAssetPath.filename().string().c_str());

			ImGui::BeginTable("MaterialFileInspectorTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg);
			if (Utils::AddTableTexture("Diffuse: ", material->diffuse.get(), ImGuiSliderFlags_None))
				material->diffuse = LoadTextureButton(std::filesystem::path{ material->mAssetPath }.parent_path().string());
			if (Utils::AddTableTexture("Normal: ", material->normal.get(), ImGuiSliderFlags_None))
				material->normal = LoadTextureButton(std::filesystem::path{ material->mAssetPath }.parent_path().string());
			if (Utils::AddTableTexture("Roughness: ", material->roughness.get(), ImGuiSliderFlags_None))
				material->roughness = LoadTextureButton(std::filesystem::path{ material->mAssetPath }.parent_path().string());
			if (Utils::AddTableTexture("Metalness: ", material->metalness.get(), ImGuiSliderFlags_None))
				material->metalness = LoadTextureButton(std::filesystem::path{ material->mAssetPath }.parent_path().string());
			if (Utils::AddTableTexture("Height: ", material->height.get(), ImGuiSliderFlags_None))
				material->height = LoadTextureButton(std::filesystem::path{ material->mAssetPath }.parent_path().string());
			ImGui::EndTable();

			ImGui::InputFloat("Flip X", &material->flip.x);
			ImGui::InputFloat("Flip Y", &material->flip.y);

			if (ImGui::Button("Apply")) {
				//material->mAssetName = std::filesystem::path{ file->directory }.stem().string();
				AssetsSerializer::SerializeMaterial(material, material->mAssetPath, Application::Get()->mSettings.mMaterialSerializationMode);
			}

			//lastFile = file;
		}
}

//inline Material* Plaza::Editor::MaterialFileInspector::material = nullptr;
//inline Editor::File* Plaza::Editor::MaterialFileInspector::lastFile = nullptr;