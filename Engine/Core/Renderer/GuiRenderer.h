#pragma once
#include "Engine/Components/Drawing/UI/TextRenderer.h"

namespace Plaza {
	class GuiRenderer {
	  public:
		virtual void Init() = 0;
		virtual void RenderText(Scene* scene, Drawing::UI::TextRenderer* textRendererComponent) = 0;
		virtual void Terminate() = 0;
	};
} // namespace Plaza
