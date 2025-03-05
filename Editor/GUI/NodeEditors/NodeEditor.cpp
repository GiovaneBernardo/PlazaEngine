#include "Engine/Core/PreCompiledHeaders.h"
#include "NodeEditor.h"
#include "Engine/Application/FileDialog/FileDialog.h"
#include <algorithm>
#include <vector>
#include <iostream>
#include "Editor/GUI/Inspector/Primitives/PrimitivesInspector.h"
#include <typeinfo>
#include "Engine/Core/AssetsManager/Serializer/AssetsSerializer.h"

namespace Plaza::Editor {
	void NodeEditor::InitNodeEditor() {
		ax::NodeEditor::Config config;
		config.SettingsFile = nullptr;
		// config.SettingsFile = "Simple.json";
		mContext = ax::NodeEditor::CreateEditor(&config);

		// Node finalNode = Node();
		// finalNode.name = "Final Node";
		// this->AddInputPin(finalNode, Pin(0, "In", PinType::Object));
		// this->AddOutputPin(finalNode, Pin(0, "Out", PinType::Object));
		// finalNode.processFunction = [](Node& node) {
		//	Node& previousNode = *node.inputs[0].nodes[1];
		//	node.outputs[0].value = previousNode.outputs[0].value;//.SetValue(previousNode.outputs[0].GetValue<void*>(),
		//false);
		//	};
		// this->AddNodeToCreate(finalNode);
		//
		// finalNode = this->SpawnNode("Final Node");
		// mFinalNodeId = finalNode.id;

		Node emptyNode = Node();
		emptyNode.name = "Empty Node";
		this->AddOutputPin(emptyNode, Pin(0, "Out", PinType::Unknown));
		emptyNode.processFunction = [](Node& node) {};
		this->AddNodeToCreate(emptyNode);

		this->InitMathNodes();
	};

	void NodeEditor::Process() {
		ax::NodeEditor::SetCurrentEditor(mContext);
		int nodeCount = ax::NodeEditor::GetNodeCount();
		std::vector<ax::NodeEditor::NodeId> orderedNodeIds;
		orderedNodeIds.resize(static_cast<size_t>(nodeCount));
		ax::NodeEditor::GetOrderedNodeIds(orderedNodeIds.data(), nodeCount);
		for (unsigned int i = 0; i < nodeCount; ++i) {
			ax::NodeEditor::NodeId nodeId = orderedNodeIds[i];
			if (GetNode(nodeId.Get()))
				GetNode(nodeId.Get())->processFunction(*GetNode(nodeId.Get()));
			// auto it = std::find_if(mNodes.begin(), mNodes.end(), [&](const Node& node) {
			//	return node.id == nodeId.Get();
			//	});
			// if (it != mNodes.end())
			//	std::swap(*it, mNodes[i]);
		}
		ax::NodeEditor::SetCurrentEditor(nullptr);

		for (auto& [key, node] : mNodesData.mNodes) {
			node.processFunction(node);
		}
	}

	void NodeEditor::CopyDeSerializedInput(Pin& from, Pin& to) {
		to.subNodes.clear();
		for (Node& subNode : from.subNodes) {
			// pin.subNodes.push_back(subNode);
			SpawnNode(subNode.name, &to, false);
		}

		Any::CopyValue(from.value, to.value);

		int subNodeIndex = 0;
		for (Node& subNode : to.subNodes) {
			int inputIndex = 0;
			for (Pin& input : subNode.inputs) {
				CopyDeSerializedInput(from.subNodes[subNodeIndex].inputs[inputIndex], input);
				inputIndex++;
			}
			subNodeIndex++;
		}
	}

	void NodeEditor::Update(Scene* scene) {
		ImGuiWindowFlags windowFlags =
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoNavFocus;

		if (ImGui::Begin("Render Graph Editor", &mExpanded, windowFlags)) {
			ImGui::BeginChild("Selection", ImVec2(300.0f, 0));

			if (ImGui::Button("Process")) {
				this->Process();
			}

			if (ImGui::Button("Serialize Nodes")) {
				AssetsSerializer::SerializeFile<NodesData>(
					mNodesData, FileDialog::SaveFileDialog(Standards::plazaRenderGraph.c_str()),
					Application::Get()->mSettings.mRenderGraphSerializationMode);
				// Application::Get()->mEditor->mGui.mRenderGraphEditor->LoadRenderGraphNodes(graph);
			}

			if (ImGui::Button("DeSerialize Nodes")) {
				std::shared_ptr<NodesData> data = AssetsSerializer::DeSerializeFile<NodesData>(
					FileDialog::OpenFileDialog(Standards::plazaRenderGraph.c_str()),
					Application::Get()->mSettings.mRenderGraphSerializationMode);
				PL_CORE_INFO(data->mNodes.size());
				mNodesData = NodesData(); //= *data.get();
				std::map<uintptr_t, uintptr_t> correctedIds = std::map<uintptr_t, uintptr_t>();
				for (auto& [key, node] : data->mNodes) {
					Node& newNode = this->SpawnNode(node.name, nullptr, false);
					correctedIds.emplace(newNode.id, key);
				}

				for (auto& [key, node] : mNodesData.mNodes) {
					uintptr_t correctedId = correctedIds.at(key);
					// if (data->mNodes[correctedId].id != key)
					//	continue;
					int index = 0;
					for (auto& pin : node.inputs) {
						// pin.subNodes.clear();
						// for (Node& subNode : data->mNodes[correctedId].inputs[index].subNodes) {
						//	//pin.subNodes.push_back(subNode);
						//	SpawnNode(subNode.name, &pin, false);
						// }
						// data->mNodes[correctedId].inputs[index].value.mCopyValueFactory =
						// pin.value.mCopyValueFactory;

						// pin.NewValue();
						// pin.value.SetValuePtr(data->mNodes[correctedId].inputs[index].value.CopyValue(), false);
						CopyDeSerializedInput(data->mNodes[correctedId].inputs[index], pin);
						index++;
					}
				}

				SpawnLockedNodes();
				// Application::Get()->mEditor->mGui.mRenderGraphEditor->LoadRenderGraphNodes(graph);
			}

			for (const auto& [key, value] : mTemplateNodes) {
				if (ImGui::Button(("Add: " + value.name).c_str())) {
					this->SpawnNode(value.name);
				}
			}
			for (const auto& [key, node] : mNodesData.mNodes) {
				if (ImGui::Button(node.name.c_str()))
					ax::NodeEditor::SelectNode(node.id);
				if (ImGui::Button("Navigate to Content"))
					ax::NodeEditor::NavigateToContent();
				if (ImGui::Button("Navigate to Selected"))
					ax::NodeEditor::NavigateToSelection(true);
			}

			ImGui::EndChild();
			ImGui::SameLine();
			ax::NodeEditor::SetCurrentEditor(mContext);
			ax::NodeEditor::Begin("Node Editor", ImVec2(0.0, 0.0f));
			uniqueId = 1;
			// Start drawing nodes.

			for (auto& [key, node] : mNodesData.mNodes) {
				this->InspectNode(node, false);
			}

			static ax::NodeEditor::PinId draggedPinId;
			// draggedPinId = {};
			if (ax::NodeEditor::BeginCreate()) {
				ax::NodeEditor::PinId inputPinId, outputPinId;
				if (ax::NodeEditor::QueryNewLink(&inputPinId, &outputPinId)) {
					Pin* inputPin =
						GetPin(inputPinId.Get()); // mNodesData.mPins.find(inputPinId.Get()) != mNodesData.mPins.end() ?
												  // mNodesData.mPins[inputPinId.Get()] : nullptr;
					Pin* outputPin =
						GetPin(outputPinId.Get()); // mNodesData.mPins.find(outputPinId.Get()) != mNodesData.mPins.end()
												   // ? mNodesData.mPins[outputPinId.Get()] : nullptr;
					// QueryNewLink returns true if editor want to create new link between pins.
					//
					// Link can be created only for two valid pins, it is up to you to
					// validate if connection make sense. Editor is happy to make any.
					//
					// Link always goes from input to output. User may choose to drag
					// link from output pin or input pin. This determine which pin ids
					// are valid and which are not:
					//   * input valid, output invalid - user started to drag new ling from input pin
					//   * input invalid, output valid - user started to drag new ling from output pin
					//   * input valid, output valid   - user dragged link over other pin, can be validated

					if (inputPinId && !outputPinId || inputPinId.Get() == outputPinId.Get()) {
						// Dragging from input pin
						draggedPinId = inputPinId; // Track the pin being dragged
					}
					else if (outputPinId && !inputPinId) {
						// Dragging from output pin
						draggedPinId = outputPinId; // Track the pin being dragged
					}

					if (inputPinId && outputPinId) { // both are valid, let's accept link
						// ed::AcceptNewItem() return true when user release mouse button.
						if (ax::NodeEditor::AcceptNewItem()) {
							// Since we accepted new link, lets add one to our list of links.
							mNodesData.mLinks.push_back(
								{ax::NodeEditor::LinkId(mNextLinkId++), inputPinId, outputPinId});

							outputPin->nodes.push_back(inputPin->nodes[0]);

							// Draw new link.
							ax::NodeEditor::Link(mNodesData.mLinks.back().id, mNodesData.mLinks.back().startPinID,
												 mNodesData.mLinks.back().endPinID);
						}

						// You may choose to reject connection between these nodes
						// by calling ed::RejectNewItem(). This will allow editor to give
						// visual feedback by changing link thickness and color.
					}
					else if (outputPinId && !inputPinId) {
					}
				}
				else if (ax::NodeEditor::AcceptNewItem()) {
					// Dropped link on an empty space
					Pin* draggedPin = mNodesData.mPins.find(draggedPinId.Get()) != mNodesData.mPins.end()
										  ? mNodesData.mPins[draggedPinId.Get()]
										  : nullptr;
					if (draggedPin && draggedPin->name != "Out") {
						Node& newNode = this->SpawnNode(draggedPin->value.type().name());
						mNodesData.mLinks.push_back(
							{ax::NodeEditor::LinkId(mNextLinkId++), newNode.outputs[0].id, draggedPin->id});
						draggedPin->nodes.push_back(newNode.outputs[0].nodes[0]);
						ax::NodeEditor::Link(mNodesData.mLinks.back().id, mNodesData.mLinks.back().startPinID,
											 mNodesData.mLinks.back().endPinID);
					}
				}
			}
			ax::NodeEditor::EndCreate();

			for (const auto& link : mNodesData.mLinks) {
				ax::NodeEditor::Link(link.id, link.startPinID, link.endPinID);
			}

			// Record popups
			ax::NodeEditor::Suspend();
			if (sOpenNodeEditorPopup) {
				ImGui::OpenPopup("NodeEditorEnumPopup");
				sOpenNodeEditorPopup = false;
			}
			if (ImGui::BeginPopup("NodeEditorEnumPopup")) {
				const char* typeRawName = sEnumToShowOnPopup->type().name();
				const std::vector<const char*>& enumNames = EnumReflection::GetEnumNames(typeRawName);
				int index = 0;
				for (const char* name : enumNames) {
					bool selected = EnumReflection::GetEnumName(typeRawName, index);
					bool clicked = ImGui::Selectable(name, &selected);

					sEnumToShowOnPopup->SetValue<int>(index, false);
					if (clicked) {
						sEnumToShowOnPopup = nullptr;
						break;
					};
					index++;
				}

				// if (sEnumToShowOnPopup)
				//	PrimitivesInspector::InspectAny(*sEnumToShowOnPopup);

				// for (int i = 0; i < items.size(); ++i)
				//{
				//	bool is_selected = (current_item == i);
				//	if (ImGui::Selectable(items[i].c_str(), is_selected))
				//	{
				//		current_item = i; // Update the selected item
				//		ImGui::CloseCurrentPopup();
				//	}
				//
				//	if (is_selected)
				//		ImGui::SetItemDefaultFocus(); // Focus on the currently selected item
				// }
				ImGui::EndPopup();
			}
			else
				sEnumToShowOnPopup = nullptr;
			ax::NodeEditor::Resume();

			ax::NodeEditor::End();

			ax::NodeEditor::SetCurrentEditor(nullptr);
			// if (ImGui::BeginPopup("EnumPopup"))
			//{
			//	for (int i = 0; i < IM_ARRAYSIZE(formats); ++i) {
			//		bool isSelected = (currentFormat == formats[i]);
			//		if (ImGui::Selectable(formats[i], isSelected)) {
			//			selectedFormat = static_cast<VkFormat>(i);  // Set selected enum value
			//		}
			//	}
			//
			//	ImGui::EndPopup();
			// }
		}

		ImGui::End();
	};

	void NodeEditor::OnKeyPress(int key, int scancode, int action, int mods) {

	};

	void NodeEditor::InspectNode(Node& node, bool inspectAsSubNode) {
		if (!inspectAsSubNode)
			ax::NodeEditor::BeginNode(node.id);
		ImGui::Text(node.name.c_str());
		for (auto& input : node.inputs) {
			cursorPos = ImGui::GetWindowPos();
			if (input.type != PinType::Object || input.isVector) {
				const char* rawname = input.value.type().name();
				const char* name = input.value.type().name();
				ImGui::PushID(uniqueId++);
				if (input.isVector) {
					ImGui::Text(input.name.c_str());
					ImGui::SameLine();
					if (ImGui::Button("+")) {
						// input.value.GetValue<std::vector<std::any>>()->push_back({});
						this->SpawnNode(input.value.type().name(), &input, false);
					}
					for (Node& subNode : input.subNodes) {
						this->InspectNode(subNode, true);
					}
				}
				else {
					ImGui::PushID(uniqueId++);
					PrimitivesInspector::InspectAny(input.value, input.name, "NodeEditorEnumPopup");
					ImGui::PopID();
				}
				ImGui::PopID();
			}
			else {
				ImGui::PushID(uniqueId++);
				ax::NodeEditor::BeginPin(input.id, ax::NodeEditor::PinKind::Input);
				ImGui::Text(input.name.c_str());
				ax::NodeEditor::EndPin();
				ImGui::PopID();
			}
		}

		if (!inspectAsSubNode) {
			for (const auto& output : node.outputs) {
				ImGui::PushID(uniqueId++);
				ax::NodeEditor::BeginPin(output.id, ax::NodeEditor::PinKind::Input);
				ImGui::Text(output.name.c_str());
				ax::NodeEditor::EndPin();
				ImGui::PopID();
			}
			ax::NodeEditor::EndNode();
		}
	}

	NodeEditor::Node& NodeEditor::SpawnNode(const std::string& nodeName, Pin* pinHolder, bool useDefaultValue) {
		Node nodeToCopy;
		auto it = mTemplateNodes.find(nodeName);
		if (it != mTemplateNodes.end())
			nodeToCopy = mTemplateNodes.at(nodeName);
		else
			nodeToCopy = mTemplateNodes.at("Empty Node");
		uintptr_t newId = GetNextId();
		Node* nodePtr = nullptr;
		if (pinHolder) {
			pinHolder->subNodes.push_back(nodeToCopy);
			nodePtr = &pinHolder->subNodes.back();
		}
		else {
			mNodesData.mNodes.emplace(newId, nodeToCopy);
			nodePtr = &mNodesData.mNodes.at(newId);
		}
		Node& newNode = *nodePtr; // mNodes.at(newId);
		newNode.id = newId;
		newNode.inputs.clear();
		newNode.outputs.clear();
		newNode.subNodes.clear();
		for (auto& input : nodeToCopy.inputs) {
			Pin newInput = input;
			const std::type_info& oldInfo = newInput.value.type();
			newInput.NewValue();
			newInput.value.SetType(oldInfo);
			newInput.id = GetNextId();
			newInput.nodes.clear();
			// newInput.value = Any();
			if (useDefaultValue)
				newInput.NewValue();
			this->AddInputPin(newNode, newInput);
		}
		for (const auto& output : nodeToCopy.outputs) {
			Pin newOutput = output;
			newOutput.id = GetNextId();
			newOutput.nodes.clear();
			if (useDefaultValue)
				newOutput.NewValue();
			this->AddOutputPin(newNode, newOutput);
		}
		for (const auto& subNode : nodeToCopy.subNodes) {
			newNode.subNodes.push_back(this->SpawnNode(subNode.name, nullptr, useDefaultValue));
		}
		return newNode;
	}

	void NodeEditor::AddNodeToCreate(const Node& newNode) {
		mTemplateNodes.emplace(newNode.name, newNode);
		// mNodes.emplace_back(GetNextId(), "Branch");
		// mNodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
		// mNodes.back().Inputs.emplace_back(GetNextId(), "Condition", PinType::Bool);
		// mNodes.back().Outputs.emplace_back(GetNextId(), "True", PinType::Flow);
		// mNodes.back().Outputs.emplace_back(GetNextId(), "False", PinType::Flow);
	}

	NodeEditor::PinType NodeEditor::ChoosePinType(const std::type_info& info) {
		if (info == typeid(float))
			return PinType::Float;
		else if (info == typeid(int))
			return PinType::Int;
		else if (info == typeid(unsigned int))
			return PinType::Int;
		else if (info == typeid(int8_t))
			return PinType::Int;
		else if (info == typeid(int16_t))
			return PinType::Int;
		else if (info == typeid(int32_t))
			return PinType::Int;
		else if (info == typeid(int64_t))
			return PinType::Int;
		else if (info == typeid(uint8_t))
			return PinType::Int;
		else if (info == typeid(uint16_t))
			return PinType::Int;
		else if (info == typeid(uint32_t))
			return PinType::Int;
		else if (info == typeid(uint64_t))
			return PinType::Int;
		else if (info == typeid(std::string))
			return PinType::String;
		else if (info == typeid(bool))
			return PinType::BoolType;
		else if (info == typeid(glm::vec2))
			return PinType::Vector2;
		else if (info == typeid(glm::vec3))
			return PinType::Vector3;
		else if (info == typeid(glm::vec4))
			return PinType::Vector4;
		else if (EnumReflection::HasTypeRawName(info.name()))
			return PinType::Enum;
		else if (std::string(info.name()).starts_with("class"))
			return PinType::Object;
		else if (std::string(info.name()).starts_with("struct"))
			return PinType::Object;
		else if (std::string(info.name()).starts_with("enum"))
			return PinType::Enum;
		else {
			std::cerr << "Unknown type: " << info.name() << std::endl;
			return PinType::Unknown;
		}
	}

	std::vector<uintptr_t> NodeEditor::GetOrderedNodes() {
		std::vector<uintptr_t> order = std::vector<uintptr_t>();
		for (auto& [key, value] : mNodesData.mNodes) {
			value.mOrderIndex = order.size();
			order.push_back(key);
		}

		unsigned int index = 0;
		for (const auto& [key, value] : mNodesData.mNodes) {
			std::vector<uintptr_t> dependencies = std::vector<uintptr_t>();
			for (const Pin& pin : value.inputs) {
				if (pin.nodes.size() > 1) {
					dependencies.push_back(pin.nodes[1]->id);
				}
			}
			unsigned int dependencyIndex = index + 1;
			for (uintptr_t dependency : dependencies) {
				if (!GetNode(dependency))
					continue;
				int from = GetNode(dependency)->mOrderIndex;
				int to = dependencyIndex;
				order.insert(order.begin() + to, order[GetNode(dependency)->mOrderIndex]);
				GetNode(dependency)->mOrderIndex = dependencyIndex;
				dependencyIndex++;
			}
			index++;
		}
		return order;
	}

	bool NodeEditor::BeginNodeCombo(const char* label, const char* preview_value, ImGuiComboFlags flags) {
		using namespace ImGui;

		ImGuiContext& g = *GImGui;
		bool has_window_size_constraint = (g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasSizeConstraint) != 0;
		g.NextWindowData.Flags &= ~ImGuiNextWindowDataFlags_HasSizeConstraint;

		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		IM_ASSERT((flags & (ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_NoPreview)) !=
				  (ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_NoPreview));

		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);

		const float arrow_size = (flags & ImGuiComboFlags_NoArrowButton) ? 0.0f : GetFrameHeight();
		const ImVec2 label_size = CalcTextSize(label, NULL, true);
		const float expected_w = CalcItemWidth();
		const float w = (flags & ImGuiComboFlags_NoPreview) ? arrow_size : expected_w;
		const ImRect frame_bb(window->DC.CursorPos,
							  window->DC.CursorPos + ImVec2(w, label_size.y + style.FramePadding.y * 2.0f));
		const ImRect total_bb(frame_bb.Min,
							  frame_bb.Max +
								  ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));
		ItemSize(total_bb, style.FramePadding.y);
		if (!ItemAdd(total_bb, id, &frame_bb))
			return false;

		bool hovered, held;
		bool pressed = ButtonBehavior(frame_bb, id, &hovered, &held);
		bool popup_open = IsPopupOpen(id, ImGuiPopupFlags_None);

		const ImU32 frame_col = GetColorU32(hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
		const float value_x2 = ImMax(frame_bb.Min.x, frame_bb.Max.x - arrow_size);
		RenderNavHighlight(frame_bb, id);
		if (!(flags & ImGuiComboFlags_NoPreview))
			window->DrawList->AddRectFilled(
				frame_bb.Min, ImVec2(value_x2, frame_bb.Max.y), frame_col, style.FrameRounding,
				(flags & ImGuiComboFlags_NoArrowButton) ? ImDrawCornerFlags_All : ImDrawCornerFlags_Left);
		if (!(flags & ImGuiComboFlags_NoArrowButton)) {
			ImU32 bg_col = GetColorU32((popup_open || hovered) ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
			ImU32 text_col = GetColorU32(ImGuiCol_Text);
			window->DrawList->AddRectFilled(ImVec2(value_x2, frame_bb.Min.y), frame_bb.Max, bg_col, style.FrameRounding,
											(w <= arrow_size) ? ImDrawCornerFlags_All : ImDrawCornerFlags_Right);
			if (value_x2 + arrow_size - style.FramePadding.x <= frame_bb.Max.x)
				RenderArrow(window->DrawList,
							ImVec2(value_x2 + style.FramePadding.y, frame_bb.Min.y + style.FramePadding.y), text_col,
							ImGuiDir_Down, 1.0f);
		}
		RenderFrameBorder(frame_bb.Min, frame_bb.Max, style.FrameRounding);
		if (preview_value != NULL && !(flags & ImGuiComboFlags_NoPreview))
			RenderTextClipped(frame_bb.Min + style.FramePadding, ImVec2(value_x2, frame_bb.Max.y), preview_value, NULL,
							  NULL, ImVec2(0.0f, 0.0f));
		if (label_size.x > 0)
			RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

		if ((pressed || g.NavActivateId == id) && !popup_open) {
			if (window->DC.NavLayerCurrent == 0)
				window->NavLastIds[0] = id;
			OpenPopupEx(id, ImGuiPopupFlags_None);
			popup_open = true;
		}

		if (!popup_open)
			return false;

		if (has_window_size_constraint) {
			g.NextWindowData.Flags |= ImGuiNextWindowDataFlags_HasSizeConstraint;
			g.NextWindowData.SizeConstraintRect.Min.x = ImMax(g.NextWindowData.SizeConstraintRect.Min.x, w);
		}
		else {
			if ((flags & ImGuiComboFlags_HeightMask_) == 0)
				flags |= ImGuiComboFlags_HeightRegular;
			IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiComboFlags_HeightMask_)); // Only one
			int popup_max_height_in_items = -1;
			if (flags & ImGuiComboFlags_HeightRegular)
				popup_max_height_in_items = 8;
			else if (flags & ImGuiComboFlags_HeightSmall)
				popup_max_height_in_items = 4;
			else if (flags & ImGuiComboFlags_HeightLarge)
				popup_max_height_in_items = 20;
			SetNextWindowSizeConstraints(ImVec2(w, 0.0f),
										 ImVec2(FLT_MAX, CalcMaxPopupHeightFromItemCount(popup_max_height_in_items)));
		}

		char name[16];
		ImFormatString(name, IM_ARRAYSIZE(name), "##Combo_%02d", g.BeginPopupStack.Size);

		// Set default position without using CalcWindowExpectedSize
		ImVec2 size_expected(w, CalcMaxPopupHeightFromItemCount(8)); // Estimate expected size
		ImVec2 pos = frame_bb.GetBL();
		if (flags & ImGuiComboFlags_PopupAlignLeft)
			pos.x -= size_expected.x - w;

		SetNextWindowPos(ax::NodeEditor::CanvasToScreen(pos));
		pos = ax::NodeEditor::CanvasToScreen(pos);

		// Move ed::Suspend and ed::Resume here
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_Popup |
										ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
										ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;

		ax::NodeEditor::Suspend();
		PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(style.FramePadding.x, style.WindowPadding.y));

		bool ret = Begin(name, NULL, window_flags);

		PopStyleVar();

		if (!ret) {
			EndPopup();
			IM_ASSERT(0);
			return false;
		}
		return true;
	}

	void NodeEditor::EndNodeCombo() {
		ImGui::EndPopup();

		ax::NodeEditor::Resume();
		// ImGui::SetWindowPos(ImVec2(500.0f, 200.0f));
	}
} // namespace Plaza::Editor
