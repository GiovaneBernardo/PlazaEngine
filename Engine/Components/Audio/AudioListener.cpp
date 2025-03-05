#include "Engine/Core/PreCompiledHeaders.h"
#include "AudioListener.h"
#include <ThirdParty/AL/include/AL/al.h>
namespace Plaza {
	void AudioListener::UpdateListener() {
		mPosition = Application::Get()->activeCamera->Position;
		// Init listener
		Application::Get()->activeCamera;
		ALfloat listenerPos[] = {mPosition.x, mPosition.y, mPosition.z};
		ALfloat listenerVel[] = {0.0, 0.0f, 0.0};

		glm::vec3 forward = Application::Get()->activeCamera->Front;
		glm::vec3 up = Application::Get()->activeCamera->Up;
		glm::vec3 right = Application::Get()->activeCamera->Right;
		float listenerOrientation[6] = {forward.x, forward.y, forward.z, up.x, up.y, up.z};
		alListenerfv(AL_POSITION, listenerPos);
		alListenerfv(AL_VELOCITY, listenerVel);
		alListenerfv(AL_ORIENTATION, listenerOrientation);
	}
} // namespace Plaza
