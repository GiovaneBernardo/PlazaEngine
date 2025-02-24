#include "Engine/Core/PreCompiledHeaders.h"
#include "ECSManager.h"
#include "Engine/Core/Physics.h"
#include "Engine/Core/Scene.h"

namespace Plaza {
	void ECS::ColliderSystem::RemoveActor(Collider* collider) {
		if (collider->mRigidActor) {
			for (auto& shape : collider->mShapes) {
				collider->mRigidActor->detachShape(*shape->mPxShape);
			}
			Physics::m_scene->removeActor(*collider->mRigidActor);
			collider->mRigidActor->release();
			collider->mRigidActor = nullptr;
		}
	}
	void ECS::ColliderSystem::Init(RigidBody* rigidBody) {

	}
	void ECS::ColliderSystem::InitCollider(Collider* collider, TransformComponent* transform, RigidBody* rigidBody) {
		collider->RemoveActor();
		if (rigidBody)
			collider->mDynamic = true;
		physx::PxTransform pxTransform = Physics::GetPxTransform(*transform);
		collider->mRigidActor = Physics::m_physics->createRigidDynamic(pxTransform);
		if (collider->mRigidActor == nullptr)
			collider->mRigidActor = Physics::m_physics->createRigidDynamic(physx::PxTransform(physx::PxIdentity));
		if (!collider->material) {
			collider->material = Physics::defaultMaterial;
			if (collider->mDynamic && rigidBody) {
				collider->material = Physics::m_physics->createMaterial(rigidBody->mStaticFriction, rigidBody->mDynamicFriction, rigidBody->mRestitution);
			}
		}
		if (!collider->mDynamic)
			collider->mRigidActor->is<physx::PxRigidDynamic>()->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);


		collider->mRigidActor->userData = reinterpret_cast<void*>(collider->mUuid);
		// Attach the shapes with the material to the actor
		for (auto& shape : collider->mShapes) {
			if (shape->mPxShape == nullptr) {
				shape->mPxShape = Physics::GetPhysXShape(shape.get(), &Physics::GetDefaultPhysicsMaterial());
			}

			shape->mPxShape->userData = reinterpret_cast<void*>(collider->mUuid);
			collider->mRigidActor->attachShape(*shape->mPxShape);
		}
		Physics::m_scene->addActor(*collider->mRigidActor);
	}

	void ECS::ColliderSystem::InitCollider(Scene* scene, uint64_t uuid) {
		ECS::ColliderSystem::InitCollider(scene->GetComponent<Collider>(uuid), scene->GetComponent<TransformComponent>(uuid), scene->GetComponent<RigidBody>(uuid));
	}

	void ECS::ColliderSystem::RemoveCollider() {

	}
	void ECS::ColliderSystem::CreateShape(Collider* collider, TransformComponent* transform, ColliderShape::ColliderShapeEnum shapeEnum, Mesh* mesh) {
		if (shapeEnum == ColliderShape::ColliderShapeEnum::BOX) {
			physx::PxBoxGeometry geometry(0.5f, 0.5f, 0.5f);
			collider->AddShape(new ColliderShape(Physics::m_physics->createShape(geometry, *Physics::defaultMaterial), ColliderShape::ColliderShapeEnum::BOX, 0));
		}
		else if (shapeEnum == ColliderShape::ColliderShapeEnum::SPHERE) {
			physx::PxSphereGeometry geometry(0.5f);
			collider->AddShape(new ColliderShape(Physics::m_physics->createShape(geometry, *Physics::defaultMaterial), ColliderShape::ColliderShapeEnum::SPHERE, 0));
		}
		else if (shapeEnum == ColliderShape::ColliderShapeEnum::PLANE) {
			physx::PxBoxGeometry geometry(0.5f, 0.001f, 0.5f);
			collider->AddShape(new ColliderShape(Physics::m_physics->createShape(geometry, *Physics::defaultMaterial), ColliderShape::ColliderShapeEnum::PLANE, 0));
		}
		else if (shapeEnum == ColliderShape::ColliderShapeEnum::CAPSULE) {
			/* TODO: Implement capsule shape*/
			std::cout << "Capsule not yet implemented" << std::endl;
		}
		else if (shapeEnum == ColliderShape::ColliderShapeEnum::CYLINDER) {
			/* TODO: Implement cylinder shape*/
			std::cout << "Cylinder not yet implemented" << std::endl;
		}
		else if (shapeEnum == ColliderShape::ColliderShapeEnum::MESH) {
			collider->AddMeshShape(mesh);
		}
		else if (shapeEnum == ColliderShape::ColliderShapeEnum::CONVEX_MESH) {
			collider->AddConvexMeshShape(mesh);
		}
	}
	void ECS::ColliderSystem::AddShape(ColliderShape* shape) {

	}
	void ECS::ColliderSystem::AddConvexMeshShape(Mesh* mesh) {

	}
	void ECS::ColliderSystem::AddMeshShape(Mesh* mesh) {

	}
	void ECS::ColliderSystem::AddHeightShape(float** heightData, int size) {

	}
	void ECS::ColliderSystem::UpdateShapeScale(Scene* scene, Collider* collider, const glm::vec3& scale) {
		PLAZA_PROFILE_SECTION("Collider: Update Shape Scale");
		if (!scene->mRunning)
			return;
		if (collider->lastScale != scale) {
			if (scale.x == 0 || scale.y == 0 || scale.z == 0) {
				collider->lastScale = scale;
				return;
			}

			for (int i = 0; i < collider->mShapes.size(); ++i) {
				physx::PxShape* shape = collider->mShapes[i]->mPxShape;
				if (!shape)
					continue;
				physx::PxGeometryHolder geometry = collider->mShapes[i]->mPxShape->getGeometry();
				physx::PxShape* newShape = collider->mShapes[i]->mPxShape;
				physx::PxMaterial* material;
				collider->mShapes[i]->mPxShape->getMaterials(&material, 1);
				// Scale the geometry
				if (collider->mShapes[i]->mEnum == ColliderShape::ColliderShapeEnum::BOX) {
					physx::PxBoxGeometry boxGeom = geometry.box();
					boxGeom.halfExtents = physx::PxVec3(scale.x / 2, scale.y / 2, scale.z / 2);
					//shape->release();
					newShape = Physics::m_physics->createShape(boxGeom, *material);
				}
				else if (collider->mShapes[i]->mEnum == ColliderShape::ColliderShapeEnum::PLANE) {
					physx::PxBoxGeometry planeGeom = geometry.box();
					planeGeom.halfExtents = physx::PxVec3(scale.x / 2, 0.001f, scale.z / 2);
					//shape->release();
					newShape = Physics::m_physics->createShape(planeGeom, *material);
				}
				else if (collider->mShapes[i]->mEnum == ColliderShape::ColliderShapeEnum::SPHERE) {
					physx::PxSphereGeometry sphereGeometry = geometry.sphere();
					sphereGeometry.radius = (scale.x + scale.y + scale.z) / 3 / 2;
					//shape->release();
					newShape = Physics::m_physics->createShape(sphereGeometry, *material);
				}
				else if (collider->mShapes[i]->mEnum == ColliderShape::ColliderShapeEnum::MESH) {
					physx::PxTriangleMeshGeometry meshGeometry(geometry.triangleMesh().triangleMesh, physx::PxMeshScale(physx::PxVec3(scale.x, scale.y, scale.z), physx::PxQuat(physx::PxIdentity)));
					newShape = Physics::m_physics->createShape(meshGeometry, *material);
					//shape->release();
				}
				else if (collider->mShapes[i]->mEnum == ColliderShape::ColliderShapeEnum::HEIGHT_FIELD) {
					physx::PxHeightFieldGeometry heightGeometry = geometry.heightField();
					heightGeometry.heightScale = 1.0f;
					//shape->release();
					newShape = Physics::m_physics->createShape(heightGeometry, *material);
				}
				else if (collider->mShapes[i]->mEnum == ColliderShape::ColliderShapeEnum::CAPSULE) {
					physx::PxCapsuleGeometry capsuleGeometry = geometry.capsule();
					capsuleGeometry.radius = scale.x / 2.0f;
					capsuleGeometry.halfHeight = scale.y;
					//shape->release();
					newShape = Physics::m_physics->createShape(capsuleGeometry, *material);
				}
				else {
					PL_CORE_ERROR("Shape not supported for scaling");
				}


				if (scene->mRunning && collider->mRigidActor) {
					if (collider->mDynamic) {
						collider->mRigidActor->detachShape(*collider->mShapes[i]->mPxShape);
						collider->mShapes[i]->mPxShape = newShape;
						collider->mRigidActor->attachShape(*newShape);
					}
					else {
						collider->mRigidActor->detachShape(*collider->mShapes[i]->mPxShape);
						collider->mShapes[i]->mPxShape = newShape;
						collider->mRigidActor->attachShape(*newShape);
					}
				}
			}
		}
		collider->lastScale = scale;
	}
	void ECS::ColliderSystem::UpdateAllShapesScale(Scene* scene, uint64_t uuid) {
		if (scene->HasComponent<Collider>(uuid))
			ECS::ColliderSystem::UpdateShapeScale(scene, scene->GetComponent<Collider>(uuid), scene->GetComponent<TransformComponent>(uuid)->GetWorldScale());
	}

	void ECS::ColliderSystem::UpdatePose(Collider* collider, TransformComponent* transform) {
		if (transform) {
			if (collider->mRigidActor) {
				glm::quat quaternion = glm::quat(transform->GetWorldRotation());

				physx::PxQuat pxQuaternion(quaternion.x, quaternion.y, quaternion.z, quaternion.w);

				glm::vec3 transformPos = transform->GetWorldPosition();
				glm::vec3 worldScale = transform->GetWorldScale();
				physx::PxVec3 pxTranslation(transformPos.x, transformPos.y, transformPos.z);
				physx::PxTransform pxTransform(pxTranslation, pxQuaternion);

				collider->mRigidActor->setGlobalPose(pxTransform);
			}
		}
	}
}