#pragma once

namespace Plaza {
	enum class RendererAPI;
}
namespace Plaza::Editor{

	static class Settings {
	public:
		std::string mName = "editor";
		bool mVsync = false;
		RendererAPI mDefaultRendererAPI;

		void ReapplyAllSettings();

		template <class Archive>
		void serialize(Archive& archive) {
			archive(mName, mVsync, mDefaultRendererAPI);
		}
	};
}