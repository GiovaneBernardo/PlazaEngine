#pragma once
#include <ThirdParty/imgui/imgui.h>

#include "Engine/Components/Core/Entity.h"
#include "Editor/GUI/guiMain.h"
#include "Editor/GUI/Style/EditorStyle.h"
namespace Plaza::Editor {
	class HierarchyWindow : public GuiWindow {
	  public:
		static inline bool sHoveringItem = false;
		HierarchyWindow(GuiLayer layer, bool startOpen = true) : GuiWindow(layer, startOpen) {}
		Scene* mScene = nullptr;

		void Init() override;
		void Update(Scene* scene) override;

	  private:
		void OnKeyPress(int key, int scancode, int action, int mods) override;

	  public:
		class Item {
		  public:
			Entity& currentObj;
			Entity& selectedGameObject;
			static inline std::string payloadName = "TreeNodeItemPayload";
			static bool firstFocus;

			static void NewItem(Entity& entity, Entity*& selectedGameObject, Scene* scene);
			static void HierarchyDragDrop(Entity& entity, Entity* currentObj, ImVec2 treeNodeMin, ImVec2 treeNodeMax,
										  Scene* scene);

			static void ItemPopup(Entity* entity, Scene* scene);
		};
	};
	// class Gui::Hierarchy {
	// public:
	//	class Item { // Item consists of a Treenode and a selectable
	//	public:
	//		Entity& currentObj;
	//		Entity& selectedGameObject;
	//		std::string payloadName = "TreeNodeItemPayload";
	//		static bool firstFocus;
	//
	//		Item(Entity& entity, Entity*& selectedGameObject);
	//		void HierarchyDragDrop(Entity& entity, Entity* currentObj, ImVec2 treeNodeMin, ImVec2 treeNodeMax);
	//
	//		static void ItemPopup(Entity& entity);
	//	};
	// };
} // namespace Plaza::Editor
