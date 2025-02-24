#include "Engine/Core/PreCompiledHeaders.h"
#include "ComponentsInspector.h"
#include "Editor/GUI/Utils/Utils.h"
#include "Engine/Components/Rendering/AnimationComponent.h"
#include "Engine/Application/FileDialog/FileDialog.h"
#include "Engine/Core/Scene.h"
#include "Engine/Core/Renderer/Vulkan/Renderer.h"
#include "Engine/Core/Scene.h"


namespace Plaza::Editor {
	void BoneParentShipTree(Bone* bone) {
		if (!bone)
			return;
		if (ImGui::TreeNodeEx(bone->mName.c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) {
			for (uint64_t childUuid : bone->mChildren)
				BoneParentShipTree(&VulkanRenderer::GetRenderer()->mBones.at(childUuid));
			ImGui::TreePop();
		}
	}
	void ComponentsInspector::AnimationComponentInspector(Scene* scene, Entity* entity) {
		AnimationComponent* component = scene->GetComponent<AnimationComponent>(entity->uuid);
		static int index = 0;
		if (Utils::ComponentInspectorHeader(component, "Animation Component")) {
			//ImGui::PushID("AnimationComponentInspector");

			ImGui::InputInt("Animation Index: ", &index);

			if (ImGui::Button("Add Animation")) {
				ImGui::OpenPopup("Select Animation Popup");
			}

			if (ImGui::BeginPopup("Select Animation Popup")) {
				for (int i = 0; i < AssetsManager::mLoadedAnimations.size(); ++i) {
					if (ImGui::Selectable(AssetsManager::mLoadedAnimations[i].mAssetName.c_str())) {
						if (!AssetsManager::mLoadedAnimations[i].GetRootBone())
							if (VulkanRenderer::GetRenderer()->mBones.find(AssetsManager::mLoadedAnimations[i].mRootBoneUuid) != VulkanRenderer::GetRenderer()->mBones.end())
								AssetsManager::mLoadedAnimations[i].SetRootBone(&VulkanRenderer::GetRenderer()->mBones.at(AssetsManager::mLoadedAnimations[i].mRootBoneUuid));
						component->AddAnimation(AssetsManager::mLoadedAnimations[i]);
					}
				}
				ImGui::EndPopup();
			}

			if (ImGui::Button("Load animations from FBX")) {
				std::string path = FileDialog::OpenFileDialog(".fbx");
				for (auto& [key, value] : Scene::GetActiveScene()->mPlayingAnimations) {
					value->Stop();
				}
				Scene::GetActiveScene()->mPlayingAnimations.clear();
				component->GetAnimation(path, VulkanRenderer::GetRenderer()->mBones, index);
			}

			for (Animation& animation : component->mAnimations) {
				//ImGui::BeginChild(std::string("Animation: " + animation.mName).c_str());
				ImGui::PushID(std::string("Animation: " + animation.mAssetName).c_str());
				if (ImGui::Button(std::string("Play" + animation.mAssetName).c_str())) {
					for (auto& [key, value] : Scene::GetActiveScene()->mPlayingAnimations) {
						value->Stop();
					}
					animation.Play();
				}
				float time = animation.mCurrentTime;
				ImGui::SliderFloat("Current Time", &time, animation.mStartTime, animation.mEndTime);
				animation.SetTime(time);

				ImGui::PopID();
				//	ImGui::EndChild();
				//}
			}


			for (Animation& animation : component->mAnimations) {
				if (ImGui::TreeNodeEx(std::string("beg" + animation.mAssetName).c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) {
					BoneParentShipTree(animation.GetRootBone());
					ImGui::TreePop();
				}
			}

			//ImGui::PopID();
		}
	}
}