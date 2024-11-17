#pragma once

namespace Plaza::Editor {
	class PrimitivesInspector {
	public:
		static void Init();

		static bool InspectAny(Any& any);

		static bool InspectString(const std::string& name, std::string& value);
		static bool InspectBool(const std::string& name, bool& value);
		static bool InspectEnum(Any& any);
		static bool InspectFloat(const std::string& name, float& value);
		static bool InspectInt(const std::string& name, int& value);
		static bool InspectUInt(const std::string& name, unsigned int& value);
		static bool InspectUInt32(const std::string& name, uint32_t& value);

		static inline std::map<const char*, std::function<bool()>> sFunctionsByRawName = std::map<const char*, std::function<bool()>>();
	};
}