#include "Engine/Core/PreCompiledHeaders.h"
#include "AudioSource.h"
#include "Engine/Core/Audio/AudioSourceLoader.h"
#include "ThirdParty/AL/include/AL/al.h"
#include "ThirdParty/AL/include/AL/alext.h"
#include "ThirdParty/AL/include/AL/alc.h"
#include "Engine/Core/Scene.h"
namespace Plaza {
	static AudioFileFormat GetFileFormat(const std::string& filePath)
	{
		std::filesystem::path path = filePath;
		std::string extension = path.extension().string();

		if (extension == ".mp3")  return AudioFileFormat::MP3;

		return AudioFileFormat::Unknown;
	}

	void AudioSource::OnInstantiate(Scene* scene, uint64_t toInstantiate) {
		AudioSource* audioSource = scene->GetComponent<AudioSource>(toInstantiate);
		this->SetGain(audioSource->mGain);
		this->SetPitch(audioSource->mPitch);
		this->SetLoop(audioSource->mLoop);
		this->SetSpatial(audioSource->mSpatial);
	}

	void AudioSource::Play() {
		alSourcePlay(mSource);
	};
	void AudioSource::Stop() {
		alSourceStop(mSource);
	};
	void AudioSource::LoadFile(std::string filePath) {
		mSourcePath = filePath;
		AudioFileFormat format = GetFileFormat(filePath);
		switch (format)
		{
			case AudioFileFormat::MP3: mSource = AudioSourceLoader::Mp3(filePath);
		}
	};

	void AudioSource::SetPosition(float x, float y, float z)
	{
		mPosition[0] = x;
		mPosition[1] = y;
		mPosition[2] = z;
		alSourcefv(mSource, AL_POSITION, mPosition);
	}

	void AudioSource::SetPosition(const glm::vec3& vec3)
	{
		mPosition[0] = vec3.x;
		mPosition[1] = vec3.y;
		mPosition[2] = vec3.z;
		alSourcefv(mSource, AL_POSITION, mPosition);
	}

	void AudioSource::SetGain(float value) {
		this->mGain = value;
		alSourcef(mSource, AL_GAIN, value);
	}

	void AudioSource::SetPitch(float value) {
		this->mPitch = value;
		alSourcef(mSource, AL_PITCH, value);
	}

	void AudioSource::SetLoop(bool loop) {
		this->mLoop = loop;

		alSourcei(mSource, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
	}

	void AudioSource::SetSpatial(bool spatial) {
		this->mSpatial = spatial;

		alSourcei(mSource, AL_SOURCE_SPATIALIZE_SOFT, mSpatial ? AL_TRUE : AL_FALSE);
		alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
	}
}