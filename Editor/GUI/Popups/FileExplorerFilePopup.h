#pragma once
#include "Popup.h"
namespace Plaza::Editor {
	class Popup::FileExplorerFilePopup {
	  public:
		static void Init();
		static void Update(File* file);
		static void UpdateContent(File* file);
	};
} // namespace Plaza::Editor
