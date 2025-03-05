#pragma once
#include "Editor/GUI/GuiWindow.h"
#include "Engine/Core/Engine.h"

namespace Plaza {
	class PLAZA_API Input {
	  public:
		static inline bool isAnyKeyPressed = false;
		static inline string mCheckFocusedMenu = "";
		class Cursor;
		static void Update();
		static bool GetKeyDown(int key);
		static bool GetKeyDownOnce();
		static bool GetKeyReleased();
		static bool GetKeyReleasedOnce();
		static void AddFunctionToOnKeyPress(std::function<void()> function,
											Editor::GuiLayer layer = Editor::GuiLayer::SCENE,
											Editor::GuiState layerState = Editor::GuiState::FOCUSED) {}
		static void SetFocusedMenuCheck(const std::string& menu);
		static bool GetMouseDown(int button);
		static glm::vec2 GetScreenSize();

	  private:
	};
} // namespace Plaza
