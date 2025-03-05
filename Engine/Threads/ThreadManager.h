#pragma once
#include "Threads.h"

namespace Plaza {
	class ThreadsManager {
	  public:
		Thread* mAssetsLoadingThread = new Thread();
		Thread* mFrameStartThread = new Thread();
		Thread* mFrameEndThread = new Thread();
		Thread* mFrameRendererStartThread = new Thread();
		Thread* mFrameRendererBeforeFenceThread = new Thread();
		Thread* mFrameRendererAfterFenceThread = new Thread();
		Thread* mFrameRendererAfterGeometry = new Thread();
		void Init() {}
		void UpdateFrameStartThread() { mFrameStartThread->Update(); }
		void UpdateFrameEndThread() { mFrameEndThread->Update(); }
	};
} // namespace Plaza
