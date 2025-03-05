#include "Engine/Core/PreCompiledHeaders.h"
#include "Time.h"
#include <ThirdParty/GLFW/include/GLFW/glfw3.h>

void Plaza::Time::Update() {
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
