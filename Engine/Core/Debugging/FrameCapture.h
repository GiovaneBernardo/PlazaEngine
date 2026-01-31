#pragma once
#include "ThirdParty/renderdoc/renderdoc_app.h"

namespace Plaza {
	class FrameCapture {
	  public:
		static void Init();
		static void Shutdown();

		// Call at the start and end of each frame (in Render function)
		static void BeginFrame();
		static void EndFrame();

		static void TriggerCapture();
		static void TriggerCaptureAndOpen(); // Captures and opens RenderDoc UI

		static bool IsAvailable();

	  private:
		static bool TryInitAPI();

		static inline RENDERDOC_API_1_6_0* sAPI = nullptr;
		static inline bool sIsCapturing = false;
		static inline bool sShouldCapture = false;
		static inline bool sShouldOpenUI = false;
		static inline uint32_t sCaptureCountBefore = 0;
	};
} // namespace Plaza
