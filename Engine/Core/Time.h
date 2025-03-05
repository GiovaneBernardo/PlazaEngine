#pragma once
#include "Engine/Application/Application.h"

#include <chrono>
using namespace std;
using namespace std::chrono;
namespace Plaza {
	class PLAZA_API Time {
	  public:
		static inline uint64_t mUniqueTriangles = 0;
		static inline uint64_t mTotalTriangles = 0;
		static inline int drawCalls = 0;
		static inline int addInstanceCalls = 0;
		static inline int frameCount = 0;
		static inline float previousTime = 0;
		static inline float deltaTime = 0;
		static inline float lastFrame = 0;
		static inline float fps = 0;
		static inline float msPerFrame = 0;
		static void Update();
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
