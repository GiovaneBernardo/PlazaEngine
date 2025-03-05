#pragma once

namespace Plaza {
	class DragBase {
	  public:
		static void Drag(auto& data);
	};

	class DropBase {
	  public:
		static void Drop(auto& data);
	};
} // namespace Plaza
