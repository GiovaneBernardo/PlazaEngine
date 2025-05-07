#pragma once
#include "Engine/Components/Component.h"
#include "Engine/Core/Engine.h"

namespace Plaza {
	class PLAZA_API RigidBody : public Component {
	  public:
		// FIX: ----- Rework Physics -----

		// physx::PxRigidStatic* rb;
		// physx::PxMaterial* mMaterial;
		virtual void OnInstantiate(Scene* scene, uint64_t toInstantiate) override;
		RigidBody(){};
		RigidBody(uint64_t uuid, bool initWithPhysics, bool dynamic = true);
		RigidBody(const RigidBody& other) = default;
		~RigidBody() {}

		void ApplyForce(const glm::vec3& force);
		void AddForce(const glm::vec3& force, physx::PxForceMode::Enum mode = physx::PxForceMode::eFORCE,
					  bool autowake = true);
		void AddTorque(const glm::vec3& torque, physx::PxForceMode::Enum mode = physx::PxForceMode::eFORCE,
					   bool autowake = true);
		float GetDrag();
		void SetDrag(float drag);
		glm::vec3 GetVelocity();
		void SetVelocity(const glm::vec3& vector);

		bool canUpdate = true;

		bool mUseGravity = true;
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

		template <class Archive> void serialize(Archive& archive) {
			uint8_t rigidDynamicLockFlagsInt = static_cast<uint8_t>(rigidDynamicLockFlags);
			archive(cereal::base_class<Component>(this), PL_SER(kinematic), PL_SER(dynamic),
					PL_SER(continuousDetection), PL_SER(mStaticFriction), PL_SER(mDynamicFriction),
					PL_SER(mRestitution), PL_SER(density), PL_SER(gravity), PL_SER(rigidDynamicLockFlagsInt));
			rigidDynamicLockFlags.setAll(static_cast<physx::PxRigidDynamicLockFlag::Enum>(rigidDynamicLockFlagsInt));
		}

	  private:
		shared_ptr<TransformComponent> transform;
	};
} // namespace Plaza
