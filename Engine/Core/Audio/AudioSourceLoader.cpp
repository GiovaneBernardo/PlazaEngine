#include "Engine/Core/PreCompiledHeaders.h"
#include "AudioSourceLoader.h"
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <filesystem>

#include <ThirdParty/AL/include/AL/al.h>
#include <ThirdParty/AL/include/AL/alext.h>
#include <ThirdParty/AL/include/AL/alc.h>

#define MINIMP3_IMPLEMENTATION
#include "ThirdParty/minimp3/minimp3.h"
#include "ThirdParty/minimp3/minimp3_ex.h"

namespace Plaza {
	static ALenum GetOpenALFormat(uint32_t channels) {
		// Note: sample size is always 2 bytes (16-bits) with
		// both the .mp3 and .ogg decoders that we're using
		switch (channels) {
			case 1:
				return AL_FORMAT_MONO16;
			case 2:
				return AL_FORMAT_STEREO16;
		}
		// assert
		return 0;
	}

	unsigned int AudioSourceLoader::Mp3(std::string filePath) {
		mp3dec_file_info_t info;

		mp3dec_t s_Mp3d;
		mp3dec_init(&s_Mp3d);

		int loadResult = mp3dec_load(&s_Mp3d, filePath.c_str(), &info, NULL, NULL);
		uint32_t size = info.samples * sizeof(mp3d_sample_t);

		auto sampleRate = info.hz;
		auto channels = info.channels;
		auto alFormat = GetOpenALFormat(channels);
		float lengthSeconds = size / (info.avg_bitrate_kbps * 1024.0f);

		ALuint buffer;
		alGenBuffers(1, &buffer);
		alBufferData(buffer, alFormat, info.buffer, size, sampleRate);

		ALuint source;
		alGenSources(1, &source);
		alSourcei(source, AL_BUFFER, buffer);

		return source;
	}
} // namespace Plaza
