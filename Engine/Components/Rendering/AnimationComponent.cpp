#include "Engine/Core/PreCompiledHeaders.h"
#include "AnimationComponent.h"
#include "ThirdParty/ufbx/ufbx/ufbx.h"
#include "Engine/Core/Scene.h"

namespace Plaza {
	static inline glm::vec3 ConvertUfbxVec3(ufbx_vec3 vec) { return glm::vec3(vec.x, vec.y, vec.z); }
	static inline glm::quat ConvertUfbxQuat(ufbx_quat const& quaternionUfbx) {
		glm::vec4 vec4GLM;
		vec4GLM.x = quaternionUfbx.x;
		vec4GLM.y = quaternionUfbx.y;
		vec4GLM.z = quaternionUfbx.z;
		vec4GLM.w = quaternionUfbx.w;
		return glm::quat(quaternionUfbx.w, quaternionUfbx.x, quaternionUfbx.y, quaternionUfbx.z);
	};
	static glm::mat4 ConvertUfbxMatrix(ufbx_matrix const& mat4Ufbx) {
		glm::mat4 mat4Glm;
		for (unsigned int column = 0; column < 4; ++column) {
			mat4Glm[0][column] = mat4Ufbx.cols[column].x;
			mat4Glm[1][column] = mat4Ufbx.cols[column].y;
			mat4Glm[2][column] = mat4Ufbx.cols[column].z;
			mat4Glm[3][column] = column < 3 ? 0.0f : 1.0f;
		}
		return mat4Glm;
	}
	static glm::mat4 ConvertUfbxMatrix2(ufbx_matrix const& mat4Ufbx) {
		glm::mat4 inverseBindMatrix = glm::identity<glm::mat4>(); // glm::mat4(1.0f);  // Identity matrix

		inverseBindMatrix[0][0] = mat4Ufbx.m00;
		inverseBindMatrix[0][1] = mat4Ufbx.m10;
		inverseBindMatrix[0][2] = mat4Ufbx.m20;

		inverseBindMatrix[1][0] = mat4Ufbx.m01;
		inverseBindMatrix[1][1] = mat4Ufbx.m11;
		inverseBindMatrix[1][2] = mat4Ufbx.m21;

		inverseBindMatrix[2][0] = mat4Ufbx.m02;
		inverseBindMatrix[2][1] = mat4Ufbx.m12;
		inverseBindMatrix[2][2] = mat4Ufbx.m22;

		inverseBindMatrix[3][0] = mat4Ufbx.m03;
		inverseBindMatrix[3][1] = mat4Ufbx.m13;
		inverseBindMatrix[3][2] = mat4Ufbx.m23;
		return inverseBindMatrix;
	}

	void Animation::Play() {
		this->mIsPlaying = true;
		Scene::GetActiveScene()->mPlayingAnimations.clear();
		Scene::GetActiveScene()->mPlayingAnimations.emplace(this->mAssetUuid, this);
	}

	Bone* Animation::GetRootBone() {
		if (!mRootBone &&
			VulkanRenderer::GetRenderer()->mBones.find(mRootBoneUuid) != VulkanRenderer::GetRenderer()->mBones.end()) {
			mRootBone = &VulkanRenderer::GetRenderer()->mBones.at(mRootBoneUuid);
		}
		return mRootBone;
	}

	void AnimationComponent::GetAnimation(std::string filePath, std::map<uint64_t, Plaza::Bone>& bonesMap,
										  unsigned int index) {
		this->mAnimations.clear();
		ufbx_load_opts opts = {};
		opts.target_axes = ufbx_axes_right_handed_y_up, opts.target_unit_meters = 1.0f;
		opts.target_axes = {
			.right = UFBX_COORDINATE_AXIS_POSITIVE_X,
			.up = UFBX_COORDINATE_AXIS_POSITIVE_Y,
			.front = UFBX_COORDINATE_AXIS_POSITIVE_Z,
		};

		ufbx_error error;
		ufbx_scene* scene = ufbx_load_file(filePath.c_str(), &opts, &error);

		if (!scene) {
			fprintf(stderr, "Failed to load scene: %s\n", error.description.data);
			return;
		}
		int meshCount = scene->meshes.count;
		std::set<uint64_t> boneIds = std::set<uint64_t>();
		std::vector<ufbx_bone*> uniqueBones = std::vector<ufbx_bone*>();

		for (auto& [key, value] : VulkanRenderer::GetRenderer()->mBones) {
			value.mChildren.clear();
		}

		for (const ufbx_skin_deformer* skin : scene->skin_deformers) {
			for (const ufbx_skin_cluster* cluster : skin->clusters) {
				// VulkanRenderer::GetRenderer()->mBones.at(cluster->bone_node->bone->element_id).mOffset =
				// (ConvertUfbxMatrix2(cluster->geometry_to_bone));
				if (cluster->bone_node->parent && cluster->bone_node->parent->bone) {
					VulkanRenderer::GetRenderer()->mBones.at(cluster->bone_node->bone->element_id).mParentId =
						cluster->bone_node->parent->bone->element_id;

					std::vector<uint64_t>& childrenVector =
						VulkanRenderer::GetRenderer()
							->mBones.at(cluster->bone_node->parent->bone->element_id)
							.mChildren;

					if (std::find(childrenVector.begin(), childrenVector.end(), cluster->bone_node->bone->element_id) ==
						childrenVector.end()) {
						childrenVector.push_back(cluster->bone_node->bone->element_id);
					}
				}
			}
		}
		for (const ufbx_anim_stack* stack : scene->anim_stacks) {
			if (stack->time_end - stack->time_begin <= 0.0f) {
				continue;
			}

			ufbx_baked_anim* bake = ufbx_bake_anim(scene, stack->anim, nullptr, nullptr);
			assert(bake != nullptr);
			if (bake == nullptr)
				continue;
			Animation& animation = mAnimations.emplace_back(Animation());
			animation.mAssetName = stack->name.data;

			for (ufbx_bone* bone : scene->bones) {
				if (bone->is_root)
					animation.SetRootBone(&VulkanRenderer::GetRenderer()->mBones.at(bone->element_id));
			}
			if (!animation.GetRootBone()) {
				animation.SetRootBone(&VulkanRenderer::GetRenderer()->mBones.at(scene->bones[0]->element_id));
			}
			// animation.mRootParentTransform = ConvertUfbxMatrix2(scene->root_node->geometry_to_world);
			animation.mStartTime = (float)bake->playback_time_begin;
			animation.mEndTime = (float)bake->playback_time_end;
			for (const ufbx_baked_node& bakeNode : bake->nodes) {
				ufbx_node* sceneNode = scene->nodes[bakeNode.typed_id];

				// Translation:
				std::map<float, glm::vec3> positions = std::map<float, glm::vec3>();
				for (const ufbx_baked_vec3& keyframe : bakeNode.translation_keys) {
					positions[keyframe.time] = glm::vec3(keyframe.value.x, keyframe.value.y, keyframe.value.z);
				}

				// Rotation:
				std::map<float, glm::quat> rotations = std::map<float, glm::quat>();
				for (const ufbx_baked_quat& keyframe : bakeNode.rotation_keys) {
					rotations[keyframe.time] = ConvertUfbxQuat(keyframe.value);
				}

				uint64_t boneUuid = sceneNode->bone->element_id;
				unsigned int maxCount = glm::max(positions.size(), rotations.size());
				std::map<float, Bone::Keyframe> keyframes = std::map<float, Bone::Keyframe>();
				for (const auto& [key, value] : positions) {
					keyframes[key].position = value;
				}
				for (const auto& [key, value] : rotations) {
					keyframes[key].orientation = value;
				}
				for (const auto& [key, value] : keyframes) {
					animation.mKeyframes[boneUuid].push_back(
						Bone::Keyframe{key, ConvertUfbxVec3(ufbx_evaluate_baked_vec3(bakeNode.translation_keys, key)),
									   ConvertUfbxQuat(ufbx_evaluate_baked_quat(bakeNode.rotation_keys, key)),
									   ConvertUfbxVec3(ufbx_evaluate_baked_vec3(bakeNode.scale_keys, key))});
				}
				if (animation.mKeyframes[boneUuid].size() == 0) {
					animation.mKeyframes[boneUuid].push_back(
						Bone::Keyframe{0.0f, glm::vec3(0.0f), glm::quat(glm::vec3(0.0f)), glm::vec3(1.0f)});
				}
			}

			ufbx_free_baked_anim(bake);
		}

		ufbx_free_scene(scene);
	}
} // namespace Plaza
