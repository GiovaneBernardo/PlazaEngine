#include "Engine/Core/PreCompiledHeaders.h"
#include "RigidBody.h"
#include "Engine/Core/Physics.h"
#include "Collider.h"
#include "Editor/GUI/gizmo.h"
#include "Engine/ECS/ECSManager.h"
#include "Engine/Core/Scene.h"

using namespace physx;
namespace Plaza {
	void RigidBody::OnInstantiate(Scene* scene, uint64_t toInstantiate) {
		if (scene->mRunning)
			ECS::RigidBodySystem::Init(scene, this->mUuid);
	}

	RigidBody::RigidBody(uint64_t uuid, bool initWithPhysics, bool dynamic) {
		if (initWithPhysics) {
			this->mUuid = uuid;
			this->dynamic = dynamic;
			// Init();
		}
	}

	void RigidBody::ApplyForce(const glm::vec3& force) {
		if (this->mRigidActor) {
			// this->mRigidActor->is<PxRigidDynamic>()->addForce(PxVec3(force.x, force.y, force.z),
			// physx::PxForceMode::eFORCE);
			this->mRigidActor->is<PxRigidDynamic>()->setLinearVelocity(PxVec3(force.x, force.y, force.z));
			// this->mRigidActor->is<PxRigidDynamic>()->addForce(PxVec3(force.x, force.y, force.z),
			// physx::PxForceMode::eFORCE);
		}
	};

	void RigidBody::AddForce(const glm::vec3& force, physx::PxForceMode::Enum mode, bool autowake) {
		if (this->mRigidActor) {
			this->mRigidActor->is<PxRigidDynamic>()->addForce(PxVec3(force.x, force.y, force.z), mode, autowake);
		}
	}

	void RigidBody::AddTorque(const glm::vec3& torque, physx::PxForceMode::Enum mode, bool autowake) {
		if (this->mRigidActor) {
			this->mRigidActor->is<PxRigidDynamic>()->addTorque(PxVec3(torque.x, torque.y, torque.z), mode, autowake);
		}
	}

	float RigidBody::GetDrag() {
		if (this->mRigidActor) {
			return this->mRigidActor->is<PxRigidDynamic>()->getLinearDamping();
		}
	}

	void RigidBody::SetDrag(float drag) {
		if (this->mRigidActor) {
			this->mRigidActor->is<PxRigidDynamic>()->setLinearDamping(drag);
		}
	}

	glm::vec3 RigidBody::GetVelocity() {
		if (this->mRigidActor) {
			physx::PxVec3 pxVec = this->mRigidActor->is<PxRigidDynamic>()->getLinearVelocity();
			return glm::vec3(pxVec.x, pxVec.y, pxVec.z);
		}
	}

	void RigidBody::SetVelocity(const glm::vec3& vector) {
		if (this->mRigidActor) {
			this->mRigidActor->is<PxRigidDynamic>()->setLinearVelocity(physx::PxVec3(vector.x, vector.y, vector.z));
		}
	}

	void RigidBody::SetRigidBodyFlag(physx::PxRigidBodyFlag::Enum flag, bool value) {
		this->mRigidActor->is<physx::PxRigidDynamic>()->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true);
	}

	void RigidBody::SetRigidDynamicLockFlags(physx::PxRigidDynamicLockFlag::Enum flag, bool value) {
		if (value)
			this->rigidDynamicLockFlags.raise(flag);
		else
			this->rigidDynamicLockFlags.clear(flag);

		this->mRigidActor->is<physx::PxRigidDynamic>()->setRigidDynamicLockFlag(flag, value);
	}
} // namespace Plaza
