#pragma once
#include "Engine/Components/Component.h"
#include <ThirdParty/AL/include/AL/al.h>
#include "Engine/Core/AssetsManager/AssetsManager.h"
#include "Engine/Core/Engine.h"

namespace Plaza {
	enum class AudioFileFormat
	{
		Unknown = 0,
		MP3
	};
	class PLAZA_API AudioSource : public Component {
	public:
		float mPosition[3] = { 0.0f, 0.0f, 0.0f };
		std::string mSourcePath;
		uint64_t mAudioAssetUuid = 0;
		ALuint mSource = 0;
		void Play();
		void Stop();
		void LoadFile(std::string filePath);

		bool mLoop = false;
		bool mSpatial = true;
		float mPitch = 1.0f;
		float mGain = 1.0f;

		void SetPosition(float x, float y, float z);
		void SetPosition(const glm::vec3& vec3);
		void SetGain(float value);
		void SetPitch(float value);
		void SetLoop(bool loop);
		void SetSpatial(bool spatial);

		//AudioSource(AudioSource& other) {
		//	this->SetGain(other.mGain);
		//	this->SetPitch(other.mPitch);
		//	this->SetLoop(other.mLoop);
		//	this->SetSpatial(other.mSpatial);
		//}

		virtual void OnInstantiate(Scene* scene, uint64_t toInstantiate) override;
		~AudioSource() override {
			//this->Stop();
		}

		template <class Archive>
		void serialize(Archive& archive) {
			archive(cereal::base_class<Component>(this), PL_SER(mPosition), PL_SER(mSourcePath), PL_SER(mSource), PL_SER(mLoop), PL_SER(mSpatial), PL_SER(mPitch), PL_SER(mGain), PL_SER(mAudioAssetUuid));

			if (mAudioAssetUuid && AssetsManager::GetAsset(mAudioAssetUuid) != nullptr) {
				this->mSourcePath = AssetsManager::GetAsset(mAudioAssetUuid)->mAssetPath.string();
				this->LoadFile(mSourcePath);
			}
		}
	};
}