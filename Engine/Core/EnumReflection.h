#pragma once
#include <iostream>
#include <typeinfo>
#include <algorithm>
#include <vector>

namespace Plaza {
	class EnumReflection {
	public:
		template<typename T>
		static void RegisterEnum() {
			static_assert(std::is_enum<T>::value, "T must be an enum type.");
			
			const int enumSize = magic_enum::enum_count<T>();
			if (sEnumNamesByTypeRawName.find(typeid(T).raw_name()) == sEnumNamesByTypeRawName.end()) {
				std::vector<const char*> names = std::vector<const char*>();
				for (int i = 0; i < enumSize; ++i) {
					names.push_back(magic_enum::enum_name(T(i)).data());
				}
				sEnumNamesByTypeRawName[typeid(T).raw_name()] = names;
			}
		}

		template<typename T>
		static void RegisterBitmaskEnum() {
			static_assert(std::is_enum<T>::value, "T must be an enum type.");

			const int enumSize = magic_enum::enum_count<T>();
			if (sEnumNamesByTypeRawName.find(typeid(T).raw_name()) == sEnumNamesByTypeRawName.end()) {
				std::vector<const char*> names = std::vector<const char*>();
				for (int i = 0; i < enumSize; ++i) {
					names.push_back(magic_enum::enum_name(T(i)).data());
				}
				sEnumNamesByTypeRawName[typeid(T).raw_name()] = names;
			}

			sBitmaskEnums.insert(typeid(T).raw_name());
		}

		static bool IsBitmask(const char* typeRawName) {
			return sBitmaskEnums.find(typeRawName) != sBitmaskEnums.end();
		}



		static const char* GetEnumName(const char* typeRawName, int index) {
			const auto& it = sEnumNamesByTypeRawName.find(typeRawName);
			if (it != sEnumNamesByTypeRawName.end() && it->second.size() > index)
				return it->second[index];
			return "";
		}

		static std::vector<const char*> GetEnumNames(const char* typeRawName) {
			const auto& it = sEnumNamesByTypeRawName.find(typeRawName);
			if (it != sEnumNamesByTypeRawName.end())
				return it->second;
			return std::vector<const char*>();
		}

		static bool HasTypeRawName(const char* rawName) {
			return sEnumNamesByTypeRawName.find(rawName) != sEnumNamesByTypeRawName.end();
		}
	public:
		static inline std::map<const char*, std::vector<const char*>> sEnumNamesByTypeRawName = std::map<const char*, std::vector<const char*>>();
		static inline std::set<const char*> sBitmaskEnums = std::set<const char*>();
	};
}