#pragma once
#include "Engine/Components/Component.h"

namespace Plaza {
	class RigidBody : public Component {
	public:
		//physx::PxRigidStatic* rb;
		//physx::PxMaterial* mMaterial;
		RigidBody() {};
		RigidBody(uint64_t uuid, bool initWithPhysics, bool dynamic = true);
		RigidBody(const RigidBody& other) = default;
		~RigidBody();

		void Init();

		void Update();
		void UpdateGlobalPose();
		void UpdateRigidBody();

		void AddCollidersOfChildren(uint64_t parent);

		void ApplyForce(glm::vec3 force);
		void AddForce(glm::vec3 force, physx::PxForceMode::Enum mode = physx::PxForceMode::eFORCE, bool autowake = true);
		void AddTorque(glm::vec3 torque, physx::PxForceMode::Enum mode = physx::PxForceMode::eFORCE, bool autowake = true);
		float GetDrag();
		void SetDrag(float drag);
		glm::vec3 GetVelocity();
		void SetVelocity(glm::vec3 vector);

		bool canUpdate = true;

		bool kinematic = false;
		bool dynamic = true;
		bool continuousDetection = false;
		float mStaticFriction = 0.0f;
		float mDynamicFriction = 1.0f;
		float mRestitution = 0.5f;

		physx::PxRigidDynamicLockFlags rigidDynamicLockFlags;
		void SetRigidDynamicLockFlags(physx::PxRigidDynamicLockFlag::Enum flag, bool value);
		void SetRigidBodyFlag(physx::PxRigidBodyFlag::Enum flag, bool value);


		bool lockRotation = false;

		float density = 50.0f;
		glm::vec3 gravity = glm::vec3(0.0f, -9.81f, 0.0f);

		physx::PxRigidActor* mRigidActor = nullptr;

		void CopyValuesFrom(const RigidBody& other) {
			dynamic = other.dynamic;
			mStaticFriction = other.mStaticFriction;
			mDynamicFriction = other.mDynamicFriction;
			mRestitution = other.mRestitution;
			density = other.density;
			gravity = other.gravity;
		}
	private:

		shared_ptr<Transform> transform;
	};
}