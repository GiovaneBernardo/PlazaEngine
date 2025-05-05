#pragma once
#include <functional>
#include "Engine/Core/Scene.h"

namespace Plaza {
	class PLAZA_API ECSManager {
	  public:
	};

	class ComponentPool;
	class PLAZA_API ECS {
	  public:
		static inline void RegisterComponents() {
			Application::Get()->mComponentCounter = 0;
			Scene::GetComponentId<TransformComponent>();
			ECS::RegisterComponent<TransformComponent>();
			Scene::GetComponentId<MeshRenderer>();
			ECS::RegisterComponent<MeshRenderer>();
			Scene::GetComponentId<Collider>();
			ECS::RegisterComponent<Collider>();
			Scene::GetComponentId<RigidBody>();
			ECS::RegisterComponent<RigidBody>();
			Scene::GetComponentId<Camera>();
			ECS::RegisterComponent<Camera>();
			Scene::GetComponentId<Light>();
			ECS::RegisterComponent<Light>();
			Scene::GetComponentId<AudioSource>();
			ECS::RegisterComponent<AudioSource>();
			Scene::GetComponentId<AudioListener>();
			ECS::RegisterComponent<AudioListener>();
			Scene::GetComponentId<CppScriptComponent>();
			ECS::RegisterComponent<CppScriptComponent>();
			Scene::GetComponentId<AnimationComponent>();
			ECS::RegisterComponent<AnimationComponent>();
		}

		static void InstantiateComponent(ComponentPool* srcPool, ComponentPool* dstPool, uint64_t srcUuid,
										 uint64_t dstUuid);
		static inline std::vector<
			std::function<void*(ComponentPool* srcPool, ComponentPool* dstPool, uint64_t srcUuid, uint64_t dstUuid)>>
			sInstantiateComponentFactory{};
		template <typename T> static inline void RegisterComponent() {
			const int componentId = Scene::GetComponentId<T>();
			if (componentId >= ECS::sInstantiateComponentFactory.size())
				ECS::sInstantiateComponentFactory.resize(componentId + 1);
			ECS::sInstantiateComponentFactory[componentId] = [](ComponentPool* srcPool, ComponentPool* dstPool,
																uint64_t srcUuid, uint64_t dstUuid) -> void* {
				T* component = dstPool->New<T>(dstUuid);
				*component = *static_cast<T*>(srcPool->Get(srcUuid));
				static_cast<Component*>(component)->mUuid = dstUuid;
				return component;
			};
		}

	  private:
	  public:
		class PLAZA_API EntitySystem {
		  public:
			static void SetParent(Scene* scene, Entity* child, Entity* newParent);
			static void Delete(Scene* scene, uint64_t uuid);
			static uint64_t Instantiate(Scene* scene, uint64_t uuidToInstantiate, bool newTransform = false);
			static uint64_t Instantiate(Scene* srcScene, Scene* dstScene, uint64_t srcUuid, uint64_t dstUuid,
										bool newTransform = false);
			static uint64_t Instantiate(const std::unordered_map<uint64_t, Entity>& srcEntities,
										const std::vector<ComponentPool*>& srcComponentPools, Scene* dstScene,
										uint64_t srcUuid, uint64_t dstUuid, bool newTransform = false);
		};
		class PLAZA_API TransformSystem {
		  public:
			static void OnInstantiate(Component* componentToInstantiate, TransformComponent& transform);
			static void UpdateTransform(TransformComponent& transform, Scene* scene);
			static void UpdateLocalMatrix(TransformComponent& transform, Scene* scene);
			static void UpdateWorldMatrix(TransformComponent& transform, const glm::mat4& parentWorldMatrix,
										  Scene* scene);
			static void UpdateChildrenTransform(TransformComponent& transform, Scene* scene);
			static void SetLocalPosition(TransformComponent& transform, Scene* scene, const glm::vec3& vector,
										 bool updateWorldMatrix = true);
			static void SetLocalRotation(TransformComponent& transform, Scene* scene, const glm::quat& quat,
										 bool updateWorldMatrix = true);
			static void SetLocalScale(TransformComponent& transform, Scene* scene, const glm::vec3& vector,
									  bool updateWorldMatrix = true);
			static void SetWorldPosition(TransformComponent& transform, Scene* scene, const glm::vec3& vector,
										 bool updateWorldMatrix = true);
			static void SetWorldRotation(TransformComponent& transform, Scene* scene, const glm::vec3& vector,
										 bool updateWorldMatrix = true);
			static void SetWorldScale(TransformComponent& transform, Scene* scene, const glm::vec3& vector,
									  bool updateWorldMatrix = true);
			static void UpdateSelfAndChildrenTransform(TransformComponent& transform,
													   TransformComponent* parentTransform, Scene* scene,
													   bool updateLocal = true, bool forceUpdateLocal = false);
		};
		class PLAZA_API ColliderSystem {
		  public:
			static void RemoveActor(Collider* collider);
			static void Init(RigidBody* rigidBody);
			static void InitDynamic(RigidBody* rigidBody = nullptr) {};
			static void InitStatic() {};
			static void InitCollider(Collider* collider, TransformComponent* transform, RigidBody* rigidBody);
			static void InitCollider(Scene* scene, uint64_t uuid);
			static void RemoveCollider();
			static void Update() {};

			static void CreateShape(Collider* collider, TransformComponent* transform,
									ColliderShape::ColliderShapeEnum shapeEnum, Mesh* mesh = nullptr);
			static void AddShape(ColliderShape* shape);
			static void AddConvexMeshShape(Mesh* mesh);
			static void AddMeshShape(Mesh* mesh);
			static void AddHeightShape(float** heightData, int size);
			static void UpdateShapeScale(Scene* scene, Collider* collider, const glm::vec3& scale);
			static void UpdateAllShapesScale(Scene* scene, uint64_t uuid);

			static void UpdatePose(Collider* collider, TransformComponent* transform);
		};

		class PLAZA_API RigidBodySystem {
		  public:
			static void Init(Scene* scene, uint64_t uuid);
			static void Update(Scene* scene, uint64_t uuid);
			static void UpdateGlobalPose(Scene* scene, uint64_t uuid);
			static void AddCollidersOfChildren(Scene* scene, uint64_t parent);
			static void UpdateRigidBody(Scene* scene, uint64_t uuid);
		};
	};
}; // namespace Plaza
