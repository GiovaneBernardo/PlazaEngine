#pragma once

namespace Plaza {
	namespace Editor {
		enum GuiLayer {
			UNKNOWN = 0,
			HIERARCHY,
			SCENE,
			EDITOR,
			INSPECTOR,
			FILE_EXPLORER,
			ASSETS_IMPORTER,
			CONSOLE,
			RENDER_GRAPH_EDITOR
		};

		enum class GuiState {
			UNKNOWN = 0,
			FOCUSED,
			UNFOCUSED,
			HOVERED
		};

		class GuiWindow {
		  public:
			GuiWindow(GuiLayer layer, bool startOpen = true) : mLayer(layer), mIsOpen(startOpen) {}
			virtual void Init() = 0;
			virtual void Update(Scene* scene) = 0;
			virtual void OnKeyPress(int key, int scancode, int action, int mods) {};

			// virtual void HandleKeyEvent();
			// virtual void HandleFocusedKeyEvent();
			// virtual void HandleHoveredKeyEvent();

			const bool IsHovered() { return mHovered; };
			const bool IsFocused() { return mFocused; };

			/// <summary>
			/// Only call this after the window creation
			/// </summary>
			void UpdateHovered() { mHovered = ImGui::IsWindowHovered() || ImGui::IsItemHovered(); }

			/// <summary>
			/// Only call this after the window creation
			/// </summary>
			void UpdateFocused() { mFocused = ImGui::IsWindowFocused() || ImGui::IsItemFocused(); }

			GuiLayer GetLayer() { return mLayer; }
			bool IsOpen() { return mIsOpen; }
			void SetOpen(bool value) { mIsOpen = value; }

			bool mExpanded = true;
			bool mIsOpen = true;

		  private:
			GuiLayer mLayer = GuiLayer::UNKNOWN;
			GuiState mState = GuiState::UNKNOWN;
			bool mHovered = false;
			bool mFocused = false;
		};
	} // namespace Editor
} // namespace Plaza
