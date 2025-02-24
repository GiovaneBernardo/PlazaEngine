#pragma once

namespace Plaza::Editor {
	class PLAZA_API ComponentsInspector {
	public:
		static void AnimationComponentInspector(Scene* scene, Entity* entity);
		static void AudioListenerInspector(Scene* scene, Entity* entity);
		static void AudioSourceInspector(Scene* scene, Entity* entity);
		static void CameraInspector(Scene* scene, Entity* entity);
		static void ColliderInspector(Scene* scene, Entity* entity);
		static void CppScriptComponentInspector(Scene* scene, Entity* entity);
		static void GuiComponentInspector(Scene* scene, Entity* entity);
		static void LightInspector(Scene* scene, Entity* entity);
		static void MeshRendererInspector(Scene* scene, Entity* entity);
		static void RigidBodyInspector(Scene* scene, Entity* entity);
		static void SceneInspector(Scene* scene, Entity* entity);
		static void TextRendererInspector(Scene* scene, Entity* entity);
		static void TransformInspector(Scene* scene, Entity* entity);
	};
}