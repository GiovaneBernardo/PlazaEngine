#include "Engine/Core/PreCompiledHeaders.h"
#include "FieldManager.h"
#include "Engine/Core/Scene.h"

namespace Plaza {
	std::any FieldManager::GetFieldValue() {
		return std::any();
	}

	void FieldManager::FieldSetValue(int type, std::any& value, Field* field) {

	}


	std::map<std::string, Field*> FieldManager::GetFieldsValues(Field* parent) {
		std::map<std::string, Field*> fields = std::map<std::string, Field*>();
		return fields;
	}

	std::map<uint64_t, std::map<std::string, std::map<std::string, Field*>>> FieldManager::GetAllScritpsFields(Scene* scene) {
		std::map<uint64_t, std::map<std::string, std::map<std::string, Field*>>> allFields = std::map<uint64_t, std::map<std::string, std::map<std::string, Field*>>>(); // Entity UUID, Class Name, Field Name

		return allFields;
	}

	void FieldManager::ApplyAllScritpsFields(Scene* scene, std::map<uint64_t, std::map<std::string, std::map<std::string, Field*>>> allFields) {
		
	}
}