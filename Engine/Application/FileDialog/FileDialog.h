#pragma once
#include <string>

#ifdef WIN32
#include <Commdlg.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include "ThirdParty/GLFW/include/GLFW/glfw3native.h"
#endif

namespace Plaza {
	class FileDialog {
	public:
		static std::string OpenFolderDialog();
		static std::string OpenFileDialog(const char* filter);
		static std::string SaveFileDialog(const char* filter);
	};
}