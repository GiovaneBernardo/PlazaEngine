#pragma once
#include "Engine/Core/PreCompiledHeaders.h"

#include "Engine/Components/Rendering/Material.h"
#include "Engine/Components/Core/Entity.h"
#include "Editor/GUI/Utils/Utils.h"

namespace Plaza::Editor {
	static class MeshRendererInspector {
	public:
		MeshRendererInspector(Entity* entity) {
			if (Utils::ComponentInspectorHeader(entity->GetComponent<MeshRenderer>(), "Mesh Renderer")) {
				ImGui::PushID("MeshRendererInspector");
				MeshRenderer* meshRenderer = entity->GetComponent<MeshRenderer>();

				MeshRenderer* meshR = entity->GetComponent<MeshRenderer>();
				RenderGroup* oldRenderGroup = entity->GetComponent<MeshRenderer>()->renderGroup;

				ImGui::Text("MeshRenderer Material: ");
				ImGui::SameLine();
				ImGui::Text(meshRenderer->material->name.c_str());
				ImGui::Text("RenderGroup Material: ");
				ImGui::SameLine();
				ImGui::Text(meshRenderer->renderGroup->material->name.c_str());

				for (auto [key, value] : Application->activeScene->materials) {
					if (value.get()->name.empty())
						continue;

					if (ImGui::Button(value->name.c_str())) {
						entity->GetComponent<MeshRenderer>()->ChangeMaterial(value.get());
						//entity->GetComponent<MeshRenderer>()->material = value.get();
						//entity->GetComponent<MeshRenderer>()->renderGroup->mesh = entity->GetComponent<MeshRenderer>()->mesh;
						//entity->GetComponent<MeshRenderer>()->renderGroup->ChangeMaterial(value.get());
					}
				}
				RenderGroup* newRenderGroup = entity->GetComponent<MeshRenderer>()->renderGroup;

				ImGui::PopID();
			}

		}
	};
}