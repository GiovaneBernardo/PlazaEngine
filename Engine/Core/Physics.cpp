#include "Engine/Core/PreCompiledHeaders.h"
#include "Physics.h"
#include <ThirdParty/PhysX/physx/include/cooking/Pxc.h>
#include "Engine/Core/Scene.h"
#include "Engine/ECS/ECSManager.h"
#include "Engine/Components/Physics/PhysicsMaterial.h"

using namespace physx;
namespace Plaza {
	physx::PxDefaultAllocator Physics::m_defaultAllocatorCallback;
	physx::PxDefaultErrorCallback Physics::m_defaultErrorCallback;
	physx::PxDefaultCpuDispatcher* Physics::m_dispatcher = nullptr;
	physx::PxTolerancesScale Physics::m_toleranceScale;
	physx::PxFoundation* Physics::m_foundation = nullptr;
	physx::PxPhysics* Physics::m_physics = nullptr;

	std::unordered_map<uint64_t, physx::PxGeometry*> Physics::sCookedGeometries =
		std::unordered_map<uint64_t, physx::PxGeometry*>();
	std::unordered_map<uint64_t, std::unordered_map<PhysicsMaterial, physx::PxShape*>> Physics::sShapes =
		std::unordered_map<uint64_t,
						   std::unordered_map<PhysicsMaterial, physx::PxShape*>>(); // Keys: Mesh UUID, Physics Material
																					// hash map

	physx::PxMaterial* Physics::defaultMaterial = nullptr;

	physx::PxScene* Physics::m_scene = nullptr;
	physx::PxMaterial* Physics::m_material = nullptr;
	physx::PxPvd* Physics::m_pvd = nullptr;
	bool Physics::m_canRun = true;

	const float Physics::maxFrameAdvance = 0.1f;
	float Physics::accumulatedTime = 0.0f;
	float Physics::stepSize = 1 / 60.0f;

	class CollisionCallback : public physx::PxSimulationEventCallback {
	  public:
		virtual void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs,
							   PxU32 nbPairs) override {
			// Your collision handling implementation
			for (PxU32 i = 0; i < nbPairs; i++) {
				const PxContactPair& cp = pairs[i];
				PxContactPairPoint contacts[1];
				PxU32 nbContacts = cp.extractContacts(contacts, 1);

				if (cp.events & PxPairFlag::eNOTIFY_TOUCH_FOUND) {
					PxVec3 collisionPoint = contacts[0].position;
					uint64_t uuid1 = (uint64_t)(pairHeader.actors[0]->userData);
					uint64_t uuid2 = (uint64_t)(pairHeader.actors[1]->userData);

					Scene* scene = Scene::GetActiveScene();
					Entity* mainEntity = scene->GetEntity(uuid1);
					CppScriptComponent* scriptComponent = scene->GetComponent<CppScriptComponent>(uuid1);
					if (scriptComponent) {
						RaycastHit hit = RaycastHit(uuid2, Physics::PhysXToGlm(collisionPoint), Physics::PhysXToGlm(contacts[0].normal), false);
						for (CppScript* script : scriptComponent->mScripts) {
							script->OnCollide(scene, hit);
						}
					}
				}
			}
		}

		virtual void onTrigger(PxTriggerPair* pairs, PxU32 count) override {
			// Your trigger handling implementation
			for (PxU32 i = 0; i < count; i++) {
				const PxTriggerPair& tp = pairs[i];
				std::cout << "Trigger detected\n";
				// Handle trigger events if necessary
			}
		}

		virtual void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override {
			// Handle constraint break events
			for (physx::PxU32 i = 0; i < count; i++) {
				// Implement the logic for constraint break events
			}
		}

		virtual void onWake(physx::PxActor** actors, physx::PxU32 count) override {
			// Handle wake events
			for (physx::PxU32 i = 0; i < count; i++) {
				// Implement the logic for wake events
			}
		}

		virtual void onSleep(physx::PxActor** actors, physx::PxU32 count) override {
			// Handle sleep events
			for (physx::PxU32 i = 0; i < count; i++) {
				// Implement the logic for sleep events
			}
		}

		virtual void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer,
							   const physx::PxU32 count) override {
			// Handle advance events
			for (physx::PxU32 i = 0; i < count; i++) {
				// Implement the logic for advance events
			}
		}
	};
	CollisionCallback mCollisionCallback;
	class UserErrorCallback : public physx::PxErrorCallback {
	  public:
		virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) {
			std::cout << "shit" << std::endl;
		}
	};

	bool Physics::Advance(float dt) {
		PLAZA_PROFILE_SECTION("Advance");
		accumulatedTime += dt;
		if (accumulatedTime < stepSize) {
			return false;
		}

		accumulatedTime -= stepSize;
		{
			PLAZA_PROFILE_SECTION("Simulate");
			m_scene->simulate(stepSize);
		}
		{
			PLAZA_PROFILE_SECTION("Fetch Results");
			Physics::m_scene->fetchResults(true);
		}
		return true;
	}

	PxFilterFlags FilterShaderExample(PxFilterObjectAttributes attributes0, PxFilterData filterData0,
									  PxFilterObjectAttributes attributes1, PxFilterData filterData1,
									  PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize) {
		// let triggers through
		if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1)) {
			pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
			return PxFilterFlag::eDEFAULT;
		}
		// generate contacts for all that were not filtered above
		pairFlags = PxPairFlag::eCONTACT_DEFAULT;

		// trigger the contact callback for pairs (A,B) where
		// the filtermask of A contains the ID of B and vice versa.
		if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
			pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;

		pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
		pairFlags |= PxPairFlag::eNOTIFY_CONTACT_POINTS;
		pairFlags |= PxPairFlag::eDETECT_CCD_CONTACT;
		return PxFilterFlags();
	}
	class ContactCallBack : public PxSimulationEventCallback {
		// PxSimulationEventCallback
		virtual void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count) override {};
		virtual void onWake(PxActor** actors, PxU32 count) override {};
		virtual void onSleep(PxActor** actors, PxU32 count) override {};
		virtual void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs,
							   PxU32 nbPairs) override {};
		virtual void onTrigger(PxTriggerPair* pairs, PxU32 count) override {};
		virtual void onAdvance(const PxRigidBody* const* bodyBuffer, const PxTransform* poseBuffer,
							   const PxU32 count) override {};
	};

	class FilterCallback : public PxSimulationFilterCallback {
		virtual PxFilterFlags pairFound(PxU64 pairID, PxFilterObjectAttributes attributes0, PxFilterData filterData0,
										const PxActor* a0, const PxShape* s0, PxFilterObjectAttributes attributes1,
										PxFilterData filterData1, const PxActor* a1, const PxShape* s1,
										PxPairFlags& pairFlags) override {
			return PxFilterFlag::eDEFAULT;
		}
		virtual void pairLost(PxU64 pairID, PxFilterObjectAttributes attributes0, PxFilterData filterData0,
							  PxFilterObjectAttributes attributes1, PxFilterData filterData1,
							  bool objectRemoved) override {};
		virtual bool statusChange(PxU64& pairID, PxPairFlags& pairFlags, PxFilterFlags& filterFlags) override {
			return false;
		}
	};

	void setupFiltering(PxShape* shape, PxU32 filterGroup, PxU32 filterMask) {
		PxFilterData filterData;
		filterData.word0 = filterGroup; // word0 = own ID
		filterData.word1 = filterMask;	// word1 = ID mask to filter pairs that trigger a contact callback
		shape->setSimulationFilterData(filterData);
	}
	CollisionCallback simulationEventCallback;
	FilterCallback filterCallback;
	physx::PxSceneDesc Physics::GetSceneDesc() {
		physx::PxSceneDesc sceneDesc(m_physics->getTolerancesScale());
		sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f); // Set your desired gravity
		sceneDesc.cpuDispatcher = m_dispatcher;
		sceneDesc.filterShader = FilterShaderExample;
		sceneDesc.filterCallback = &filterCallback;
		sceneDesc.simulationEventCallback = &simulationEventCallback;
		sceneDesc.flags = PxSceneFlag::eENABLE_CCD;
		// sceneDesc.simulationEventCallback = &collisionCallback;
		// sceneDesc.dynamicTreeRebuildRateHint = 100;
		return sceneDesc;
	}

	void Physics::Init() {
		PL_CORE_INFO("Initializating Physics");
		m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_defaultAllocatorCallback, m_defaultErrorCallback);
		if (!m_foundation) {
			PL_CORE_CRITICAL("PhysX foundation creation failed!");
		}
		Physics::InitPhysics();
		Physics::InitScene();
		Physics::InitDefaultGeometries();
	}

	void Physics::InitPhysics() {
		PxTolerancesScale toleranceScale;
		toleranceScale.speed = 9.81f;
		toleranceScale.length = 1;
		m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation, toleranceScale, true);
		m_dispatcher = PxDefaultCpuDispatcherCreate(2);
		if (!m_dispatcher) {
			std::cerr << "PhysX CPU dispatcher creation failed!" << std::endl;
		}

		// Physics::InitDefaultGeometries();
	}

	void Physics::InitScene() {
		// Create the PhysX scene
		m_scene = m_physics->createScene(Physics::GetSceneDesc());
		// m_scene->setSimulationEventCallback(&collisionCallback);
		Physics::defaultMaterial = Physics::m_physics->createMaterial(
			0.0f, 1.0f, 0.0f); /* TODO: MAKE DYNAMIC MATERIALS AND SET RESTITUTION ON DEFAULT MATERIAL TO 0.5 AGAIN*/

		std::cout << "Physics Initialized" << std::endl;
	}

	void Physics::Update(Scene* scene) {
		PLAZA_PROFILE_SECTION("Physics::Update");
		for (const uint64_t& uuid : SceneView<RigidBody>(scene)) {
			ECS::RigidBodySystem::Update(scene, uuid);
		}
	}

	physx::PxTransform Physics::GetPxTransform(TransformComponent& transform) {
		//// Create a dynamic rigid body
		glm::quat quaternion = glm::normalize(transform.GetWorldQuaternion());
		physx::PxQuat pxQuaternion(quaternion.x, quaternion.y, quaternion.z, quaternion.w);
		glm::vec3 worldPosition = transform.GetWorldPosition();
		physx::PxTransform pxTransform =
			physx::PxTransform(worldPosition.x, worldPosition.y, worldPosition.z, pxQuaternion);
		return pxTransform;
	}

	physx::PxTransform* Physics::ConvertMat4ToPxTransform(const glm::mat4& mat) {
		glm::vec3 translation = mat[3];
		glm::quat rotationQuaternion = glm::quat_cast(glm::mat3(mat));
		physx::PxQuat pxQuaternion(rotationQuaternion.x, rotationQuaternion.y, rotationQuaternion.z,
								   rotationQuaternion.w);
		return new physx::PxTransform(physx::PxVec3(translation.x, translation.y, translation.z), pxQuaternion);
	}

	physx::PxShape* Physics::GetPhysXShape(ColliderShape* colliderShape, PhysicsMaterial* material) {
		if (colliderShape->mPxShape != nullptr)
			return colliderShape->mPxShape;

		physx::PxGeometry* geometry = nullptr;
		uint64_t meshUuid = 0;
		bool foundCookedGeometry = sCookedGeometries.find(colliderShape->mMeshUuid) != sCookedGeometries.end();
		Mesh* mesh = AssetsManager::GetMesh(colliderShape->mMeshUuid);
		switch (colliderShape->mEnum) {
			case ColliderShape::BOX:
				meshUuid = 1;
				geometry = Physics::GetCubeGeometry();
				break;
			case ColliderShape::PLANE:
				meshUuid = 2;
				geometry = Physics::GetPlaneGeometry();
				break;
			case ColliderShape::SPHERE:
				meshUuid = 3;
				geometry = Physics::GetSphereGeometry();
				break;
			case ColliderShape::CAPSULE:
				meshUuid = 4;
				geometry = Physics::GetCapsuleGeometry();
				break;
			case ColliderShape::MESH:
				meshUuid = colliderShape->mMeshUuid;
				geometry = foundCookedGeometry ? Physics::GetGeometry(colliderShape->mMeshUuid)
											   : Physics::CookMeshGeometry(mesh);
				break;
			case ColliderShape::CONVEX_MESH:
				meshUuid = colliderShape->mMeshUuid;
				geometry = foundCookedGeometry ? Physics::GetGeometry(colliderShape->mMeshUuid)
											   : Physics::CookConvexMeshGeometry(mesh);
				break;
		}

		if (sShapes[meshUuid][*material] == nullptr) {
			sShapes[meshUuid][*material] = Physics::m_physics->createShape(*geometry, *material->mPhysxMaterial);
		}

		return sShapes[meshUuid][*material];
	}

	/*
					physx::PxGeometryHolder geometry = this->mShapes[i]->mPxShape->getGeometry();
				physx::PxShape* newShape = this->mShapes[i]->mPxShape;
				physx::PxMaterial* material;
				this->mShapes[i]->mPxShape->getMaterials(&material, 1);
				// Scale the geometry parameters by the given factor
				if (this->mShapes[i]->mEnum == ColliderShape::ColliderShapeEnum::BOX) {
					physx::PxBoxGeometry boxGeom = geometry.box();
					boxGeom.halfExtents = physx::PxVec3(scale.x / 2, scale.y / 2, scale.z / 2);
					shape->release();
					newShape = Physics::m_physics->createShape(boxGeom, *material);
				}
				else if (this->mShapes[i]->mEnum == ColliderShape::ColliderShapeEnum::PLANE) {
					physx::PxBoxGeometry planeGeom = geometry.box();
					planeGeom.halfExtents = physx::PxVec3(scale.x / 2, 0.001f, scale.z / 2);
					shape->release();
					newShape = Physics::m_physics->createShape(planeGeom, *material);
				}
				else if (this->mShapes[i]->mEnum == ColliderShape::ColliderShapeEnum::SPHERE) {
					physx::PxSphereGeometry sphereGeometry = geometry.sphere();
					//boxGeom.halfExtents = physx::PxVec3(scale.x / 2, scale.y / 2, scale.z / 2);
					sphereGeometry.radius = (scale.x + scale.y + scale.z) / 3;
					shape->release();
					newShape = Physics::m_physics->createShape(sphereGeometry, *material);
				}
				else if (this->mShapes[i]->mEnum == ColliderShape::ColliderShapeEnum::MESH) {
					physx::PxTriangleMeshGeometry meshGeometry(geometry.triangleMesh().triangleMesh,
	   physx::PxMeshScale(physx::PxVec3(scale.x, scale.y, scale.z), physx::PxQuat(physx::PxIdentity))); newShape =
	   Physics::m_physics->createShape(meshGeometry, *material); shape->release();
				}
				else if (this->mShapes[i]->mEnum == ColliderShape::ColliderShapeEnum::HEIGHT_FIELD) {
					physx::PxHeightFieldGeometry heightGeometry = geometry.heightField();
					heightGeometry.heightScale = 1.0f;
					shape->release();
					newShape = Physics::m_physics->createShape(heightGeometry, *material);
				}
				else {
					std::cout << "Shape not supported for scaling" << std::endl;
				}


				if (Application::Get()->runningScene && this->mRigidActor) {
					if (this->mDynamic) {
						this->mRigidActor->detachShape(*this->mShapes[i]->mPxShape);
						this->mShapes[i]->mPxShape = newShape;
						this->mRigidActor->attachShape(*newShape);
					}
					else {
						this->mRigidActor->detachShape(*this->mShapes[i]->mPxShape);
						this->mShapes[i]->mPxShape = newShape;
						this->mRigidActor->attachShape(*newShape);
					}
				}
	*/

	physx::PxMaterial* Physics::InitializePhysicsMaterial(float staticFriction, float dynamicFriction,
														  float restitution) {
		return Physics::m_physics->createMaterial(staticFriction, dynamicFriction, restitution);
	}

	PhysicsMaterial& Physics::GetDefaultPhysicsMaterial() {
		return AssetsManager::GetPhysicsMaterial(0.5f, 0.5f, 0.5f);
	}

	void Physics::InitDefaultGeometries() {
		sCookedGeometries.emplace(1, new physx::PxBoxGeometry(0.5f, 0.5f, 0.5f));
		sCookedGeometries.emplace(2, new physx::PxPlaneGeometry());
		sCookedGeometries.emplace(3, new physx::PxSphereGeometry(1.0f));
		sCookedGeometries.emplace(4, new physx::PxCapsuleGeometry(0.5f, 1.0f));
	}

	physx::PxGeometry* Physics::GetGeometry(uint64_t uuid) {
		auto it = sCookedGeometries.find(uuid);
		if (it != sCookedGeometries.end())
			return it->second;
		return nullptr;
	}

	physx::PxGeometry* Physics::GetCubeGeometry() { return GetGeometry(1); }

	physx::PxGeometry* Physics::GetPlaneGeometry() { return GetGeometry(2); }

	physx::PxGeometry* Physics::GetSphereGeometry() { return GetGeometry(3); }

	physx::PxGeometry* Physics::GetCapsuleGeometry() { return GetGeometry(4); }

	physx::PxGeometry* Physics::CookMeshGeometry(Mesh* mesh) {
		physx::PxShape* shape;
		physx::PxTriangleMeshGeometry triangleGeometry;
		std::vector<physx::PxVec3> vertices = std::vector<physx::PxVec3>();
		std::vector<physx::PxU32> indices = std::vector<physx::PxU32>();

		// Extract vertices and indices from the mesh
		vertices.resize(mesh->vertices.size());
		std::transform(mesh->vertices.begin(), mesh->vertices.end(), vertices.begin(),
					   [](const glm::vec3& glmVertex) { return physx::PxVec3(glmVertex.x, glmVertex.y, glmVertex.z); });
		indices.resize(mesh->indices.size());
		indices = mesh->indices;

		physx::PxTriangleMeshDesc meshDesc;

		meshDesc.points.data = vertices.data();
		meshDesc.points.count = static_cast<physx::PxU32>(vertices.size());
		meshDesc.points.stride = sizeof(physx::PxVec3);
		meshDesc.triangles.data = indices.data();
		meshDesc.triangles.count = static_cast<physx::PxU32>(indices.size()) / 3;
		meshDesc.triangles.stride = sizeof(physx::PxU32) * 3;
		physx::PxCookingParams params(Physics::m_physics->getTolerancesScale());
		params.meshPreprocessParams |= physx::PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;
		params.meshPreprocessParams |= physx::PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;
		// params.meshcook = physx::PxMeshCookingHint::eCOOKING_PERFORMANCE;

		// params. = physx::PxMeshCookingHint::eSIM_PERFORMANCE;
		physx::PxDefaultMemoryOutputStream buf;
		if (!PxCookTriangleMesh(params, meshDesc, buf))
			printf("failed");
		physx::PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
		physx::PxTriangleMesh* triangleMesh = Physics::m_physics->createTriangleMesh(input);
		physx::PxTriangleMeshGeometry* geometry = new physx::PxTriangleMeshGeometry(triangleMesh);
		sCookedGeometries.emplace(mesh->uuid, geometry);
		return geometry;
	}

	physx::PxGeometry* Physics::CookConvexMeshGeometry(Mesh* mesh) {
		physx::PxTriangleMeshGeometry triangleGeometry;
		std::vector<physx::PxVec3> vertices;
		for (const glm::vec3& vertex : mesh->vertices) {
			vertices.push_back(physx::PxVec3(vertex.x, vertex.y, vertex.z));
		}
		physx::PxConvexMeshDesc meshDesc;
		meshDesc.points.data = vertices.data();
		meshDesc.points.count = static_cast<physx::PxU32>(vertices.size());
		meshDesc.points.stride = sizeof(physx::PxVec3);
		meshDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;

		physx::PxTolerancesScale scale;
		physx::PxCookingParams params(scale);

		physx::PxDefaultMemoryOutputStream buf;
		physx::PxConvexMeshCookingResult::Enum result;
		if (!PxCookConvexMesh(params, meshDesc, buf, &result))
			PL_CORE_WARN("Convex Mesh Cooking failed!");
		physx::PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
		physx::PxConvexMesh* convexMesh = Physics::m_physics->createConvexMesh(input);
		physx::PxConvexMeshGeometry* geometry = new physx::PxConvexMeshGeometry(convexMesh);
		sCookedGeometries.emplace(mesh->uuid, geometry);

		return geometry;
	}

	void Physics::DeleteShape() {}
	void Physics::Raycast(glm::vec3 origin, glm::vec3 direction, RaycastHit& hit) {
		MyQueryFilterCallback filterCallback{};
		filterCallback.setEntityToIgnore(reinterpret_cast<void*>(0));
		physx::PxQueryFilterData filterData(physx::PxQueryFlag::ePREFILTER | physx::PxQueryFlag::eSTATIC |
											physx::PxQueryFlag::eDYNAMIC);

		physx::PxRaycastBuffer hitPhysx;
		bool status = Physics::m_scene->raycast(physx::PxVec3(origin.x, origin.y, origin.z),
												physx::PxVec3(direction.x, direction.y, direction.z), 15000.0f,
												hitPhysx, physx::PxHitFlag::eDEFAULT, filterData, &filterCallback);
		if (status) {
			hit.hitUuid = (uint64_t)hitPhysx.block.actor->userData;
			hit.point = glm::vec3(hitPhysx.block.position.x, hitPhysx.block.position.y, hitPhysx.block.position.z);
			hit.normal = glm::vec3(hitPhysx.block.normal.x, hitPhysx.block.normal.y, hitPhysx.block.normal.z);
		}
		else
			hit.missed = true;
	}

	physx::PxVec3 Physics::GlmToPhysX(const glm::vec3& vector) { return physx::PxVec3(vector.x, vector.y, vector.z); }
	glm::vec3 Physics::PhysXToGlm(const physx::PxVec3& vector) { return glm::vec3(vector.x, vector.y, vector.z); }
} // namespace Plaza
