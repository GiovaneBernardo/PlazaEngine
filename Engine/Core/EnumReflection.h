#pragma once
#include <iostream>
#include <typeinfo>
#include <algorithm>
#include <vector>
#include "ThirdParty/magic_enum/magic_enum.hpp"

namespace Plaza {
	class EnumReflection {
	public:
		template<typename T>
		static void RegisterEnum() {
			static_assert(std::is_enum<T>::value, "T must be an enum type.");
			
			const int enumSize = magic_enum::enum_count<T>();
			if (sEnumNamesByTypeRawName.find(typeid(T).name()) == sEnumNamesByTypeRawName.end()) {
				std::vector<const char*> names = std::vector<const char*>();
				for (int i = 0; i < enumSize; ++i) {
					names.push_back(magic_enum::enum_name(T(i)).data());
				}
				sEnumNamesByTypeRawName[typeid(T).name()] = names;
			}
		}

		template<typename T>
		static void RegisterBitmaskEnum() {
			static_assert(std::is_enum<T>::value, "T must be an enum type.");

			const int enumSize = magic_enum::enum_count<T>();
			if (sEnumNamesByTypeRawName.find(typeid(T).name()) == sEnumNamesByTypeRawName.end()) {
				std::vector<const char*> names = std::vector<const char*>();
				for (int i = 0; i < enumSize; ++i) {
					names.push_back(magic_enum::enum_name(T(1 << i)).data());
				}
				sEnumNamesByTypeRawName[std::string(typeid(T).name())] = names;
			}

			sBitmaskEnums.insert(typeid(T).name());
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

		static std::vector<int> GetEnumValues(const char* typeRawName) {
			std::vector<int> values = std::vector<int>();
			values.resize(GetEnumNames(typeRawName).size());
			for (int i = 0; i < values.size(); i++) {
				values[i] = 1 << i;
			}
			return values;
		}

		static bool HasTypeRawName(const char* rawName) {
			return sEnumNamesByTypeRawName.find(std::string(rawName)) != sEnumNamesByTypeRawName.end();
		}
	public:
		static inline std::map<std::string, std::vector<const char*>> sEnumNamesByTypeRawName = std::map<std::string, std::vector<const char*>>();
		static inline std::set<const char*> sBitmaskEnums = std::set<const char*>();
	};
}