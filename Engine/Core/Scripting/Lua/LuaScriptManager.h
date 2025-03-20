#include "LuaScript.h"
#include <sol/sol.hpp>

namespace Plaza {
	class PLAZA_API LuaScriptManager {
	  public:
		static void Init();
		static void Terminate();

		static void AddLuaScriptToEntity(Scene* scene, uint64_t entity, Asset* scriptAsset);

	  private:
		static inline std::shared_ptr<sol::state> mLua = std::make_shared<sol::state>();
	};
} // namespace Plaza
