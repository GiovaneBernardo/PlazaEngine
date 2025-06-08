#pragma once
#include "Engine/Core/Engine.h"

#include <chrono>
using namespace std;
using namespace std::chrono;
namespace Plaza {
	class PLAZA_API Time {
	  public:
		static uint64_t mUniqueTriangles;
		static uint64_t mTotalTriangles;
		static int drawCalls;
		static int addInstanceCalls;
		static int frameCount;
		static float previousTime;
		static float deltaTime;
		static float lastFrame;
		static float fps;
		static float msPerFrame;
		static void Update();
		static float GetDeltaTime();
	};

	// class PLAZA_API Profiler {
	// public:
	//	string name;
	//	int divider;
	//	Profiler(string name_t, int divider_t = 10) : name(name_t) {
	//		this->divider = divider_t;
	//		Start();
	//	}
	//	high_resolution_clock::time_point startTime;
	//	high_resolution_clock::time_point endTime;
	//	void Start() {
	//		if (Time::frameCount % divider)
	//			startTime = std::chrono::high_resolution_clock::now();
	//	}
	//	void Stop() {
	//		if (Time::frameCount % divider) {
	//			endTime = std::chrono::high_resolution_clock::now();
	//			Print();
	//		}
	//	}
	//
	//	void Print() {
	//		if (Time::frameCount % divider) {
	//			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
	//			// Print the execution time
	//			std::cout << name << ": " << duration.count() << " ms" << std::endl;
	//		}
	//	}
	// };
} // namespace Plaza
