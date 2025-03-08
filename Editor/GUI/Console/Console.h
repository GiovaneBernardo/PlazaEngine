#pragma once
#include "Editor/GUI/GuiWindow.h"
#include "Engine/Core/Time.h"

namespace Plaza {
	namespace Editor {
		struct ConsoleMessage {
			ConsoleMessage(std::string message, int severity = 0) : mMessage(message), mSeverity(severity) {
				mTime = time(&mTime);
			}
			std::string mMessage;
			int mSeverity = 0; // Bigger = More severe; 0 = INFO; 1 = WARNING; 2 = ERROR; 3 = FATAL ERROR
			time_t mTime;
		};

		class Console : public GuiWindow {
		  public:
			Console(GuiLayer layer, bool startOpen = true) : GuiWindow(layer, startOpen) {}
			char mConsoleInput[4096] = "";

			void Init() override;
			void Update(Scene* scene) override;

			void Print(std::string value);
			void AddConsoleVariable(void* value, std::string name) {}

			struct TemporaryVariables {
				bool updateIndirectInstances = true;
			} mTemporaryVariables;

			void AddMessage(const std::string& messageContent, int severity);

		  private:
			void OnKeyPress(int key, int scancode, int action, int mods) override {};

			std::map<std::string, void*> mVariables = std::map<std::string, void*>();

			std::vector<ConsoleMessage> mMessages = std::vector<ConsoleMessage>();
		};
	}; // namespace Editor
} // namespace Plaza
