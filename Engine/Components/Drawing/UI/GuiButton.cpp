#include "Engine/Core/PreCompiledHeaders.h"
#include "GuiButton.h"
#include "Engine/Core/Scene.h"

namespace Plaza {
	bool GuiButton::MouseIsInsideButton(glm::vec2 mousePos) {
#ifdef EDITOR_MODE
		mousePos.x -= Application::Get()->appSizes->hierarchySize.x;
		mousePos.y -= Application::Get()->appSizes->sceneImageStart.y + 35;
#endif
		glm::vec2 worldPosition = this->GetLocalPosition();
		glm::vec2 size = this->GetLocalSize() / glm::vec2(2.0f, 1.0f);

		bool insideX = mousePos.x >= worldPosition.x - size.x && mousePos.x <= worldPosition.x + size.x;
		bool insideY = mousePos.y >= worldPosition.y && mousePos.y <= worldPosition.y + size.y;

		if (insideX && insideY)
			return true;
		else
			return false;
	}
}