#include "Engine/Core/PreCompiledHeaders.h"
#include "Editor/GUI/Hierarchy/Hierarchy.h"
#include "Engine/Utils/vectorUtils.h"
using namespace Engine;
void payloadDrop(Engine::GameObject* gameObject, Engine::GameObject* currentObj, ImVec2 treeNodeMin, ImVec2 treeNodeMax, const ImGuiPayload* payload);
void InsertBefore(Engine::GameObject* payloadObj, Engine::GameObject* currentObj);
void InsertAsChild(Engine::GameObject* payloadObj, Engine::GameObject* currentObj);
void InsertAfter(Engine::GameObject* payloadObj, Engine::GameObject* currentObj);


namespace Engine::Editor {
	void Gui::Hierarchy::Item::HierarchyDragDrop(GameObject& gameObject, GameObject* currentObj, ImVec2 treeNodeMin, ImVec2 treeNodeMax) {
		if (gameObject.parentUuid && ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadName.c_str())) {
				payloadDrop(&gameObject, currentObj, treeNodeMin, treeNodeMax, payload);
			}
			ImGui::EndDragDropTarget();
		}
	}
}
void payloadDrop(Engine::GameObject* gameObject, Engine::GameObject* currentObj, ImVec2 treeNodeMin, ImVec2 treeNodeMax, const ImGuiPayload* payload) {
	//if (payload->DataSize == sizeof(Engine::GameObject*)) {
	//	/* The current "object" is the one that the mouse was over when dropped*/
	//	/* PayloadObj is the dragged object */
	//	Engine::GameObject* payloadObj = *static_cast<Engine::GameObject**>(payload->Data);  // Dereference the pointer			
	//	std::vector<Engine::GameObject*>& currentChildren = currentObj->GetParent()->children;
	//	std::vector<Engine::GameObject*>& payloadChildren = payloadObj->parent->children;

	//	ImVec2 mousePos = ImGui::GetMousePos();
	//	ImVec2 treeNodePos = treeNodeMin;
	//	ImVec2 treeNodeSize = ImVec2(treeNodeMax.x - treeNodeMin.x, treeNodeMax.y - treeNodeMin.y);
	//	ImVec2 treeNodeCenter = ImVec2(treeNodePos.x, treeNodePos.y + treeNodeSize.y / 2);

	//	std::cout << "Mouse Pos: x " << mousePos.x << " y " << mousePos.y << std::endl;
	//	std::cout << "Tree Node Pos: x " << treeNodePos.x << ", y " << treeNodePos.y << std::endl;
	//	std::cout << "Tree Node Size: x " << treeNodeSize.x << ", y " << treeNodeSize.y << std::endl;
	//	std::cout << "Tree Node Center: x " << treeNodeCenter.x << ", y " << treeNodeCenter.y << std::endl;

	//	if (mousePos.y < treeNodeCenter.y - treeNodeSize.y * 0.15f) { // Top
	//		InsertBefore(payloadObj, currentObj);
	//	}
	//	else if (mousePos.y > treeNodeCenter.y + treeNodeSize.y * 0.15f) { // Bottom

	//		if (currentObj->children.size() > 0) {
	//			InsertAsChild(payloadObj, currentObj);
	//		}
	//		else {
	//			InsertAfter(payloadObj, currentObj);
	//		}
	//	}
	//	else { // Center
	//		InsertAsChild(payloadObj, currentObj);
	//	}
	//}
}

void InsertBefore(Engine::GameObject* payloadObj, Engine::GameObject* currentObj) {
	//// Becomes child of the target's parent
	//payloadObj->parent->children.erase(std::remove(payloadObj->parent->children.begin(), payloadObj->parent->children.end(), payloadObj), payloadObj->parent->children.end());
	//payloadObj->parent = currentObj->parent;
	//currentObj->parent->children.insert(currentObj->parent->children.begin() + Utils::Vector::indexOf(currentObj->parent->children, currentObj), payloadObj);
	////  PayloadObj index now must be currentObj index - 1, so it comes before

	//// Update positions
	//payloadObj->parent->transform->UpdateChildrenTransform();
}

void InsertAfter(Engine::GameObject* payloadObj, Engine::GameObject* currentObj) {
	//payloadObj->parent->children.erase(std::remove(payloadObj->parent->children.begin(), payloadObj->parent->children.end(), payloadObj), payloadObj->parent->children.end());
	//payloadObj->parent = currentObj->parent;
	//currentObj->parent->children.insert(currentObj->parent->children.begin() + Utils::Vector::indexOf(currentObj->parent->children, currentObj) + 1, payloadObj);
	//payloadObj->parent->transform->UpdateChildrenTransform();
}

void InsertAsChild(Engine::GameObject* payloadObj, Engine::GameObject* currentObj) {
	//payloadObj->parent->children.erase(std::remove(payloadObj->parent->children.begin(), payloadObj->parent->children.end(), payloadObj), payloadObj->parent->children.end());
	//payloadObj->parent = currentObj;
	//currentObj->children.insert(currentObj->children.begin(), payloadObj);
	//payloadObj->parent->transform->UpdateChildrenTransform();
}