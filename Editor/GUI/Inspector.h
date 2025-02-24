#pragma once

namespace Plaza::Editor {
	class Inspector {
	public:
		class FileInspector {
		public:
			static void CreateInspector();
			static void CreateRespectiveInspector(File* file);
		};

		class ComponentInspector {
		public:
			static vector<Component*> components;

			static void CreateInspector(Scene* scene);
			static void UpdateComponents();

			static void CreateRespectiveInspector(Scene* scene, Component* component);
		};


	};
}