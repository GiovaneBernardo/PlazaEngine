#pragma once
#include "Engine/Core/PreCompiledHeaders.h"
#include "ComponentsInspector.h"
#include "Engine/Components/Rendering/Material.h"
#include "Engine/Components/Core/Entity.h"
#include "Editor/GUI/Utils/Utils.h"
#include "Editor/GUI/Inspector/FileInspector/MaterialFileInspector.h"
#include "Editor/GUI/ContextWindows/SearchContext.h"
#include "Engine/Core/Scene.h"

namespace Plaza::Editor {
	static inline SearchContext* sMaterialsSearch = new SearchContext("MaterialSearchContext");

	void ComponentsInspector::MeshRendererInspector(Scene* scene, Entity* entity) {
		if (Utils::ComponentInspectorHeader(scene->GetComponent<MeshRenderer>(entity->uuid), "Mesh Renderer")) {
			ImGui::PushID("MeshRendererInspector");
			MeshRenderer* meshRenderer = scene->GetComponent<MeshRenderer>(entity->uuid);

			if (ImGui::TreeNodeEx("Materials List", ImGuiTreeNodeFlags_DefaultOpen)) {
				unsigned int index = 0;
				for (Material* material : meshRenderer->GetMaterials()) {
					std::string idString = std::to_string(material->mAssetUuid) + "meshRendererMaterial";
					ImGui::PushID(idString.c_str());
					std::string treeNodeName = std::to_string(index) + ": " + material->mAssetName;
					bool treeNodeOpen = ImGui::TreeNode(treeNodeName.c_str());
					ImGui::SameLine();
					if (ImGui::Button("Change Material")) {
						sMaterialsSearch->SetOpen(true);
						sMaterialsSearch->mCallback = [&, index](uint64_t uuid) {
							meshRenderer->ChangeMaterial(AssetsManager::mMaterials.at(uuid).get(), index);
						};
					}
					ImGui::SameLine();
					if (ImGui::Button("Remove Button")) {
						meshRenderer->mMaterialsUuids.erase(meshRenderer->mMaterialsUuids.begin() + index);
					}

					index++;
					if (treeNodeOpen) {
						if (AssetsManager::GetAsset(material->mAssetUuid) == nullptr) {
							ImGui::TreePop();
							ImGui::PopID();
							continue;
						}
						File file = File(material->mAssetName,
										 AssetsManager::GetAsset(material->mAssetUuid)->mAssetPath.string(),
										 Standards::materialExtName);
						Plaza::Editor::MaterialFileInspector(material);

						ImGui::TreePop();
					}
					ImGui::PopID();
				}

				if (ImGui::Button("Add Material")) {
					sMaterialsSearch->SetOpen(true);
					sMaterialsSearch->mCallback = [&](uint64_t uuid) { meshRenderer->mMaterialsUuids.push_back(uuid); };
				}

				ImGui::Text("Vertices: ");
				ImGui::SameLine();
				ImGui::Text(std::to_string(meshRenderer->GetMesh()->vertices.size()).c_str());

				ImGui::Text("Indices: ");
				ImGui::SameLine();
				ImGui::Text(std::to_string(meshRenderer->GetMesh()->indices.size()).c_str());

				ImGui::Text("Normals: ");
				ImGui::SameLine();
				ImGui::Text(std::to_string(meshRenderer->GetMesh()->normals.size()).c_str());

				ImGui::Text("Triangles: ");
				ImGui::SameLine();
				ImGui::Text(std::to_string(meshRenderer->GetMesh()->indices.size() / 3).c_str());

				ImGui::TreePop();
			}

			if (sMaterialsSearch->IsOpen())
				sMaterialsSearch->Update(scene);
			ImGui::PopID();
		}
	}
} // namespace Plaza::Editor
