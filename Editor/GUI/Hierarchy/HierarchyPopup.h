#pragma once
namespace Plaza::Editor {
	class HierarchyPopup {
	  public:
		static bool hoveringNothing;
		static void Update(Scene* scene, Entity* entity);
		static void UpdateAddComponentPopup(Scene* scene, Entity* entity);
	};
} // namespace Plaza::Editor
