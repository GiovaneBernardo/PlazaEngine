#pragma once
#include "Editor/GUI/GuiWindow.h"

namespace Plaza {
	namespace Editor {
		class EditorTool {
		  public:
			enum class ToolType {
				NONE = 0,
				TERRAIN_EDITOR
			};

			ToolType mType;
			virtual void UpdateGui() {}
			virtual void OnMouseClick(int button, int action, int mods) {}
			virtual void OnKeyPress(int key, int scancode, int action, int mods) {}

			EditorTool() {
				// Callbacks::AddFunctionToMouseCallback({ OnMouseClick, Editor::GuiLayer::SCENE,
				// Editor::GuiState::HOVERED }); Callbacks::AddFunctionToKeyCallback({ OnKeyPress,
				// Editor::GuiLayer::SCENE, Editor::GuiState::HOVERED }); struct CallbackFunction {
				//	std::function<void(int key, int scancode, int action, int mods)> function;
				//	Editor::GuiLayer layerToExecute;
				//	Editor::GuiState layerStateToExecute = Editor::GuiState::FOCUSED;
				// };
			}

			void CaptureMouseClick(bool value) {
				// if (value == true)
				//	Gui::sEditorToolCaptureMouseClick = this->mType;
				// else if (Gui::sEditorToolCaptureMouseClick == this->mType)
				//	Gui::sEditorToolCaptureMouseClick = ToolType::NONE;
			}

			void CaptureKeyPress(bool value) {
				// if (value == true)
				//	Gui::sEditorToolCaptureKeyPress = this->mType;
				// else if (Gui::sEditorToolCaptureKeyPress == this->mType)
				//	Gui::sEditorToolCaptureKeyPress = ToolType::NONE;
			}

		  private:
			bool mCaptureExclusive = false;
		};
	} // namespace Editor
} // namespace Plaza
