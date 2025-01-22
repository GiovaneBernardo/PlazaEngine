#pragma once
#include "Engine/Core/Engine.h"
#include "Engine/Components/Physics/PhysicsMaterial.h"

namespace Plaza {
	class PhysicsMaterial;
	class PLAZA_API MyQueryFilterCallback : public physx::PxQueryFilterCallback {
	public:
		virtual physx::PxQueryHitType::Enum preFilter(
			const physx::PxFilterData& filterData,
			const physx::PxShape* shape,
			const physx::PxRigidActor* actor,
			physx::PxHitFlags& queryFlags) override
		{
			if (actor && actor->userData == mEntityToIgnore) {
				return physx::PxQueryHitType::eNONE;
			}
			return physx::PxQueryHitType::eBLOCK;
		}

		// Set the entity to ignore
		void setEntityToIgnore(void* entityToIgnore) {
			mEntityToIgnore = entityToIgnore;
		}

		physx::PxQueryHitType::Enum postFilter(const physx::PxFilterData& filterData, const physx::PxQueryHit& hit, const physx::PxShape* shape, const physx::PxRigidActor* actor) override {
			return physx::PxQueryHitType::eTOUCH;
		}

	private:
		void* mEntityToIgnore = nullptr;
	};

	struct PLAZA_API RaycastHit {
		uint64_t hitUuid;
		glm::vec3 point;
		glm::vec3 normal;
		bool missed = false;
	};

	class Mesh;
	class ColliderShape;
	class PLAZA_API Physics {
	public:
		static physx::PxDefaultAllocator m_defaultAllocatorCallback;
		static physx::PxDefaultErrorCallback m_defaultErrorCallback;
		static physx::PxDefaultCpuDispatcher* m_dispatcher;
		static physx::PxTolerancesScale m_toleranceScale;
		static physx::PxFoundation* m_foundation;
		static physx::PxPhysics* m_physics;

		static std::unordered_map<uint64_t, physx::PxGeometry*> sCookedGeometries;
		static std::unordered_map<uint64_t, std::unordered_map<PhysicsMaterial, physx::PxShape*>> sShapes;

		static physx::PxMaterial* defaultMaterial;

		static physx::PxMaterial* InitializePhysicsMaterial(float staticFriction, float dynamicFriction, float restitution);
		static PhysicsMaterial& GetDefaultPhysicsMaterial();

		static physx::PxScene* m_scene;
		static physx::PxMaterial* m_material;
		static physx::PxPvd* m_pvd;
		static bool m_canRun;

		static const float maxFrameAdvance;
		static float accumulatedTime;
		static float stepSize;

		static physx::PxSceneDesc GetSceneDesc();

		static bool Advance(float dt);
		static void Init();
		static void InitScene();
		static void InitPhysics();
		static void Update(Scene* scene);

		static physx::PxTransform GetPxTransform(TransformComponent& transform);
		static physx::PxTransform* ConvertMat4ToPxTransform(const glm::mat4& mat);

		static physx::PxShape* GetPhysXShape(ColliderShape* colliderShape, PhysicsMaterial* material);
		static void InitDefaultGeometries();
		static physx::PxGeometry* CookMeshGeometry(Mesh* mesh);
		static physx::PxGeometry* CookConvexMeshGeometry(Mesh* mesh);
		static physx::PxGeometry* GetGeometry(uint64_t uuid);
		static physx::PxGeometry* GetCubeGeometry();
		static physx::PxGeometry* GetPlaneGeometry();
		static physx::PxGeometry* GetSphereGeometry();
		static physx::PxGeometry* GetCapsuleGeometry();
		static void DeleteShape();

		static void Raycast(glm::vec3 origin, glm::vec3 direction, RaycastHit& hit);
	private:
		static physx::PxVec3 GlmToPhysX(const glm::vec3& vector);
	};
}