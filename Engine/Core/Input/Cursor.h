#pragma once
#include "Engine/Core/Engine.h"

namespace Plaza {
	class PLAZA_API Input::Cursor {
	  public:
		static bool show;

		static double lastX;
		static double lastY;

		static double deltaX;
		static double deltaY;

		static void Update();

		static void SetX(float value);
		static void SetY(float value);
		static glm::vec2 GetMousePosition();
		static glm::vec2 GetDeltaMousePosition();
	};
} // namespace Plaza
