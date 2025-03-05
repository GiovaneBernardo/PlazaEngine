#pragma once
#include <string>
#include <ThirdParty/glm/glm.hpp>
#include "Engine/Core/UUID.h"
#include "Engine/Core/Renderer/Texture.h"

namespace Plaza::Editor::Utils {
	struct GuiTable {
		ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
		std::string tableName = "";
		std::vector<std::string> names = std::vector<std::string>();
		std::vector<std::any> values = std::vector<std::any>();

		uint64_t tableUuid = Plaza::UUID::NewUUID();
	};
	static void ShowTable(GuiTable& table, unsigned int columnsCount = 2) {
		if (table.tableName.empty())
			table.tableName = table.tableUuid;

		ImGui::BeginTable(table.tableName.c_str(), columnsCount, table.flags);

		for (size_t i = 0; i < table.names.size(); ++i) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Text(table.names[i].c_str());
			ImGui::TableNextColumn();
			std::any_cast<std::function<void()>>(table.values[i])();
		}

		ImGui::EndTable();
	}
	static void AddTableValue(GuiTable& table, std::string name, std::function<void()> func) {
		table.names.push_back(name);
		table.values.push_back(func);
	}

	static void AddTableCheckbox(std::string name, bool* value, std::function<void(bool*)> callback = nullptr) {
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text(name.c_str());
		ImGui::TableNextColumn();
		if (ImGui::Checkbox(std::string("##" + name).c_str(), value))
			if (callback)
				callback(value);
	}

	static void AddTableVector4(std::string name, glm::vec4* value, float speed = 1.0f, float min = 0.0f,
								float max = 0.0f, const char* format = "%.3f",
								ImGuiSliderFlags flags = ImGuiSliderFlags_None,
								std::function<void(glm::vec4*)> callback = nullptr) {
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text(name.c_str());
		ImGui::TableNextColumn();
		if (ImGui::DragFloat4(std::string("##" + name).c_str(), &value->x, speed, min, max, format, flags))
			if (callback)
				callback(value);
	}

	static void AddTableVector3(std::string name, glm::vec3* value, float speed = 1.0f, float min = 0.0f,
								float max = 0.0f, const char* format = "%.3f",
								ImGuiSliderFlags flags = ImGuiSliderFlags_None,
								std::function<void(glm::vec3*)> callback = nullptr) {
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text(name.c_str());
		ImGui::TableNextColumn();
		if (ImGui::DragFloat3(std::string("##" + name).c_str(), &value->x, speed, min, max, format, flags))
			if (callback)
				callback(value);
	}

	static void AddTableVector2(std::string name, glm::vec2* value, float speed = 1.0f, float min = 0.0f,
								float max = 0.0f, const char* format = "%.3f",
								ImGuiSliderFlags flags = ImGuiSliderFlags_None,
								std::function<void(glm::vec2*)> callback = nullptr) {
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text(name.c_str());
		ImGui::TableNextColumn();
		if (ImGui::DragFloat2(std::string("##" + name).c_str(), &value->x, speed, min, max, format, flags))
			if (callback)
				callback(value);
	}

	static void AddTableFloat(std::string name, float* value, float speed = 1.0f, float min = 0.0f, float max = 0.0f,
							  const char* format = "%.3f", ImGuiSliderFlags flags = ImGuiSliderFlags_None,
							  std::function<void(float*)> callback = nullptr) {
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text(name.c_str());
		ImGui::TableNextColumn();
		if (ImGui::DragFloat(std::string("##" + name).c_str(), value, speed, min, max, format, flags))
			if (callback)
				callback(value);
	}

	static void AddTableInt(std::string name, int* value, float speed = 1.0f, float min = 0.0f, float max = 0.0f,
							const char* format = "%d", ImGuiSliderFlags flags = ImGuiSliderFlags_None,
							std::function<void(int*)> callback = nullptr) {
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text(name.c_str());
		ImGui::TableNextColumn();
		if (ImGui::DragInt(std::string("##" + name).c_str(), value, speed, min, max, format, flags))
			if (callback)
				callback(value);
	}

	static void AddTableSingleString(std::string name, ImGuiSliderFlags flags = ImGuiSliderFlags_None) {
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text(name.c_str());
		ImGui::TableNextColumn();
	}

	static bool AddTableButton(std::string name, bool* value, ImGuiSliderFlags flags = ImGuiSliderFlags_None,
							   std::function<void(bool*)> callback = nullptr) {
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		if (ImGui::Button(name.c_str())) {
			if (callback)
				callback(value);
			ImGui::TableNextColumn();
			return true;
		}
		ImGui::TableNextColumn();
		return false;
	}

	static bool AddTableTexture(std::string label, Texture* texture, ImGuiSliderFlags flags = ImGuiSliderFlags_None) {
		std::string id = std::to_string(texture->mAssetUuid) + label;
		ImGui::PushID(id.c_str());
		ImGui::TableNextRow();

		ImGui::TableNextColumn();
		ImGui::Text(label.c_str());
		ImGui::TableNextColumn();
		ImGui::DragFloat("Intensity", &texture->mIntensity);
		ImGui::TableNextColumn();
		bool textureButton = ImGui::Button("Texture");
		ImGui::TableNextColumn();
		std::string colorPickerName = "Color ##" + std::to_string(texture->mAssetUuid);
		ImGui::ColorEdit4(colorPickerName.c_str(), &texture->rgba.x, ImGuiColorEditFlags_NoInputs);
		ImGui::TableNextColumn();
		ImGui::PopID();

		return textureButton;
	}

	static void AddTableButtonString(std::string name, std::string buttonName, bool* value,
									 ImGuiSliderFlags flags = ImGuiSliderFlags_None,
									 std::function<void(bool*)> callback = nullptr) {
		ImGui::TableNextRow();
		ImGui::Text(name.c_str());
		ImGui::TableNextColumn();
		if (ImGui::Button(std::string(buttonName + "##" + name).c_str()))
			if (callback)
				callback(value);
		ImGui::TableNextColumn();
	}

	static float GetMinimumSpeed(float value, float divider, float min) {
		float dividedValue = glm::abs(value) / (divider * 10);
		float minimumSpeed = dividedValue > 0 ? dividedValue : min;
		return minimumSpeed > 0 ? minimumSpeed : min * divider;
	}

	static void Vec3Slider(std::string name, glm::vec3& vec3) {
		ImGui::SetNextItemWidth(225);
		ImGui::SliderFloat3(name.c_str(), &vec3.x, 0, 1.0f);
	}

	static void DragFloat(std::string name, float& value, float speed, std::function<void(float)> callback,
						  float min = std::numeric_limits<float>().max(),
						  float max = std::numeric_limits<float>().max(), const char* format = "%.3f",
						  bool maxZero = false) {
		ImGui::Text(name.c_str());
		ImGui::SameLine();
		if (ImGui::DragFloat(("##" + name).c_str(), &value, speed, min, max, format, ImGuiInputTextFlags_CallbackEdit))
			callback(value);
	}
	static void DragFloat(std::string name, float& value, float speed, float min = std::numeric_limits<float>().max(),
						  float max = std::numeric_limits<float>().max(), const char* format = "%.3f",
						  bool maxZero = false) {
		ImGui::Text(name.c_str());
		ImGui::SameLine();
		ImGui::DragFloat(("##" + name).c_str(), &value, speed, min, max, format, ImGuiInputTextFlags_CallbackEdit);
	}

	static void DragFloat3(std::string name, glm::vec3& value, float speed, std::function<void(glm::vec3)> callback,
						   float min = std::numeric_limits<float>().max(),
						   float max = std::numeric_limits<float>().max(), const char* format = "%.3f",
						   bool maxZero = false) {
		ImGui::PushItemWidth(75);
		ImGui::Text(name.c_str());
		ImGui::SameLine();
		ImGui::Text("X:");
		ImGui::SameLine();
		if (ImGui::DragFloat(("##x" + name).c_str(), &value.x, speed, min, max, format,
							 ImGuiInputTextFlags_CallbackEdit))
			callback(value);
		// ImGui::SetWindowSize(ImVec2(10, 50));
		ImGui::SameLine();
		ImGui::Text("Y:");
		ImGui::SameLine();
		if (ImGui::DragFloat(("##y" + name).c_str(), &value.y, speed, min, max, format,
							 ImGuiInputTextFlags_CallbackEdit))
			callback(value);
		ImGui::SameLine();
		ImGui::Text("Z:");
		ImGui::SameLine();
		if (ImGui::DragFloat(("##z" + name).c_str(), &value.z, speed, min, max, format,
							 ImGuiInputTextFlags_CallbackEdit))
			callback(value);
	}

	static bool DragFloat3(std::string name, glm::vec3& value, float speed,
						   float min = std::numeric_limits<float>().max(),
						   float max = std::numeric_limits<float>().max(), const char* format = "%.3f",
						   bool maxZero = false) {
		ImGui::PushItemWidth(75);
		ImGui::Text(name.c_str());
		ImGui::SameLine();
		ImGui::Text("X:");
		ImGui::SameLine();
		bool isDragging = false;
		if (ImGui::DragFloat(("##x" + name).c_str(), &value.x, speed, min, max, format,
							 ImGuiInputTextFlags_CallbackEdit))
			isDragging = true;
		// ImGui::SetWindowSize(ImVec2(10, 50));
		ImGui::SameLine();
		ImGui::Text("Y:");
		ImGui::SameLine();
		if (ImGui::DragFloat(("##y" + name).c_str(), &value.y, speed, min, max, format,
							 ImGuiInputTextFlags_CallbackEdit))
			isDragging = true;
		ImGui::SameLine();
		ImGui::Text("Z:");
		ImGui::SameLine();
		if (ImGui::DragFloat(("##z" + name).c_str(), &value.z, speed, min, max, format,
							 ImGuiInputTextFlags_CallbackEdit))
			isDragging = true;

		return isDragging;
	}

	static bool DragFloat4(std::string name, glm::vec4& value, float speed,
						   float min = std::numeric_limits<float>().max(),
						   float max = std::numeric_limits<float>().max(), const char* format = "%.3f",
						   bool maxZero = false) {
		ImGui::PushItemWidth(75);
		ImGui::Text(name.c_str());
		ImGui::SameLine();
		ImGui::Text("X:");
		ImGui::SameLine();
		bool isDragging = false;
		if (ImGui::DragFloat(("##x" + name).c_str(), &value.x, speed, min, max, format,
							 ImGuiInputTextFlags_CallbackEdit))
			isDragging = true;
		// ImGui::SetWindowSize(ImVec2(10, 50));
		ImGui::SameLine();
		ImGui::Text("Y:");
		ImGui::SameLine();
		if (ImGui::DragFloat(("##y" + name).c_str(), &value.y, speed, min, max, format,
							 ImGuiInputTextFlags_CallbackEdit))
			isDragging = true;
		ImGui::SameLine();
		ImGui::Text("Z:");
		ImGui::SameLine();
		if (ImGui::DragFloat(("##z" + name).c_str(), &value.z, speed, min, max, format,
							 ImGuiInputTextFlags_CallbackEdit))
			isDragging = true;
		ImGui::SameLine();
		ImGui::Text("W:");
		ImGui::SameLine();
		if (ImGui::DragFloat(("##W" + name).c_str(), &value.w, speed, min, max, format,
							 ImGuiInputTextFlags_CallbackEdit))
			isDragging = true;

		return isDragging;
	}
} // namespace Plaza::Editor::Utils
