#pragma once
#include <variant>
#include <any>
#include <iostream>
#include <typeinfo>
#include <typeindex>
#include "Engine/Core/Engine.h"
#include "Engine/Core/EnumReflection.h"

namespace Plaza {
	class Any {
	  public:
		Any() noexcept : mType(&typeid(void)) {
			// mTypeRawName = mType->name();
		}

		~Any() { Reset(); }

		template <typename T> void SetValue(T value, bool changeType = true) {
			mValue = static_cast<void*>(new T(value));

			if (changeType) {
				mSize = sizeof(T);
				// if (mValue)
				Any::RegisterType<T>();
				mType = &typeid(T);
				mTypeRawName = mType->name();
			}
		}

		template <typename T> void SetValuePtr(T* value, bool changeType = true) {
			mValue = value; // static_cast<void*>(new T(value));

			// if (changeType) {
			//	//mSize = sizeof(T);
			//	if (mValue)
			//		this->RegisterType<T>();
			//	mType = &typeid(T);
			// }
		}

		template <typename T> void SetType() {
			Any::RegisterType<T>();
			mType = &typeid(T);
			mTypeRawName = mType->name();
		}

		void SetType(const type_info& type) {
			mType = &type;
			mTypeRawName = mType->name();
		}

		template <typename T> T* GetValue() const {
			if constexpr (std::is_reference_v<T>) {
				return static_cast<T*>(mValue); // Return the address for references
			}
			else {
				return static_cast<T*>(mValue);
			}
		}

		static void CopyValue(Any& from, Any& to) {
			if (sCopyValueFactoryByRawName.find(from.mTypeRawName) != sCopyValueFactoryByRawName.end())
				sCopyValueFactoryByRawName.at(from.mTypeRawName)(from, to);
			else if (sCopyValueFactoryByRawName.find(to.mTypeRawName) != sCopyValueFactoryByRawName.end())
				sCopyValueFactoryByRawName.at(to.mTypeRawName)(from, to);
		}

		template <typename T> Any& operator=(T&& value) {
			SetValue(std::forward<T>(value), true);
			return *this;
		}

		const std::type_info& type() const { return *mType; }

		void Reset() {
			// if (mValue)
			//	delete mValue; // Type-safe deletion
			mValue = nullptr;
			mType = &typeid(void);
			mTypeRawName = mType->name();
		}

		void* mValue = nullptr;
		uintptr_t mSize = 0;

		template <typename T> struct always_false : std::false_type {};
		template <class Archive> void serialize(Archive& archive) {
			archive(PL_SER(mTypeRawName));
			if (mTypeRawName == "")
				return;
			if constexpr (std::is_same_v<Archive, cereal::JSONOutputArchive> ||
						  std::is_same_v<Archive, cereal::JSONInputArchive>) {
				// Handle JSON-specific serialization or deserialization
				if constexpr (Archive::is_saving::value) {
					if (mValue == nullptr)
						return;

					if (EnumReflection::sEnumNamesByTypeRawName.find(mTypeRawName) !=
						EnumReflection::sEnumNamesByTypeRawName.end())
						sJsonSerializeByRawName[typeid(int).name()](archive, *this);
					else
						sJsonSerializeByRawName[mTypeRawName](archive, *this);
				}
				else if constexpr (Archive::is_loading::value) {
					// JSON deserialization
					// Example: archive(value);
					if (EnumReflection::sEnumNamesByTypeRawName.find(mTypeRawName) !=
						EnumReflection::sEnumNamesByTypeRawName.end())
						sJsonDeSerializeByRawName[typeid(int).name()](archive, *this);
					else if (sJsonDeSerializeByRawName.find(mTypeRawName) != sJsonDeSerializeByRawName.end())
						sJsonDeSerializeByRawName[mTypeRawName](archive, *this);
				}
			}
			else if constexpr (std::is_same_v<Archive, cereal::BinaryOutputArchive> ||
							   std::is_same_v<Archive, cereal::BinaryInputArchive>) {
				// Handle Binary-specific serialization or deserialization
				if constexpr (Archive::is_saving::value) {
					if (mValue == nullptr)
						return;
					// Binary serialization
					// Example: archive(value);
					if (EnumReflection::sEnumNamesByTypeRawName.find(mTypeRawName) !=
						EnumReflection::sEnumNamesByTypeRawName.end())
						sBinarySerializeByRawName[typeid(int).name()](archive, *this);
					else
						sBinarySerializeByRawName[mTypeRawName](archive, *this);
				}
				else if constexpr (Archive::is_loading::value) {
					// Binary deserialization
					// Example: archive(value);
					if (EnumReflection::sEnumNamesByTypeRawName.find(mTypeRawName) !=
						EnumReflection::sEnumNamesByTypeRawName.end())
						sBinaryDeSerializeByRawName[typeid(int).name()](archive, *this);
					else
						sBinaryDeSerializeByRawName[mTypeRawName](archive, *this);
				}
			}
			else {
				static_assert(always_false<Archive>::value, "Unsupported archive type");
			}
		}

		template <typename T> static void RegisterType() {
			////mTypeRawName = mType->name();
			// std::string typeRawName = typeid(T).name();
			// if (sRegisteredRawNames.find(typeRawName) != sRegisteredRawNames.end())
			//	return;
			//// Setup serialization
			// sJsonSerializeByRawName.emplace(typeRawName, [](cereal::JSONOutputArchive& archive, Any& any) {
			//	T* newValue = static_cast<T*>(any.mValue);
			//	archive(*newValue); // Serialize the value
			//	});
			// sBinarySerializeByRawName.emplace(typeRawName, [](cereal::BinaryOutputArchive& archive, Any& any) {
			//	T* newValue = static_cast<T*>(any.mValue);
			//	archive(*newValue); // Serialize the value
			//	});
			//
			//// Setup deserialization
			// sJsonDeSerializeByRawName.emplace(typeRawName, [](cereal::JSONInputArchive& archive, Any& any) {
			//	T newValue;
			//	archive(newValue); // Deserialize into newValue
			//	any.SetValue(newValue, false); // Store the new valu)e
			//	});
			// sBinaryDeSerializeByRawName.emplace(typeRawName, [](cereal::BinaryInputArchive& archive, Any& any) {
			//	T newValue;
			//	archive(newValue); // Deserialize into newValue
			//	any.SetValue(newValue, false); // Store the new valu)e
			//	});
			//
			// sCopyValueFactoryByRawName.emplace(typeRawName, [](Any& from, Any& to) {
			//	if (from.mValue == nullptr)
			//		return;
			//	T* newValue = new T(*from.GetValue<T>());
			//	//archive(newValue);
			//	//any.SetValue(newValue, false);
			//	to.mValue = static_cast<void*>(newValue);
			//	});
		}

		static inline std::set<std::string> sRegisteredRawNames = std::set<std::string>();
		static inline std::unordered_map<std::string, std::function<void(cereal::JSONOutputArchive&, Any& any)>>
			sJsonSerializeByRawName =
				std::unordered_map<std::string, std::function<void(cereal::JSONOutputArchive&, Any& any)>>();
		static inline std::unordered_map<std::string, std::function<void(cereal::JSONInputArchive&, Any& any)>>
			sJsonDeSerializeByRawName =
				std::unordered_map<std::string, std::function<void(cereal::JSONInputArchive&, Any& any)>>();

		static inline std::unordered_map<std::string, std::function<void(cereal::BinaryOutputArchive&, Any& any)>>
			sBinarySerializeByRawName =
				std::unordered_map<std::string, std::function<void(cereal::BinaryOutputArchive&, Any& any)>>();
		static inline std::unordered_map<std::string, std::function<void(cereal::BinaryInputArchive&, Any& any)>>
			sBinaryDeSerializeByRawName =
				std::unordered_map<std::string, std::function<void(cereal::BinaryInputArchive&, Any& any)>>();

		static inline std::map<std::string, std::function<void(Any& from, Any& to)>> sCopyValueFactoryByRawName =
			std::map<std::string, std::function<void(Any& from, Any& to)>>();

	  private:
		const std::type_info* mType;
		std::string mTypeRawName = "";
	};
} // namespace Plaza
