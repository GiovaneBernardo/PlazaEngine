#include "Engine/Core/PreCompiledHeaders.h"
#include "EditorInspector.h"
#include "Editor/GUI/Utils/Utils.h"
#include <iostream>
#include "Editor/Settings/EditorSettings.h"
#include "Editor/Settings/SettingsSerializer.h"
#include "Engine/Core/Renderer/Vulkan/VulkanBloom.h"
#include "Editor/GUI/Utils/DataVisualizer.h"
#include "Engine/Core/Renderer/Vulkan/Renderer.h"
#include "Engine/Core/AssetsManager/Serializer/AssetsSerializer.h"

namespace Plaza::Editor {
	void EditorInspector::Update() {
		if (Utils::ComponentInspectorHeader(nullptr, "Editor")) {
			ImGui::PushID("EditorInspector");

			/* Editor Settings */
			if (Utils::ComponentInspectorHeader(nullptr, "Settings")) {
				if (ImGui::Checkbox("VSync", &Application::Get()->mSettings.mVsync)) {
					// Application::Get()->mEditor->mSettings.ReapplyAllSettings();
				}

				ImGui::Checkbox("Show Cascade Levels", &Application::Get()->showCascadeLevels);

				ImGui::DragFloat("Bloom Intensity", &VulkanRenderer::GetRenderer()->mBloom.mBloomIntensity);
				ImGui::DragFloat("Bloom Knee", &VulkanRenderer::GetRenderer()->mBloom.mKnee);
				ImGui::DragFloat("Bloom Threshold", &VulkanRenderer::GetRenderer()->mBloom.mThreshold);
				ImGui::DragInt("Bloom Mip Count", &VulkanRenderer::GetRenderer()->mBloom.mMipCount);

				ImGui::DragFloat("Exposure", &VulkanRenderer::GetRenderer()->exposure);
				ImGui::DragFloat("Gamma", &VulkanRenderer::GetRenderer()->gamma);
				ImGui::ColorPicker3("Directional Light Color",
									&VulkanRenderer::GetRenderer()->mLighting->directionalLightColor.x);
				ImGui::DragFloat("Directional Intensity",
								 &VulkanRenderer::GetRenderer()->mLighting->directionalLightIntensity);
				ImGui::ColorPicker3("Ambient Light Color",
									&VulkanRenderer::GetRenderer()->mLighting->ambientLightColor.x);
				ImGui::DragFloat("Ambient Intesnity", &VulkanRenderer::GetRenderer()->mLighting->ambientLightIntensity);
				ImGui::DragFloat("Skybox Intesnity", &VulkanRenderer::GetRenderer()->mSkyboxIntensity);

				if (ImGui::Button("Save Settings")) {
					// Editor::EditorSettingsSerializer::Serialize();
					AssetsSerializer::SerializeFile<EngineSettings>(
						Application::Get()->mSettings,
						Application::Get()->enginePathAppData + "Settings" + Standards::editorSettingsExtName,
						Application::Get()->mSettings.mCommonSerializationMode);
				}
			}

			/* Render Graph */
			if (Utils::ComponentInspectorHeader(nullptr, "RenderGraph", 0)) {
				ImGui::BeginTable("RenderGraph Settings", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg);
				for (const auto& [key, pass] : VulkanRenderer::GetRenderer()->mRenderGraph->mPasses) {
					Utils::AddTableSingleString(pass->mName);
					bool recompile;
					Utils::AddTableButtonString(pass->mName, "Recompile Shaders", &recompile, 0, [pass](bool* value) {
						if (*value) {
							pass->ReCompileShaders(true);
						}
					});
				}
				ImGui::EndTable();
			}

			/* Skybox Render Graph */
			if (Utils::ComponentInspectorHeader(nullptr, "Skbyox RenderGraph", 0)) {
				if (ImGui::Button("Build and Run"))
					VulkanRenderer::GetRenderer()->mRenderGraph->RunSkyboxRenderGraph(
						VulkanRenderer::GetRenderer()->mRenderGraph->BuildSkyboxRenderGraph());
			}

			/* Colors */
			if (Utils::ComponentInspectorHeader(nullptr, "Colors", 0)) {
				int index = 0;
				for (auto& color : ImGui::GetStyle().Colors) {
					ImGui::PushID("EditorInspectorlabel" + index);
					float col[3];
					col[0] = color.x;
					col[1] = color.y;
					col[2] = color.z;
					if (ImGui::ColorPicker3(ImGui::GetStyleColorName(index), col)) {
						color.x = col[0];
						color.y = col[1];
						color.z = col[2];
					};
					index++;
					ImGui::PopID();
				}
			}
			ImGui::PopID();
		}
	}
} // namespace Plaza::Editor
