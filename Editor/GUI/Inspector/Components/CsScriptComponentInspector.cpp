#include "Engine/Core/PreCompiledHeaders.h"
#include "ComponentsInspector.h"
#include "Engine/Components/Scripting/CsScriptComponent.h"
#include "Engine/Core/Scripting/FieldManager.h"
#include "Engine/Core/Scene.h"
#include "Editor/GUI/Utils/Utils.h"
#include <string>

void CreateRespectiveInteractor(MonoObject* monoObject, MonoClassField* field, int& sliderIndex, MonoClass* monoClass = nullptr) {
	int type = mono_type_get_type(mono_field_get_type(field));

	std::string fieldName = mono_field_get_name(field);
	std::string fieldId = "##: " + std::to_string(sliderIndex) + fieldName;

	if (type == MONO_TYPE_R4 || type == MONO_TYPE_R8) { // Float
		float value;
		mono_field_get_value(monoObject, field, &value);
		if (ImGui::DragFloat(fieldId.c_str(), &value)) {
			mono_field_set_value(monoObject, field, &value);
		}
	}
	else if (type == MONO_TYPE_I4) { // Int
		int value;
		mono_field_get_value(monoObject, field, &value);
		if (ImGui::DragInt(fieldId.c_str(), &value)) {
			mono_field_set_value(monoObject, field, &value);
		}
	}
	else if (type == MONO_TYPE_U8) {
		uint64_t value;
		mono_field_get_value(monoObject, field, &value);
		std::string stringValue = std::to_string(value);

		// Create a non-const character buffer and copy the characters
		char* charBuffer = new char[stringValue.length() + 1];
		strcpy(charBuffer, stringValue.c_str());
		if (ImGui::InputText(fieldId.c_str(), charBuffer, 2048)) {
			uint64_t result = 0;
			result = std::strtoull(charBuffer, nullptr, 10);
			mono_field_set_value(monoObject, field, &result);
		}
	}
	else if (type == MONO_TYPE_CLASS) { // Class
		MonoObject* newMonoObject = nullptr;
		mono_field_get_value(monoObject, field, &newMonoObject);
		if (newMonoObject != nullptr) {
			void* iter = NULL;
			MonoClass* classInstance = mono_object_get_class(newMonoObject);
			MonoClassField* subField;
			while ((subField = mono_class_get_fields(classInstance, &iter)) != NULL) {
				sliderIndex++;
				ImGui::Text(mono_field_get_name(subField));
				ImGui::Text("  Sub  ");
				CreateRespectiveInteractor(newMonoObject, subField, sliderIndex);
			}
			// Recursively collect fields from the base class if available
			MonoClass* baseClass = mono_class_get_parent(classInstance);
			if (baseClass) {
				void* iter2 = NULL;
				MonoClassField* parentField;
				//std::cout << mono_field_get_name(field) << "\n";
				while ((parentField = mono_class_get_fields(baseClass, &iter2)) != NULL) {
					//std::cout << mono_field_get_name(parentField) << "\n";
					CreateRespectiveInteractor(newMonoObject, parentField, sliderIndex);
					//CreateRespectiveInteractor(nullptr, parentField, sliderIndex);
				}
			}
		}
	}
	sliderIndex++;
}
namespace Plaza::Editor {
	void ComponentsInspector::CsScriptComponentInspector(Scene* scene, Entity* entity) {
		CsScriptComponent* scriptComponent = scene->GetComponent<CsScriptComponent>(entity->uuid);
		ImGui::PushID("CsScriptComponentInspector");
		if (Utils::ComponentInspectorHeader(scriptComponent, "Script Component")) {
			int sliderIndex = 0;
			// Get the fields of the class
			for (auto& [key, scriptClass] : scriptComponent->scriptClasses) {
				MonoClassField* field = NULL;
				void* iter = NULL;
				std::unordered_map<std::string, uint32_t> classFields;
				if (scriptClass->monoObject->vtable) {
					while ((field = mono_class_get_fields(mono_object_get_class(scriptClass->monoObject), &iter)) != NULL)
					{
						ImGui::Text(mono_field_get_name(field));
						CreateRespectiveInteractor(scriptClass->monoObject, field, sliderIndex);
					}
				}
				else {
					ImGui::Text("Vtable is a nullptr");
				}
			}
		}
		ImGui::PopID();
	}
}