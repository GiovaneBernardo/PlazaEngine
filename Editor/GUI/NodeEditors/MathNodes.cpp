#include "NodeEditor.h"

namespace Plaza::Editor {
	void NodeEditor::InitMathNodes() {
		Node node = Node();
		node.name = "Multiply";
		this->AddInputPin(node, Pin(0, "X", PinType::Float));
		this->AddOutputPin(node, Pin(0, "Y", PinType::Float, PinKind::Output));
		node.processFunction = [](Node& node) {
			node.outputs[0].SetValue<float>(node.inputs[0].GetValue<float>() * 10.0f);
		};
		this->AddNodeToCreate(node);
	}
} // namespace Plaza::Editor
