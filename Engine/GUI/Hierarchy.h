#pragma once
#include "Engine/Components/GameObject.h"
#include "Engine/GUI/guiMain.h"
#include <imgui/imgui.h>

namespace Gui {
	namespace Hierarchy {
		class Item { // Item consists of a Treenode and a selectable
		public:
			GameObject* gameObject;
			GameObject*& selectedGameObject;

			Item(GameObject* gameObject, GameObject*& selectedGameObject) : gameObject(gameObject), selectedGameObject(selectedGameObject) {
				// Push the gameObject id, to prevent it to collpases all the treenodes with same id
				ImGui::PushID(gameObject->id.c_str());
				// Start the treenode before the component selectable, but only assign its values after creating the button
				bool treeNodeOpen = ImGui::TreeNodeEx("");
				ImGui::SameLine();
				ImGui::Selectable(gameObject->name.c_str());
				// Change the selected gameobject if user clicked on the selectable
				if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
					selectedGameObject = gameObject;

				if (treeNodeOpen)
				{
					// Content inside the TreeNode
					ImGui::Text("Content inside the TreeNode");

					ImGui::TreePop();
				}
				ImGui::PopID();
			}
		};



	}
}
