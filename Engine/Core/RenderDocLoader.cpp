// RenderDoc Early Loader
// =======================
// Runs before main() via #pragma init_seg(lib) to set up RenderDoc.
//
// SETUP: Place renderdoc.dll and renderdoc.json in the exe directory.
//        CMake does this automatically from ThirdParty/renderdoc/.
//
// HOW IT WORKS:
// 1. Sets ENABLE_VULKAN_RENDERDOC_CAPTURE=1 to trigger RenderDoc's layer
// 2. Sets VK_ADD_IMPLICIT_LAYER_PATH to the exe directory
// 3. Loads renderdoc.dll for API access
// 4. When Vulkan initializes, it loads the RenderDoc layer automatically
//
// USAGE: Use "Capture Frame (RenderDoc)" button in Editor Settings.
//        Works with VS debugging - no relaunch needed.

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace {
    // Helper to concatenate strings without std::string (CRT may not be initialized)
    void ConcatPath(char* dest, size_t destSize, const char* path1, const char* path2) {
        size_t len1 = 0;
        while (path1[len1]) len1++;

        size_t i = 0;
        for (; i < len1 && i < destSize - 1; i++) {
            dest[i] = path1[i];
        }

        size_t len2 = 0;
        while (path2[len2]) len2++;

        for (size_t j = 0; j < len2 && i < destSize - 1; j++, i++) {
            dest[i] = path2[j];
        }
        dest[i] = '\0';
    }

    struct RenderDocEarlyLoader {
        HMODULE module = nullptr;

        RenderDocEarlyLoader() {
            // Get exe directory using only Windows API
            char exePath[MAX_PATH];
            DWORD len = GetModuleFileNameA(NULL, exePath, MAX_PATH);
            if (len == 0) return;

            // Find last backslash
            char* lastSlash = nullptr;
            for (char* p = exePath; *p; p++) {
                if (*p == '\\' || *p == '/') lastSlash = p;
            }
            if (lastSlash) *lastSlash = '\0';

            // Build paths
            char dllPath[MAX_PATH];
            char jsonPath[MAX_PATH];
            ConcatPath(dllPath, MAX_PATH, exePath, "\\renderdoc.dll");
            ConcatPath(jsonPath, MAX_PATH, exePath, "\\renderdoc.json");

            // Check if files exist
            if (GetFileAttributesA(dllPath) == INVALID_FILE_ATTRIBUTES ||
                GetFileAttributesA(jsonPath) == INVALID_FILE_ATTRIBUTES) {
                return;
            }

            // Set environment variables for Vulkan implicit layer
            // ENABLE_VULKAN_RENDERDOC_CAPTURE triggers the layer's enable_environment
            SetEnvironmentVariableA("ENABLE_VULKAN_RENDERDOC_CAPTURE", "1");

            // Use VK_ADD_IMPLICIT_LAYER_PATH for implicit layers (those with enable_environment)
            char existingPath[4096] = "";
            GetEnvironmentVariableA("VK_ADD_IMPLICIT_LAYER_PATH", existingPath, sizeof(existingPath));

            char newPath[4096];
            if (existingPath[0]) {
                ConcatPath(newPath, sizeof(newPath), exePath, ";");
                size_t newLen = 0;
                while (newPath[newLen]) newLen++;
                ConcatPath(newPath + newLen, sizeof(newPath) - newLen, existingPath, "");
            } else {
                ConcatPath(newPath, sizeof(newPath), exePath, "");
            }
            SetEnvironmentVariableA("VK_ADD_IMPLICIT_LAYER_PATH", newPath);

            // Load RenderDoc DLL
            module = LoadLibraryA(dllPath);
        }
    };

    // Use init_seg(lib) instead of init_seg(compiler) - still early but CRT is initialized
    #pragma warning(push)
    #pragma warning(disable: 4073)
    #pragma init_seg(lib)
    static RenderDocEarlyLoader g_renderDocEarlyLoader;
    #pragma warning(pop)
}
#endif
