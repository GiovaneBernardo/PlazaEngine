#include "Engine/Core/PreCompiledHeaders.h"
#include "FrameCapture.h"

#ifdef _WIN32
#include <Windows.h>
#endif

namespace Plaza {

	bool FrameCapture::TryInitAPI() {
#ifdef _WIN32
		if (sAPI)
			return true;

		HMODULE rdocModule = GetModuleHandleA("renderdoc.dll");
		if (!rdocModule)
			return false;

		pRENDERDOC_GetAPI getAPI = (pRENDERDOC_GetAPI)GetProcAddress(rdocModule, "RENDERDOC_GetAPI");
		if (!getAPI)
			return false;

		int ret = getAPI(eRENDERDOC_API_Version_1_6_0, (void**)&sAPI);
		if (ret != 1) {
			sAPI = nullptr;
			return false;
		}

		PL_CORE_INFO("RenderDoc API initialized successfully");

		// Disable the overlay (FPS counter, etc.)
		sAPI->MaskOverlayBits(eRENDERDOC_Overlay_None, eRENDERDOC_Overlay_None);

		// Configure RenderDoc
		sAPI->SetCaptureOptionU32(eRENDERDOC_Option_AllowVSync, 1);
		sAPI->SetCaptureOptionU32(eRENDERDOC_Option_AllowFullscreen, 1);
		sAPI->SetCaptureOptionU32(eRENDERDOC_Option_APIValidation, 0);
		sAPI->SetCaptureOptionU32(eRENDERDOC_Option_DebugOutputMute, 1);
		sAPI->SetCaptureOptionU32(eRENDERDOC_Option_CaptureCallstacks, 0);
		sAPI->SetCaptureOptionU32(eRENDERDOC_Option_RefAllResources, 0);

		// Disable RenderDoc's default capture keys so only our button works
		sAPI->SetCaptureKeys(nullptr, 0);

		// Set capture path
		char exePath[MAX_PATH];
		GetModuleFileNameA(NULL, exePath, MAX_PATH);
		std::string captureDir = std::filesystem::path(exePath).parent_path().string() + "\\Captures";
		std::filesystem::create_directories(captureDir);
		std::string capturePath = captureDir + "\\PlazaCapture";
		sAPI->SetCaptureFilePathTemplate(capturePath.c_str());

		PL_CORE_INFO("RenderDoc captures will be saved to: {}", captureDir);
		return true;
#else
		return false;
#endif
	}

	void FrameCapture::Init() {
		TryInitAPI();
	}

	void FrameCapture::Shutdown() { sAPI = nullptr; }

	void FrameCapture::TriggerCapture() {
		if (!TryInitAPI()) {
			PL_CORE_WARN("RenderDoc not attached. Please attach RenderDoc to this process first.");
			return;
		}

		sAPI->TriggerCapture();
		PL_CORE_INFO("Frame capture triggered");
	}

	void FrameCapture::TriggerCaptureAndOpen() {
		if (!TryInitAPI()) {
			PL_CORE_WARN("RenderDoc not available. Make sure renderdoc.dll and renderdoc.json are in the exe directory.");
			return;
		}

		PL_CORE_INFO("Triggering frame capture...");

		sCaptureCountBefore = sAPI->GetNumCaptures();
		sShouldCapture = true;
		sShouldOpenUI = true;
	}

	void FrameCapture::BeginFrame() {
		if (!sAPI || !sShouldCapture)
			return;

		// Start capturing this frame
		sAPI->StartFrameCapture(nullptr, nullptr);
		sIsCapturing = true;
		sShouldCapture = false;
		PL_CORE_INFO("RenderDoc: Frame capture started");
	}

	void FrameCapture::EndFrame() {
		if (!sAPI)
			return;

		// End capture if we were capturing
		if (sIsCapturing) {
			uint32_t result = sAPI->EndFrameCapture(nullptr, nullptr);
			sIsCapturing = false;

			if (result == 1) {
				PL_CORE_INFO("RenderDoc: Frame capture completed");
			} else {
				PL_CORE_ERROR("RenderDoc: Frame capture failed");
				sShouldOpenUI = false;
			}
		}

		// Check if we should open the UI
		if (sShouldOpenUI) {
			uint32_t numCaptures = sAPI->GetNumCaptures();
			if (numCaptures > sCaptureCountBefore) {
				char filename[4096];
				uint32_t pathLength = 4096;

				if (sAPI->GetCapture(numCaptures - 1, filename, &pathLength, nullptr)) {
					PL_CORE_INFO("Opening capture: {}", filename);
					sAPI->LaunchReplayUI(1, filename);
				}
				sShouldOpenUI = false;
			}
		}
	}

	bool FrameCapture::IsAvailable() {
		return TryInitAPI();
	}

} // namespace Plaza
