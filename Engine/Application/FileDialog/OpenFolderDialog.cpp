#include "Engine/Core/PreCompiledHeaders.h"
#include "FileDialog.h"

#ifdef WIN32
#include <ShlObj.h>
#include <ShObjIdl.h>
namespace Plaza {
	// Function to open the file dialog and get the selected file path
	std::string FileDialog::OpenFolderDialog() {
		std::string selectedFolder;

		CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

		IFileDialog* pFolderDialog;
		HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFolderDialog));

		if (SUCCEEDED(hr)) {
			DWORD dwOptions;
			pFolderDialog->GetOptions(&dwOptions);
			pFolderDialog->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);

			hr = pFolderDialog->Show(NULL);

			if (SUCCEEDED(hr)) {
				IShellItem* pItem;
				hr = pFolderDialog->GetResult(&pItem);
				if (SUCCEEDED(hr)) {
					PWSTR pszFolderPath;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFolderPath);
					if (SUCCEEDED(hr)) {
						int bufferSize = WideCharToMultiByte(CP_UTF8, 0, pszFolderPath, -1, NULL, 0, NULL, NULL);
						if (bufferSize > 0) {
							char* buffer = new char[bufferSize];
							WideCharToMultiByte(CP_UTF8, 0, pszFolderPath, -1, buffer, bufferSize, NULL, NULL);
							selectedFolder = buffer;
							delete[] buffer;
						}
						CoTaskMemFree(pszFolderPath);
					}
					pItem->Release();
				}
			}

			pFolderDialog->Release();
		}

		CoUninitialize();

		return selectedFolder;
	}
} // namespace Plaza

#elif __linux__
#include <stdio.h>
#include <stdlib.h>

namespace Plaza {
	std::string FileDialog::OpenFolderDialog() {
		char foldername[1024] = {0};
		FILE* fp = popen("zenity --file-selection --directory", "r");
		if (fp) {
			fgets(foldername, sizeof(foldername), fp);
			pclose(fp);
			foldername[strcspn(foldername, "\n")] = 0;
			return std::string(foldername);
		}

		return "";
	}
} // namespace Plaza
#endif
