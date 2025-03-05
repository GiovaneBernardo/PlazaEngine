#pragma once
#include <string>
namespace Plaza {
	using namespace std;
	class Standards {
	  public:
		static inline std::string engineExtName = ".plz";
		static inline std::string projectExtName = engineExtName + "prj";
		static inline std::string sceneExtName = engineExtName + "scn";
		static inline std::string materialExtName = engineExtName + "mat";
		static inline std::string modelExtName = engineExtName + "mod";
		static inline std::string prefabExtName = engineExtName + "prfb";
		static inline std::string scriptConfigExtName = engineExtName + "scp";
		static inline std::string metadataExtName = engineExtName + "meta";
		static inline std::string animationExtName = engineExtName + "anim";
		static inline std::string editorCacheExtName = engineExtName + "cache";
		static inline std::string plazaRenderGraph = engineExtName + "grph";
		static inline std::string editorSettingsExtName = engineExtName + "cfg";
	};
} // namespace Plaza
