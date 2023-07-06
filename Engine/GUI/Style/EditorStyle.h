#pragma once
#include "Engine/Vendor/imgui/imgui.h"
class EditorStyle {
public:
#pragma region Hierarchy
	ImVec4 hierarchyBackgroundColor = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);

#pragma region TreeNode
	ImVec4 treeNodeBackgroundColor = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
	ImVec4 selectedTreeNodeBackgroundColor = ImVec4(0.0f, 0.56f, 0.87f, 1.0f);
	ImVec4 treeNodeHoverBackgroundColor = ImVec4(0.135f, 0.135f, 0.135f, 1.0f);
	ImVec4 treeNodeActiveBackgroundColor = ImVec4(0.195f, 0.195f, 0.195f, 1.0f);
	
#pragma endregion TreeNode


#pragma endregion Hierarchy




};

extern EditorStyle editorStyle;