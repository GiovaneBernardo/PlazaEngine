#pragma once
#define AL_LIBTYPE_STATIC
#include <ThirdParty/AL/include/AL/al.h>
#include <ThirdParty/AL/include/AL/alc.h>
#include <ThirdParty/AL/include/AL/alext.h>
namespace Plaza {
	class Audio {
	  public:
		static ALCdevice* sAudioDevice;
		static void Init();
		static void UpdateListener(Scene* scene);
	};
} // namespace Plaza
