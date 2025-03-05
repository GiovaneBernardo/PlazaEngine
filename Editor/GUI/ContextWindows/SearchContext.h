#pragma once

namespace Plaza::Editor {
	class SearchContext : public GuiWindow {
	  public:
		std::function<void(uint64_t)> mCallback;
		SearchContext(std::string name, GuiLayer layer = GuiLayer::UNKNOWN, bool startOpen = true)
			: mName(name), GuiWindow(layer, startOpen) {
			this->SetOpen(false);
		};
		std::string mName;
		char mSearchBuffer[512] = "";
		// std::vector<std::any&> mValues;
		void Init() override;
		void Update(Scene* scene) override;
		void SetValues();
		void OnKeyPress(int key, int scancode, int action, int mods) override {}
		void ExecuteCallback(uint64_t uuid) {
			mCallback(uuid);
			this->SetOpen(false);
		}
		// void SetValues(std::vector<std::any&>& values);
	};
} // namespace Plaza::Editor
