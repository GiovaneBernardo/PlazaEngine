#pragma once
#include "Editor/GUI/GuiWindow.h"
#include "ThirdParty/imgui/nodes/imgui_node_editor.h"
#include "ThirdParty/imgui/imgui_internal.h"
#include <any>
#include "ThirdParty/magic_enum/magic_enum.hpp"
#include "Engine/Core/EnumReflection.h"
#include "Engine/Core/Any.h"
#include <ThirdParty/pfr/include/boost/pfr/core.hpp>
#include <ThirdParty/pfr/include/boost/pfr/core_name.hpp>
#include <inttypes.h>

// namespace ed = ax::NodeEditor;
namespace Plaza::Editor {
	class NodeEditor : public GuiWindow {
	  public:
		class Node;
		class Pin;
		class Link;

		ax::NodeEditor::EditorContext* mContext = nullptr;

		NodeEditor(const std::string& name, GuiLayer layer, bool startOpen = true) : GuiWindow(layer, startOpen) {}

		void InitNodeEditor();
		// void Init() override {};
		void Update(Scene* scene) override;
		void OnKeyPress(int key, int scancode, int action, int mods) override;

		void InitMathNodes();
		void InspectNode(Node& node, bool inspectAsSubNode);
		void CopyDeSerializedInput(Pin& from, Pin& to);

		struct NodesData : public Asset {
			std::map<uintptr_t, Node> mNodes = std::map<uintptr_t, Node>();
			std::map<std::string, Node*> mNodesByName = std::map<std::string, Node*>();
			std::vector<Link> mLinks;
			std::map<uintptr_t, Pin*> mPins = std::map<uintptr_t, Pin*>();

			template <class Archive> void serialize(Archive& archive) { archive(PL_SER(mNodes), PL_SER(mLinks)); }
		} mNodesData;

		Node* GetNode(uintptr_t id) {
			if (mNodesData.mNodes.find(id) != mNodesData.mNodes.end())
				return &mNodesData.mNodes.at(id);
			return nullptr;
		}
		Pin* GetPin(uintptr_t id) {
			if (mNodesData.mPins.find(id) != mNodesData.mPins.end())
				return mNodesData.mPins.at(id);
			return nullptr;
		}

		std::string mName = "";
		int mNextLinkId = 1;
		const int mPinIconSize = 24;
		std::map<std::string, Node> mTemplateNodes;
		const float mTouchTime = 1.0f;
		static inline Any* sEnumToShowOnPopup = nullptr;
		static inline bool sOpenNodeEditorPopup = false;
		uintptr_t mFinalNodeId = 1;
		uintptr_t mNextId = 1;
		const uintptr_t& GetNextId() {
			while (GetNode(mNextId++) != nullptr) {
				mNextId++;
			}
			return mNextId;
		}

		Pin& AddInputPin(Node& node, Pin pin) {
			pin.id = GetNextId();
			pin.nodes.push_back(&node);
			node.inputs.emplace_back(pin);
			mNodesData.mPins.emplace(pin.id.Get(), &node.inputs.back());
			return node.inputs.back();
		}
		Pin& AddOutputPin(Node& node, Pin pin) {
			pin.id = GetNextId();
			mNextId++;
			pin.nodes.push_back(&node);
			node.outputs.emplace_back(pin);
			mNodesData.mPins.emplace(pin.id.Get(), &node.outputs.back());
			return node.outputs.back();
		}

		std::vector<uintptr_t> GetOrderedNodes();

		template <typename T> void SetFinalNode() {
			Node finalNode = Node();
			finalNode.name = "Final Node";
			this->AddInputPin(finalNode, Pin(0, "In", PinType::Object));
			this->AddOutputPin(finalNode, Pin(0, "Out", PinType::Object));
			finalNode.processFunction = [](Node& node) {
				// Node& previousNode = *node.inputs[0].nodes[1];
				// node.outputs[0].value =
				// previousNode.outputs[0].value;//.SetValue(previousNode.outputs[0].GetValue<void*>(), false);
				T obj{};
				// boost::pfr::for_each_field(obj, [&node, &obj](auto& field, auto i) {
				//	using FieldType = std::decay_t<decltype(field)>;
				//	field = node.inputs[i].GetValue<FieldType>();
				//	});
				if (node.inputs[0].nodes.size() > 1) {
					T* ptr = node.inputs[0].nodes[1]->outputs[0].value.GetValue<T>();
					obj = *node.inputs[0].nodes[1]->outputs[0].value.GetValue<T>(); // node.inputs[0].GetValue<T>();
					node.outputs[0].SetValue<T>(obj);
				}
				else {
					node.outputs[0].SetValue<T>(T());
				}
			};
			this->AddNodeToCreate(finalNode);

			finalNode = this->SpawnNode("Final Node");
			mFinalNodeId = finalNode.id;
		}

	  private:
		int uniqueId = 1;

	  public:
		enum class PinType {
			Unknown,
			Flow,
			BoolType,
			Int,
			Float,
			String,
			Object,
			Function,
			Delegate,
			Enum,
			Vector2,
			Vector3,
			Vector4,
			Array
		};

		enum class PinKind {
			Output,
			Input,
			Constant
		};

		enum class NodeType {
			Blueprint,
			Simple,
			Tree,
			Comment,
			Houdini
		};

		Node& SpawnNode(const std::string& nodeName, Pin* pinHolder = nullptr, bool useDefaultValue = true);
		void RemoveNode(int index);

		virtual void SpawnLockedNodes() {};
		virtual void Process();

		void AddNodeToCreate(const Node& newNode);

		template <typename T> struct is_specialization_of_vector : std::false_type {};

		template <typename T, typename Alloc>
		struct is_specialization_of_vector<std::vector<T, Alloc>> : std::true_type {};

		template <typename T> struct is_specialization_of_array : std::false_type {};

		// Specialization for std::array
		template <typename T, std::size_t N> struct is_specialization_of_array<std::array<T, N>> : std::true_type {};

		template <typename T> Node& AddStructNode() {
			T obj = T();
			Node node = Node();
			node.name = typeid(T).name();
			boost::pfr::for_each_field(obj, [this, &node](const auto f, auto i) {
				// std::cout << boost::pfr::get_name<i, T>() << ": " << f << '\n';
				std::string_view fieldName = boost::pfr::get_name<i, T>();
				using FieldType = std::decay_t<decltype(f)>;
				bool isStruct = false;
				if constexpr (is_specialization_of_vector<FieldType>::value) {
					using ContentType = typename FieldType::value_type;
					if (NodeEditor::ChoosePinType(typeid(ContentType)) == PinType::Unknown)
						isStruct = true;
				}
				else {
					if (NodeEditor::ChoosePinType(typeid(FieldType)) == PinType::Unknown)
						isStruct = true;
				}

				if (isStruct) {
					if constexpr (is_specialization_of_vector<FieldType>::value) {
						// Is a vector
						using ContentType = typename FieldType::value_type;
						Any::RegisterType<ContentType>();
						node.subNodes.push_back(this->SpawnNode(typeid(ContentType).name()));
					}
					else
						node.subNodes.push_back(this->SpawnNode(typeid(FieldType).name()));
				}
				else {
					this->AddInputPin(node, Pin(i, fieldName.data(), NodeEditor::ChoosePinType(typeid(FieldType))));
					Any::RegisterType<FieldType>();
					if constexpr (std::is_enum_v<FieldType>) {
						node.inputs.back().SetEnumValue<FieldType>(0);
						node.inputs.back().NewValue();
					}
					else {
						node.inputs.back().SetValue<FieldType>({});
						node.inputs.back().NewValue();
					}

					if constexpr (is_specialization_of_vector<FieldType>::value) {
						using ContentType = typename FieldType::value_type;
						node.inputs.back().value.SetType<ContentType>(); // SetValue<ContentType>({});
						Any::RegisterType<ContentType>();
						node.inputs.back().isVector = true;
					}
				}
			});
			AddOutputPin(node, Pin(0, "Out", NodeEditor::PinType::Object, PinKind::Output));
			node.processFunction = [](Node& node) {
				for (Pin& input : node.inputs) {
					for (Node& subNode : input.subNodes) {
						subNode.processFunction(subNode);
					}
				}

				T obj{};
				boost::pfr::for_each_field(obj, [&node](auto& field, auto i) {
					using FieldType = std::decay_t<decltype(field)>;
					if constexpr (is_specialization_of_vector<FieldType>::value) {
						using ContentType = typename FieldType::value_type;

						std::vector<ContentType> fieldObj =
							std::vector<ContentType>(); // Prepare a vector to store subNode outputs.
						for (Node& subNode : node.inputs[i].subNodes) {
							if (subNode.outputs[0].value.mValue != nullptr)
								fieldObj.push_back(subNode.outputs[0].GetValue<ContentType>());
						}

						// Assign the fieldObj directly to field if FieldType is the expected vector type.
						field = fieldObj;
					}
					else if (node.inputs[i].nodes.size() > 1) {
						field = node.inputs[i].nodes[1]->outputs[0].template GetValue<FieldType>();
					}
					else
						field = node.inputs[i].template GetValue<FieldType>();
				});

				/*
							node.processFunction = [](Node& node) {
				T obj{};
				boost::pfr::for_each_field(obj, [&node](auto& field, auto i) {
					using FieldType = std::decay_t<decltype(field)>;
					field = node.inputs[i].GetValue<FieldType>();
					if (node.inputs[i].nodes.size() > 1) {
						field = node.inputs[i].nodes[1]->outputs[0].GetValue<FieldType>();
					}
					});
				*/
				node.outputs[0].SetValue<T>(obj);
			};
			AddNodeToCreate(node);
			return node;
		}

		template <typename T> Node& AddNodeToCreateByType(const std::string& categoryName) {
			std::string typeName = std::string(typeid(T).name());
			if (typeName.starts_with("struct")) {
				return AddStructNode<T>();
			}
			else if (typeName.starts_with("enum")) {
				Node node = Node();
				node.name = typeName;
				AddInputPin(node, Pin(0, typeName.c_str(), NodeEditor::PinType::Enum, PinKind::Constant));
				node.inputs[0].value.SetValue(T());
				AddOutputPin(node, Pin(0, "Out", NodeEditor::PinType::Enum, PinKind::Output));
				node.processFunction = [](Node& node) {
					node.outputs[0].SetValue<T>(node.inputs[0].GetValue<T>(), false);
				};
				AddNodeToCreate(node);
				return node;
			}
		}

		static NodeEditor::PinType ChoosePinType(const std::type_info& info);

	  public:
		// struct NodeIdLess
		//{
		//	bool operator()(const ax::NodeEditor::NodeId& lhs, const ax::NodeEditor::NodeId& rhs) const
		//	{
		//		return lhs.AsPointer() < rhs.AsPointer();
		//	}
		// };
		class Pin {
		  public:
			ax::NodeEditor::PinId id;
			std::vector<Node*> nodes;
			std::vector<Node> subNodes;
			std::string name;
			NodeEditor::PinType type;
			PinKind kind;
			// std::any value;
			Any value;
			int enumSize = 0;
			int enumIndex = 0;
			bool isVector = false;

			Pin(){};
			Pin(int newId, const char* newName, NodeEditor::PinType newType, PinKind newKind = PinKind::Input)
				: id(newId), nodes(std::vector<Node*>()), name(newName), type(newType), kind(newKind) {}

			template <typename T> T GetValue() { return *value.GetValue<T>(); }

			template <typename T> void SetValue(const T& newValue, bool changeType = true) {
				if (changeType) {
					value.SetValue<T>(newValue, true);
					mFactory = [newValue](Any& any) -> void* {
						T* val = new T();
						*val = newValue;
						any.SetValue<T>(newValue, true);
						return val;
					};
				}
				else
					value.SetValue(newValue, false);
				Any::RegisterType<T>();
			}

			void NewValue() {
				if (mFactory)
					value.mValue = mFactory(this->value);
				// value.SetValue(mFactory());
			}

			template <typename T> std::enable_if_t<std::is_enum_v<T>, void> SetEnumValue(int newValue) {
				static_assert(std::is_enum<T>::value, "T must be an enum type.");
				value = static_cast<T>(newValue);
				enumIndex = newValue;

				enumSize = magic_enum::enum_count<T>();

				std::vector<const char*> names = std::vector<const char*>();
				for (int i = 0; i < enumSize; ++i) {
					names.push_back(magic_enum::enum_name(T(i)).data());
				}
				if (names.size() > 0 && (names[0] == nullptr || (names.size() > 1 && names[1] == nullptr)))
					EnumReflection::RegisterBitmaskEnum<T>();
				else
					EnumReflection::RegisterEnum<T>();
			}

			void SetEnumIndex(int newValue) {
				value.SetValue(newValue, false);
				enumIndex = newValue;
			}

			template <class Archive> void serialize(Archive& archive) {
				uintptr_t newId = id.Get();
				if (isVector)
					value.SetValuePtr<int>(nullptr, false);
				if (name == "Pipelines") {
					std::cout << "here \n";
				}
				archive(PL_SER(newId), PL_SER(subNodes), PL_SER(name), PL_SER(type), PL_SER(kind), PL_SER(enumSize),
						PL_SER(enumIndex), PL_SER(isVector));
				if (name == "Pipelines") {
					std::cout << "here \n";
				}

				if (!isVector && kind != PinKind::Output)
					archive(PL_SER(value));
			}

		  private:
			std::function<void*(Any& any)> mFactory;
		};
		class Node {
		  public:
			int id;
			std::string name;
			std::vector<Pin> inputs;
			std::vector<Pin> outputs;
			std::vector<Node> subNodes;
			ImColor color = ImColor(1.0f, 1.0f, 1.0f, 1.0f);
			NodeType type;
			ImVec2 size = ImVec2(0, 0);
			std::function<void(Node& node)> processFunction;
			unsigned int mOrderIndex = 0;
			void Process() { processFunction(*this); }

			template <class Archive> void serialize(Archive& archive) {
				archive(PL_SER(id), PL_SER(name), PL_SER(inputs), PL_SER(outputs), PL_SER(subNodes), PL_SER(type));
			}
		};
		struct Link {
			ax::NodeEditor::LinkId id;

			ax::NodeEditor::PinId startPinID;
			ax::NodeEditor::PinId endPinID;

			ImColor color = ImColor(1.0f, 1.0f, 1.0f, 1.0f);
			;

			Link(){};
			Link(ax::NodeEditor::LinkId id, ax::NodeEditor::PinId startPinId, ax::NodeEditor::PinId endPinId)
				: id(id), startPinID(startPinId), endPinID(endPinId), color(255, 255, 255) {}

			template <class Archive> void serialize(Archive& archive) {
				uintptr_t newId = id.Get();
				uintptr_t newStartPinId = startPinID.Get();
				uintptr_t newEndPinId = endPinID.Get();
				archive(PL_SER(newId), PL_SER(newStartPinId), PL_SER(newEndPinId));
				id = newId;
				startPinID = newStartPinId;
				endPinID = newEndPinId;
			}
		};

	  private:
		ImVec2 cursorPos;
		static float CalcMaxPopupHeightFromItemCount(int items_count) {
			ImGuiContext& g = *GImGui;
			if (items_count <= 0)
				return FLT_MAX;
			return (g.FontSize + g.Style.ItemSpacing.y) * items_count - g.Style.ItemSpacing.y +
				   (g.Style.WindowPadding.y * 2);
		}
		bool BeginNodeCombo(const char* label, const char* preview_value, ImGuiComboFlags flags);

		void EndNodeCombo();
	};
} // namespace Plaza::Editor
