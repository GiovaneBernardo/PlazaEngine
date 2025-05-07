#include "Engine/Core/PreCompiledHeaders.h"
#include "ECSManager.h"
#include "Engine/Core/Physics.h"
#include "Engine/Core/Scene.h"

namespace Plaza {
	void ECS::RigidBodySystem::Init(Scene* scene, uint64_t uuid) {
		if (!scene->mRunning)
			return;
		RigidBody* rigidBody = scene->GetComponent<RigidBody>(uuid);
		Collider* collider = scene->GetComponent<Collider>(uuid);
		// AddCollidersOfChildren(this->uuid);
		if (collider) {
			if (!collider->mRigidActor)
				ECS::ColliderSystem::InitCollider(scene, uuid);
			rigidBody->mRigidActor = collider->mRigidActor;
			physx::PxRigidBodyExt::setMassAndUpdateInertia(*rigidBody->mRigidActor->is<physx::PxRigidDynamic>(),
														   physx::PxReal(rigidBody->density));
			physx::PxRigidBodyExt::updateMassAndInertia(*rigidBody->mRigidActor->is<physx::PxRigidDynamic>(),
														physx::PxReal(rigidBody->density));

			collider->SetFlags(collider, rigidBody->rigidDynamicLockFlags);
		}
		else {
			rigidBody->mRigidActor = Physics::m_physics->createRigidDynamic(*new physx::PxTransform(physx::PxIdentity));
		}

		rigidBody->SetRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, rigidBody->continuousDetection);
		rigidBody->mRigidActor->is<physx::PxRigidDynamic>()->setLinearDamping(0.0f);
		rigidBody->mRigidActor->is<physx::PxRigidDynamic>()->setAngularDamping(0.0f);
		rigidBody->mRigidActor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
	}

	void ECS::RigidBodySystem::Update(Scene* scene, uint64_t uuid) {
		RigidBody* rigidBody = scene->GetComponent<RigidBody>(uuid);
		if (!rigidBody || !rigidBody->canUpdate)
			return;

		TransformComponent& transform = *scene->GetComponent<TransformComponent>(uuid);
		TransformComponent& parentTransform =
			*scene->GetComponent<TransformComponent>(scene->GetEntity(uuid)->parentUuid);
		// Convert Px to Glm
		physx::PxTransform pxTransform = rigidBody->mRigidActor->getGlobalPose();
		physx::PxQuat rotationQuaternion = pxTransform.q;
		glm::quat glmQuaternion(rotationQuaternion.w, rotationQuaternion.x, rotationQuaternion.y, rotationQuaternion.z);
		glm::vec3 eulerAngles = glm::eulerAngles(glm::normalize(glmQuaternion));

		// Transform the world rotation of the PxTransform to local rotation
		glm::quat transformedRotation =
			glm::normalize(glm::quat_cast(glm::inverse(parentTransform.GetWorldMatrix()) * glm::toMat4(glmQuaternion)));

		// Apply the delta rotation to prevent gimbal lock
		ECS::TransformSystem::SetLocalRotation(
			transform, scene, transformedRotation); //+= transformedRotation - glm::quat_cast(transform.localMatrix);
		ECS::TransformSystem::SetLocalPosition(transform, scene,
											   glm::vec3(pxTransform.p.x, pxTransform.p.y, pxTransform.p.z));
		ECS::TransformSystem::UpdateSelfAndChildrenTransform(transform, &parentTransform, scene);
	}

	void ECS::RigidBodySystem::UpdateGlobalPose(Scene* scene, uint64_t uuid) {
		RigidBody* rigidBody = scene->GetComponent<RigidBody>(uuid);
		if (rigidBody && rigidBody->mRigidActor) {
			TransformComponent& transform = *scene->GetComponent<TransformComponent>(uuid);
			physx::PxTransform* pxTransform = Physics::ConvertMat4ToPxTransform(transform.mWorldMatrix);
			rigidBody->mRigidActor->setGlobalPose(*pxTransform);
		}
	}

	void ECS::RigidBodySystem::AddCollidersOfChildren(Scene* scene, uint64_t parent) {
		RigidBody* rigidBody = scene->GetComponent<RigidBody>(parent);
		for (uint64_t child : scene->GetEntity(parent)->childrenUuid) {
			if (scene->HasComponent<Collider>(child) && !scene->HasComponent<RigidBody>(child)) {
				Collider* rigidBodyCollider = scene->GetComponent<Collider>(rigidBody->mUuid);
				Collider* collider = scene->GetComponent<Collider>(child);
				for (auto& colliderShape : collider->mShapes) {
					rigidBodyCollider->mShapes.push_back(colliderShape);
				}
				collider->RemoveActor();
				AddCollidersOfChildren(scene, child);
			}
		}
	}

	void ECS::RigidBodySystem::UpdateRigidBody(Scene* scene, uint64_t uuid) {
		RigidBody* rigidBody = scene->GetComponent<RigidBody>(uuid);
		if (rigidBody->mRigidActor) {
			Physics::m_scene->removeActor(*rigidBody->mRigidActor);
			rigidBody->mRigidActor = nullptr;
			if (scene->HasComponent<Collider>(uuid))
				scene->GetComponent<Collider>(uuid)->mRigidActor = nullptr;
			ECS::RigidBodySystem::Init(scene, uuid);
		}
	}
} // namespace Plaza
