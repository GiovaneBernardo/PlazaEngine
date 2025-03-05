#pragma once
#include "ThirdParty/Filewatcher/Filewatcher.h"
#include <queue>
#include <mutex>
namespace Plaza::Editor {
	class Filewatcher {
	  public:
		static void Start(std::string path);
		static void UpdateOnMainThread();
		static void AddToMainThread(const std::function<void()>& function);

		static std::unordered_map<uint64_t, std::function<void()>> mMainThreadQueue;
		static std::map<filewatch::Event, std::string> mQueuedEvents;
		static std::queue<std::function<void()>> taskQueue;
		static std::mutex queueMutex;
	};
} // namespace Plaza::Editor
