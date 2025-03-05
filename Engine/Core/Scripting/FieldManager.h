#pragma once
#include <any>

namespace Plaza {
	class Field {
	  public:
		Field(){};
		Field(std::string name, std::any value, int type) : mName(name), mValue(value), mType(type) {}
		std::any mValue;
		std::string mName;
		int mType;
		std::map<std::string, Field*> mChildren = std::map<std::string, Field*>();
	};
	class FieldManager {
	  public:
		static std::map<std::string, Field*> GetFieldsValues(Field* parent = nullptr);
		static std::any GetFieldValue();
		static void FieldSetValue(int type, std::any& value, Field* field);
		static std::map<uint64_t, std::map<std::string, std::map<std::string, Field*>>> GetAllScritpsFields(
			Scene* scene);
		static void ApplyAllScritpsFields(
			Scene* scene, std::map<uint64_t, std::map<std::string, std::map<std::string, Field*>>> allFields);
	};
} // namespace Plaza
