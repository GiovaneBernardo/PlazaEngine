#pragma once
#include "Editor/GUI/GuiWindow.h"

namespace Plaza {
	namespace Editor {
		class Console : public GuiWindow {
		  public:
			Console(GuiLayer layer, bool startOpen = true) : GuiWindow(layer, startOpen) {}

			void Init() override;
			void Update(Scene* scene) override;

			void Print(std::string value);
			void AddConsoleVariable(void* value, std::string name) {}

			struct TemporaryVariables {
				bool updateIndirectInstances = true;
			} mTemporaryVariables;

		  private:
			void OnKeyPress(int key, int scancode, int action, int mods) override {};

			std::map<std::string, void*> mVariables = std::map<std::string, void*>();
		};
	}; // namespace Editor
} // namespace Plaza
