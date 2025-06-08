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
		// Delta time
		int& frameCount = Time::frameCount;
		float& previousTime = Time::previousTime;
		float& deltaTime = Time::deltaTime;
		float& lastFrame = Time::lastFrame;
		// Measure speed
		double currentTime = glfwGetTime();
		float timeDifference = currentTime - previousTime;
		frameCount++;

		// If a second has passed.
		if (timeDifference >= 1.0f / 20.0f) {
			// Display the frame count here any way you want.
			// std::cout << frameCount << std::endl;
			fps = (1.0f / timeDifference) * frameCount; // frameCount;
			msPerFrame = (timeDifference / frameCount) * 1000;
			frameCount = 0;
			previousTime = glfwGetTime();
		}

		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
	}

	float Time::GetDeltaTime() { return deltaTime; }
} // namespace Plaza
