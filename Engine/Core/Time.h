#pragma once
#include "Engine/Application/Application.h"
namespace Engine {
	class Time
	{
	public:
		static int frameCount;
		static float previousTime;
		static float deltaTime;
		static float lastFrame;

		static void Update();
	};
}


inline int Engine::Time::frameCount = 0;
inline float Engine::Time::previousTime = 0;
inline float Engine::Time::deltaTime = 0;
inline float Engine::Time::lastFrame = 0;