#pragma once
#include "ThirdParty/PhysX/physx/include/PxPhysicsAPI.h"
// #include <PhysX/physx/include/PxPhysicsAPI.h>
#include "Engine/Components/Core/Transform.h"
#include "Engine/Components/Physics/RigidBody.h"
#include "Engine/Components/Rendering/MeshRenderer.h"
#include "Engine/Core/Engine.h"

namespace Plaza {

	struct ColliderShape {
		enum ColliderShapeEnum {
			BOX,
			SPHERE,
			CAPSULE,
			PLANE,
			CYLINDER,
			MESH,
			CONVEX_MESH,
			HEIGHT_FIELD
		};

		ColliderShape(){};
		ColliderShape(physx::PxShape* shape, ColliderShapeEnum newEnum = ColliderShapeEnum::BOX, uint64_t meshUuid = 0)
			: mPxShape(shape), mEnum(newEnum), mMeshUuid(meshUuid) {}
		physx::PxShape* mPxShape = nullptr;
		ColliderShapeEnum mEnum = ColliderShapeEnum::BOX;
		uint64_t mMeshUuid = 0;

		template <class Archive> void serialize(Archive& archive) { archive(PL_SER(mEnum), PL_SER(mMeshUuid)); }
	};
	class PLAZA_API Collider : public Component {
	  public:
		glm::vec3 lastScale = glm::vec3(1.0f);
		std::vector<std::shared_ptr<ColliderShape>> mShapes = std::vector<std::shared_ptr<ColliderShape>>();
		physx::PxRigidActor* mRigidActor = nullptr;
		physx::PxRigidBody* mStaticPxRigidBody = nullptr;
		physx::PxRigidBody* pxRigidBody = nullptr;
		physx::PxMaterial* material = nullptr;
		bool mDynamic = false;
		virtual void OnInstantiate(Scene* scene, uint64_t toInstantiate) override;
		~Collider() override;
		void RemoveActor();
		void RemoveCollider();
		void Update() {};

		void AddShape(ColliderShape* shape);
		void AddConvexMeshShape(Mesh* mesh);
		void AddMeshShape(Mesh* mesh);
		void AddHeightShape(float** heightData, int size);
		template <typename EnumType> void SetSelfAndChildrenFlag(EnumType flag, bool value);

		void SetFlag(physx::PxRigidBody* rigidBody, physx::PxRigidBodyFlag::Enum flag, bool value);
		void SetFlag(Collider* collider, physx::PxRigidBodyFlag::Enum flag, bool value);
		void SetFlag(physx::PxRigidActor* actor, physx::PxActorFlag::Enum flag, bool value);
		void SetFlags(Collider* collider, physx::PxRigidDynamicLockFlags flags);

		template <class Archive> void serialize(Archive& archive) {
			archive(cereal::base_class<Component>(this), mDynamic, mShapes);
		}
	};
} // namespace Plaza
