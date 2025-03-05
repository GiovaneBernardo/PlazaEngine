#pragma once
#include <string>
#include <map>
#include "Engine/Core/Scripting/FieldManager.h"
#include "Editor/GUI/guiMain.h"
#include "Editor/Settings/EditorSettings.h"
#include "Editor/CopyPasteManager.h"

namespace Plaza {
	namespace Editor {
		class EditorClass {
		  public:
			Gui mGui;
			std::string directory;
			Settings mSettings;

			CopyPasteManager* mCopyPasteManager = new CopyPasteManager();
		};
	} // namespace Editor
} // namespace Plaza
