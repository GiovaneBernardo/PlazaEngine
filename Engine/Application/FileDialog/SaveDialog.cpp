#include "Engine/Core/PreCompiledHeaders.h"
#include "FileDialog.h"

#ifdef WIN32
namespace Plaza {
    std::string FileDialog::SaveFileDialog(const char* filter) {
        OPENFILENAMEA  ofn;
        CHAR  szFileName[260] = { 0 };

        ZeroMemory(&ofn, sizeof(OPENFILENAME));

        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = glfwGetWin32Window(Application::Get()->mWindow->glfwWindow);
        ofn.lpstrFile = szFileName;
        ofn.nMaxFile = sizeof(szFileName);
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;

        if (GetSaveFileNameA(&ofn) == TRUE) {
            return ofn.lpstrFile;
        }

        return "";
    }
}

#elif __linux__
#include <stdio.h>
#include <stdlib.h>

namespace Plaza {
    std::string FileDialog::SaveFileDialog(const char* filter) {
        char filename[1024] = {0};
        FILE* fp = popen("zenity --file-selection --save", "r");
        if(fp){
            fgets(filename, sizeof(filename), fp);
            pclose(fp);
            filename[strcspn(filename, "\n")] = 0;
            return std::string(filename);
        }

        return "";
    }
}