#pragma once
#include <Engine/Core/AssetsManager/Asset.h>
#include <Engine/Core/Engine.h>
#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include <Engine/Components/Component.h>
#include <Engine/Core/Renderer/Mesh.h>
#include <cereal/types/base_class.hpp>

namespace Plaza {
	class PLAZA_API Animation : public Asset {
	  public:
		float mStartTime = 0.0f;
		float mEndTime = 0.0f;
		float mCurrentTime = 0.0f;
		float mAnimationSpeed = 1.0f;
		std::map<uint64_t, std::vector<Bone::Keyframe>> mKeyframes = std::map<uint64_t, std::vector<Bone::Keyframe>>();

		void Play();
		void Stop() {};
		void UpdateTime(float deltaTime) {
			mCurrentTime += deltaTime * mAnimationSpeed;
			if (mCurrentTime > mEndTime)
				SetTime(mStartTime);
		}
		void SetTime(float time) { this->mCurrentTime = time; }
		bool GetState() { return mIsPlaying; };

		uint64_t mRootBoneUuid;
		Bone* GetRootBone();
		void SetRootBone(Bone* newRootBone) {
			this->mRootBone = newRootBone;
			this->mRootBoneUuid = newRootBone->mId;
		}

		void SetRootBoneUuid(uint64_t uuid) { this->mRootBoneUuid = uuid; }

		template <class Archive> void serialize(Archive& archive) {
			archive(PL_SER(mAssetUuid), PL_SER(mAssetName), PL_SER(mStartTime), PL_SER(mEndTime), PL_SER(mCurrentTime),
					PL_SER(mAnimationSpeed), PL_SER(mKeyframes), PL_SER(mRootBoneUuid));
		}

		// template <class Archive>
		// void serialize(Archive& archive) {
		//	archive(mUuid, mName, mStartTime, mEndTime, mCurrentTime, mAnimationSpeed, mKeyframes, mRootBoneUuid);
		// }
	  private:
		Bone* mRootBone = nullptr;
		bool mIsPlaying = false;
		std::vector<int64_t> mBonesIds = std::vector<int64_t>();
	};

	PLAZA_API class AnimationComponent : public Component {
	  public:
		float mImportScale = 1.0f; // 0.01f;
		std::vector<Animation> mAnimations = std::vector<Animation>();
		void GetAnimation(std::string filePath, std::map<uint64_t, Plaza::Bone>& bonesMap, unsigned int index = 0);
		void AddAnimation(Animation animation) { mAnimations.push_back(animation); }

		template <class Archive> void serialize(Archive& archive) {
			archive(cereal::base_class<Component>(this), mImportScale, mAnimations);
		}

	  private:
		friend class Animation;
	};
} // namespace Plaza
