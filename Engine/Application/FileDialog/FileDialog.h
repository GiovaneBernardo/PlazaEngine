#pragma once
#include <string>

#ifdef WIN32
#include <Commdlg.h>
#endif
#ifdef WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#elif __linux__
#define GLFW_EXPOSE_NATIVE_X11
#endif
#include "ThirdParty/GLFW/include/GLFW/glfw3native.h"

namespace Plaza {
	class FileDialog {
	  public:
		static std::string OpenFolderDialog();
		static std::string OpenFileDialog(const char* filter);
		static std::string SaveFileDialog(const char* filter);
	};
} // namespace Plaza
