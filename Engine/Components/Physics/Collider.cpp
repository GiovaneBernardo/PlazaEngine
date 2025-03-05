#include "Engine/Core/PreCompiledHeaders.h"
#include "Collider.h"
#include "Engine/Core/Physics.h"
#include "ThirdParty/PhysX/physx/include/geometry/PxHeightFieldGeometry.h"
#include "Engine/Core/Scene.h"

namespace Plaza {

	void Collider::OnInstantiate(Scene* scene, uint64_t toInstantiate) {
		if (!scene->mRunning)
			return;
		Collider* componentToInstantiate = scene->GetComponent<Collider>(toInstantiate);
		mShapes.clear();
		mRigidActor = nullptr;
		for (auto& shape : static_cast<Collider*>(componentToInstantiate)->mShapes) {
			shape->mPxShape = Physics::GetPhysXShape(shape.get(), &Physics::GetDefaultPhysicsMaterial());
			physx::PxGeometryHolder geometry = shape->mPxShape->getGeometry();
			physx::PxMaterial* shapeMaterial = Physics::defaultMaterial;
			// Create a new shape with the same properties
			physx::PxShape* newShape = Physics::m_physics->createShape(geometry.triangleMesh(), *shapeMaterial, false);
			mShapes.push_back(std::make_shared<ColliderShape>(newShape, shape->mEnum, shape->mMeshUuid));
		}
		// Scene::GetActiveScene()->colliderComponents.find(mUuid)->second.Init(nullptr);
	}

	Collider::~Collider() {
		if (mRigidActor) {
			for (auto& shape : mShapes) {
				this->mRigidActor->detachShape(*shape->mPxShape);
				// if (shape->mPxShape->userData)
				//	shape->mPxShape->release();
			}
			Physics::m_scene->removeActor(*this->mRigidActor);
			if (this->mRigidActor->userData)
				this->mRigidActor->release();
			// if (this->material->userData)
			//	this->material->release();
		}
	}

	void Collider::RemoveActor() {
		if (this->mRigidActor) {
			for (auto& shape : mShapes) {
				this->mRigidActor->detachShape(*shape->mPxShape);
			}
			Physics::m_scene->removeActor(*this->mRigidActor);
			this->mRigidActor->release();
			mRigidActor = nullptr;
		}
	}

	void Collider::RemoveCollider() { this->RemoveActor(); }

	void Collider::AddConvexMeshShape(Mesh* mesh) {
		physx::PxShape* shape;
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
			printf("failed");
		physx::PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
		physx::PxConvexMesh* convexMesh = Physics::m_physics->createConvexMesh(input);
		shape = Physics::m_physics->createShape(physx::PxConvexMeshGeometry(convexMesh), *Physics::defaultMaterial);

		this->mShapes.push_back(
			std::make_shared<ColliderShape>(shape, ColliderShape::ColliderShapeEnum::CONVEX_MESH, mesh->uuid));
		delete mesh;
	}

	void Collider::AddMeshShape(Mesh* mesh) {
		auto start0 = std::chrono::high_resolution_clock::now();
		physx::PxShape* shape;
		physx::PxTriangleMeshGeometry triangleGeometry;
		std::vector<physx::PxVec3> vertices = std::vector<physx::PxVec3>();
		std::vector<physx::PxU32> indices = std::vector<physx::PxU32>();

		// Extract vertices and indices from the mesh
		auto start1 = std::chrono::high_resolution_clock::now();
		vertices.resize(mesh->vertices.size());
		std::transform(mesh->vertices.begin(), mesh->vertices.end(), vertices.begin(),
					   [](const glm::vec3& glmVertex) { return physx::PxVec3(glmVertex.x, glmVertex.y, glmVertex.z); });
		indices.resize(mesh->indices.size());
		indices = mesh->indices;

		auto end1 = std::chrono::high_resolution_clock::now();
		// for (const glm::vec3& vertex : mesh->vertices) {
		//	vertices.push_back(physx::PxVec3(vertex.x, vertex.y, vertex.z));
		// }
		// for (const unsigned int& index : mesh->indices) {
		//	indices.push_back(static_cast<physx::PxU32>(index));
		// }

		auto start2 = std::chrono::high_resolution_clock::now();
		physx::PxTriangleMeshDesc meshDesc;

		meshDesc.points.data = vertices.data();
		meshDesc.points.count = static_cast<physx::PxU32>(vertices.size());
		meshDesc.points.stride = sizeof(physx::PxVec3);
		meshDesc.triangles.data = indices.data();
		meshDesc.triangles.count = static_cast<physx::PxU32>(indices.size()) / 3;
		meshDesc.triangles.stride = sizeof(physx::PxU32) * 3;
		// meshDesc. = physx::PxSDFDesc::
		physx::PxCookingParams params(Physics::m_physics->getTolerancesScale());
		params.meshPreprocessParams |= physx::PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;
		// disable edge precompute, edges are set for each triangle, slows contact generation
		params.meshPreprocessParams |= physx::PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;
		// lower hierarchy for internal mesh
		// params.meshcook = physx::PxMeshCookingHint::eCOOKING_PERFORMANCE;

		// params. = physx::PxMeshCookingHint::eSIM_PERFORMANCE;
		auto end2 = std::chrono::high_resolution_clock::now();
		auto start3 = std::chrono::high_resolution_clock::now();
		physx::PxDefaultMemoryOutputStream buf;
		if (!PxCookTriangleMesh(params, meshDesc, buf))
			printf("failed");
		auto end3 = std::chrono::high_resolution_clock::now();
		auto start4 = std::chrono::high_resolution_clock::now();
		physx::PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
		physx::PxTriangleMesh* triangleMesh = Physics::m_physics->createTriangleMesh(input);
		shape = Physics::m_physics->createShape(physx::PxTriangleMeshGeometry(triangleMesh), *Physics::defaultMaterial);
		auto end4 = std::chrono::high_resolution_clock::now();
		this->mShapes.push_back(
			std::make_shared<ColliderShape>(shape, ColliderShape::ColliderShapeEnum::MESH, mesh->uuid));
		auto end0 = std::chrono::high_resolution_clock::now();

		std::cout << "Time taken by function 1: "
				  << std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1).count() << " milliseconds"
				  << std::endl;
		std::cout << "Time taken by function 2: "
				  << std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2).count() << " milliseconds"
				  << std::endl;
		std::cout << "Time taken by function 3: "
				  << std::chrono::duration_cast<std::chrono::milliseconds>(end3 - start3).count() << " milliseconds"
				  << std::endl;
		std::cout << "Time taken by function 4: "
				  << std::chrono::duration_cast<std::chrono::milliseconds>(end4 - start4).count() << " milliseconds"
				  << std::endl;
		std::cout << "Time taken by function 0: "
				  << std::chrono::duration_cast<std::chrono::milliseconds>(end0 - start0).count() << " milliseconds"
				  << std::endl;
		// delete mesh;
	}

	void Collider::AddHeightShape(float** heightData, int size) {
		auto start0 = std::chrono::high_resolution_clock::now();
		physx::PxShape* shape;
		physx::PxHeightFieldSample* hfSamples = new physx::PxHeightFieldSample[size * size];

		physx::PxReal min = PX_MAX_F32;
		physx::PxReal max = -PX_MAX_F32;

		Mesh* debugMesh = new Mesh();
		debugMesh->vertices.reserve(size);
		debugMesh->indices.reserve(size);
		debugMesh->indices.push_back(0);
		debugMesh->indices.push_back(0);
		debugMesh->indices.push_back(0);
		physx::PxReal quantization = (physx::PxReal)0x7fff;

		for (unsigned int col = 0; col < size; col++) {
			for (unsigned int row = 0; row < size; row++) {
				min = physx::PxMin(min, heightData[col][row]);
				max = physx::PxMax(max, heightData[col][row]);
			}
		}
		using namespace physx;
		physx::PxReal deltaHeight = max - min;
		physx::PxReal heightScale = physx::PxMax<physx::PxReal>(deltaHeight / quantization, PX_MIN_HEIGHTFIELD_Y_SCALE);

		for (unsigned int col = 0; col < size; col++) {
			for (unsigned int row = 0; row < size; row++) {
				physx::PxI16 height = physx::PxI16(quantization * ((heightData[col][row] - min) / deltaHeight));
				physx::PxHeightFieldSample& smp = hfSamples[(row * size) + col]; // Corrected indexing here
				smp.height = heightData[col][row];
			}
		}

		// Cook the height field
		physx::PxHeightFieldDesc heightFieldDesc;
		heightFieldDesc.format = physx::PxHeightFieldFormat::eS16_TM;
		heightFieldDesc.nbColumns = size;
		heightFieldDesc.nbRows = size;
		heightFieldDesc.samples.data = hfSamples;
		heightFieldDesc.flags = physx::PxHeightFieldFlags();
		heightFieldDesc.samples.data = new float[size * size];
		heightFieldDesc.samples.stride = sizeof(float);
		char* currentByte = (char*)heightFieldDesc.samples.data;
		for (physx::PxU32 row = 0; row < size; row++) {
			for (physx::PxU32 col = 0; col < size; col++) {
				physx::PxHeightFieldSample* currentSample = (physx::PxHeightFieldSample*)currentByte;
				currentSample->height = (float)heightData[row][col];
				currentByte += heightFieldDesc.samples.stride;
			}
		}

		physx::PxHeightField* aHeightField = PxCreateHeightField(heightFieldDesc);
		std::cout << "First: " << heightData[255][255] << "\n";
		std::cout << "Second: " << aHeightField->getSample(255, 255).height << "\n";
		std::cout << "Third: " << size * 0.5f * 20 << "\n";
		physx::PxHeightFieldGeometry hfGeom(aHeightField, physx::PxMeshGeometryFlags(), 1, 20, 20);

		float terrainWidth = 256 / 2 * 20;
		PxTransform localPose;
		localPose.p = PxVec3(-terrainWidth,		// make it so that the center of the
							 0, -terrainWidth); // heightfield is at world (0,minHeight,0)
		localPose.q = PxQuat(PxIdentity);
		// shape->setLocalPose(localPose);
		shape = Physics::m_physics->createShape(hfGeom, *Physics::defaultMaterial, false);
		shape->setLocalPose(localPose);
		this->mShapes.push_back(std::make_shared<ColliderShape>(shape, ColliderShape::ColliderShapeEnum::HEIGHT_FIELD));
		auto end0 = std::chrono::high_resolution_clock::now();
		delete[] hfSamples;

		std::cout << "Time taken by function 0: "
				  << std::chrono::duration_cast<std::chrono::milliseconds>(end0 - start0).count() << " milliseconds"
				  << std::endl;
		// delete mesh;
	}

	void Collider::AddShape(ColliderShape* shape) { this->mShapes.push_back(std::make_shared<ColliderShape>(*shape)); }

	template <typename EnumType> void Collider::SetSelfAndChildrenFlag(EnumType flag, bool value) {
		// FIX: Move to systems manager
		// SetFlag(&Scene::GetActiveScene()->rigidBodyComponents.at(this->mUuid).mRigidActor, flag, value);
		// for (uint64_t child : Scene::GetActiveScene()->entities.at(this->mUuid).childrenUuid) {
		//	SetFlag(&Scene::GetActiveScene()->rigidBodyComponents.at(child).mRigidActor, flag, value);
		//}
	}

	void Collider::SetFlag(physx::PxRigidActor* actor, physx::PxActorFlag::Enum flag, bool value) {
		actor->setActorFlag(flag, value);
	}
	void Collider::SetFlag(physx::PxRigidBody* rigidBody, physx::PxRigidBodyFlag::Enum flag, bool value) {
		rigidBody->setRigidBodyFlag(flag, value);
	}
	void Collider::SetFlag(Collider* collider, physx::PxRigidBodyFlag::Enum flag, bool value) {
		collider->mStaticPxRigidBody->setRigidBodyFlag(flag, value);
	}
	void Collider::SetFlags(Collider* collider, physx::PxRigidDynamicLockFlags flags) {
		collider->mRigidActor->is<physx::PxRigidDynamic>()->setRigidDynamicLockFlags(flags);
	}
} // namespace Plaza
