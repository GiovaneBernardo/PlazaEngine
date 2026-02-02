#include "Engine/Core/PreCompiledHeaders.h"
#include "Time.h"
#include <ThirdParty/GLFW/include/GLFW/glfw3.h>

namespace Plaza {
	PLAZA_API uint64_t Time::mUniqueTriangles = 0;
	PLAZA_API uint64_t Time::mTotalTriangles = 0;
	PLAZA_API int Time::drawCalls = 0;
	PLAZA_API int Time::addInstanceCalls = 0;
	PLAZA_API int Time::frameCount = 0;
	PLAZA_API float Time::previousTime = 0;
	PLAZA_API float Time::deltaTime = 0;
	PLAZA_API float Time::lastFrame = 0;
	PLAZA_API float Time::fps = 0;
	PLAZA_API float Time::msPerFrame = 0;

	void Time::Update() {
		float currentTime = static_cast<float>(glfwGetTime());

		// Delta Time
		deltaTime = currentTime - lastFrame;
		lastFrame = currentTime;

		// FPS tracking (updates every 1 second)
		frameCount++;
		float elapsed = currentTime - previousTime;
		if (elapsed >= 0.1f) {
			fps = static_cast<float>(frameCount) / elapsed;
			msPerFrame = 1000.0f / fps;
			frameCount = 0;
			previousTime = currentTime;
		}
	}


	float Time::GetDeltaTime() { return deltaTime; }
} // namespace Plaza
