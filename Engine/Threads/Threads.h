#pragma once
#include <mutex>
#include <queue>
#include <algorithm>
#include <execution>
#include <deque>
#include <thread>
#include <functional>

namespace Plaza {
	class Thread {
	  public:
		Thread(){};
		void RunParallel() {
			if (mRunning)
				return;

			mThread = std::thread([&]() {
				mRunning = true;
				while (mRunning) {
					UpdateParallel();
					if (mQueue.size() == 0)
						mRunning = false;
				}
				mRunning = false;
			});
			mThread.detach();
		}
		void AddToParallelQueue(std::function<void()> function) {
			{
				std::lock_guard<std::mutex> lock(mQueueMutex);
				mQueue.push_back(function);
			}
			RunParallel();
		}

		void UpdateParallel() {
			std::deque<std::function<void()>> tasks;
			{
				std::lock_guard<std::mutex> lock(mQueueMutex);
				tasks.swap(mQueue);
			}

			std::for_each(std::execution::par, tasks.begin(), tasks.end(), [](std::function<void()>& task) {
				if (task) {
					task();
				}
			});
		}

		void AddToQueue(std::function<void()> function) {
			{
				std::lock_guard<std::mutex> lock(mQueueMutex);
				mQueue.push_back(function);
			}
		}
		void Update() {
			std::function<void()> task;
			std::lock_guard<std::mutex> lock(mQueueMutex);

			while (!mQueue.empty()) {
				task = mQueue.front();
				mQueue.pop_front();
				if (task) {
					task();
				}
			}
		}

	  private:
		bool mRunning = false;
		std::mutex mQueueMutex;
		std::deque<std::function<void()>> mQueue = std::deque<std::function<void()>>();
		std::thread mThread;
	};
} // namespace Plaza
