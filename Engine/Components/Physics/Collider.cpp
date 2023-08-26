#include "Engine/Core/PreCompiledHeaders.h"
#include "Collider.h"
#include "Engine/Core/Physics.h"

Plaza::Vertex CombineVertices(const Plaza::Vertex& v1, const Plaza::Vertex& v2) {
	// Calculate the new position by averaging the positions of v1 and v2
	Plaza::Vertex combinedVertex = Plaza::Vertex(glm::vec3(0.0f));
	combinedVertex.position = (v1.position + v2.position) * 0.5f;
	// You might also want to consider other attributes like normals, UVs, etc.
	return combinedVertex;
}

float CalculateEdgeLength(const glm::vec3& p1, const glm::vec3& p2) {
	return glm::length(p2 - p1);
}

void SimplifyMesh(std::vector<Plaza::Vertex>& vertices, std::vector<unsigned int>& indices, int targetTriangleCount) {
	// Simplification loop
	while (indices.size() > targetTriangleCount * 3) {
		// Find the index of the triangle to remove
		float minCollapseCost = std::numeric_limits<float>::max();
		size_t collapseIndex = 0;

		for (size_t i = 0; i < indices.size(); i += 3) {
			// Calculate the cost of collapsing this triangle edge
			// You can use metrics like edge length, normal difference, etc.
			// Here, a simple metric is used: edge length
			float cost = CalculateEdgeLength(vertices[indices[i]].position, vertices[indices[i + 1]].position);

			if (cost < minCollapseCost) {
				minCollapseCost = cost;
				collapseIndex = i;
			}
		}

		// Collapse the edge
		vertices[indices[collapseIndex]] = CombineVertices(vertices[indices[collapseIndex]],
			vertices[indices[collapseIndex + 1]]);
		vertices[indices[collapseIndex + 1]].isValid = false; // Mark vertex as invalid

		// Update indices to remove the collapsed triangle
		indices.erase(indices.begin() + collapseIndex, indices.begin() + collapseIndex + 3);
	}

	// Remove invalid vertices
	vertices.erase(std::remove_if(vertices.begin(), vertices.end(),
		[](const Plaza::Vertex& vertex) { return !vertex.isValid; }),
		vertices.end());
}

namespace Plaza {
	
	Collider::Collider(std::uint64_t uuid, RigidBody* rigidBody) {
		this->uuid = uuid;
		if (Application->runningScene)
			this->Init(rigidBody);
	}

	Collider::~Collider() {
		if (mRigidActor) {
			for (physx::PxShape* shape : mShapes) {
				this->mRigidActor->detachShape(*shape);
			}
			Physics::m_scene->removeActor(*this->mRigidActor);
		}
	}

	void Collider::RemoveActor() {
		if (this->mRigidActor) {
			for (physx::PxShape* shape : mShapes) {
				this->mRigidActor->detachShape(*shape);
			}
			Physics::m_scene->removeActor(*this->mRigidActor);
			mRigidActor = nullptr;
		}
	}

	void Collider::Init(RigidBody* rigidBody) {
		// Check if Rigid Body exists
		this->mDynamic = rigidBody != nullptr;
		if (Application->runningScene || Application->copyingScene)
			InitCollider(rigidBody);
	}

	void Collider::InitCollider(RigidBody* rigidBody) {
		this->RemoveActor();
		physx::PxTransform* pxTransform = Physics::GetPxTransform(Application->activeScene->transformComponents.at(this->uuid));
		this->mRigidActor = Physics::m_physics->createRigidDynamic(*pxTransform);
		if(this->mRigidActor == nullptr)
			this->mRigidActor = Physics::m_physics->createRigidDynamic(*new physx::PxTransform(physx::PxIdentity(1.0f)));
		physx::PxMaterial* material = Physics::defaultMaterial;

		if (this->mDynamic) {
			material = Physics::m_physics->createMaterial(rigidBody->mStaticFriction, rigidBody->mDynamicFriction, rigidBody->mRestitution);
		}
		else {
			this->mRigidActor->is<physx::PxRigidDynamic>()->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
		}

		// Attach the shapes with the material to the actor
		for (physx::PxShape* shape : mShapes) {
			shape->setMaterials(&material, 1);
			this->mRigidActor->attachShape(*shape);
		}
		Physics::m_scene->addActor(*this->mRigidActor);
	}

	void Collider::AddConvexMeshShape(Mesh* mesh) {
		physx::PxShape* shape;
		physx::PxTriangleMeshGeometry triangleGeometry;
		std::vector<physx::PxVec3> vertices;
		for (const Plaza::Vertex& vertex : mesh->vertices) {
			vertices.push_back(physx::PxVec3(vertex.position.x, vertex.position.y, vertex.position.z));
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
		shape = Physics::m_physics->createShape(physx::PxConvexMeshGeometry(convexMesh),
			*Physics::defaultMaterial);
		this->mShapes.push_back(shape);
		delete mesh;
	}

	void Collider::AddMeshShape(Mesh* mesh) {
		//SimplifyMesh(mesh->vertices, mesh->indices, 50);
		physx::PxShape* shape;
		physx::PxTriangleMeshGeometry triangleGeometry;
		std::vector<physx::PxVec3> vertices;
		std::vector<physx::PxU32> indices;

		// Extract vertices and indices from the mesh
		for (const Plaza::Vertex& vertex : mesh->vertices) {
			vertices.push_back(physx::PxVec3(vertex.position.x, vertex.position.y, vertex.position.z));
		}
		for (const unsigned int& index : mesh->indices) {
			indices.push_back(static_cast<physx::PxU32>(index));
		}

		physx::PxTriangleMeshDesc meshDesc;
		meshDesc.points.data = vertices.data();
		meshDesc.points.count = static_cast<physx::PxU32>(vertices.size());
		meshDesc.points.stride = sizeof(physx::PxVec3);
		meshDesc.triangles.data = indices.data();
		meshDesc.triangles.count = static_cast<physx::PxU32>(indices.size()) / 3;
		meshDesc.triangles.stride = sizeof(physx::PxU32) * 3;
		//meshDesc. = physx::PxSDFDesc::
		physx::PxCookingParams params(Physics::m_physics->getTolerancesScale());
		//params. = physx::PxMeshCookingHint::eSIM_PERFORMANCE;
		physx::PxDefaultMemoryOutputStream buf;
		if (!PxCookTriangleMesh(params, meshDesc, buf))
			printf("failed");

		physx::PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
		physx::PxTriangleMesh* triangleMesh = Physics::m_physics->createTriangleMesh(input);
		shape = Physics::m_physics->createShape(physx::PxTriangleMeshGeometry(triangleMesh),
			*Physics::defaultMaterial);

		this->mShapes.push_back(shape);
		delete mesh;
	}

	void Collider::AddShape(physx::PxShape* shape) {
		this->mShapes.push_back(shape);
		Init(nullptr);
	}

	void Collider::UpdateAllShapesScale() {

	}

	void Collider::UpdateShapeScale(glm::vec3 scale) {
		for (int i = 0; i < this->mShapes.size(); ++i) {
			physx::PxShape* shape = this->mShapes[i];
			physx::PxGeometryHolder geometry = this->mShapes[i]->getGeometry();
			physx::PxShape* newShape = this->mShapes[i];
			physx::PxMaterial* material;
			this->mShapes[i]->getMaterials(&material, 1);
			// Scale the geometry parameters by the given factor
			if (geometry.getType() == physx::PxGeometryType::eBOX) {
				physx::PxBoxGeometry boxGeom = geometry.box();
				boxGeom.halfExtents = physx::PxVec3(scale.x / 2, scale.y / 2, scale.z / 2);
				newShape = Physics::m_physics->createShape(boxGeom, *material);
			}
			else if (geometry.getType() == physx::PxGeometryType::eSPHERE) {
				physx::PxSphereGeometry sphereGeometry = geometry.sphere();
				//boxGeom.halfExtents = physx::PxVec3(scale.x / 2, scale.y / 2, scale.z / 2);
				sphereGeometry.radius = (scale.x + scale.y + scale.z) / 3;
				newShape = Physics::m_physics->createShape(sphereGeometry, *material);
				//physx::PxSphereGeometry sphereGeom;
				//sphereGeom.radius *= 3;
				//this->mShapes[i]->setGeometry(physx::PxSphereGeometry(sphereGeom));
			}

			if (Application->runningScene && Application->runtimeScene->rigidBodyComponents.find(this->uuid) != Application->runtimeScene->rigidBodyComponents.end()) {
				// Found RigidBody
				RigidBody* rigidBody = &Application->runtimeScene->rigidBodyComponents.at(this->uuid);

			}
			else {

			}

			if (Application->runningScene) {
				if (this->mDynamic) {
					this->mRigidActor->detachShape(*this->mShapes[i]);
					this->mShapes[i] = newShape;
					this->mRigidActor->attachShape(*this->mShapes[i]);
				}
				else {
					this->mRigidActor->detachShape(*this->mShapes[i]);
					this->mShapes[i] = newShape;
					this->mRigidActor->attachShape(*this->mShapes[i]);
				}
			}
		}
	}

	template <typename EnumType>
	void Collider::SetSelfAndChildrenFlag(EnumType flag, bool value) {
		SetFlag(&Application->activeScene->rigidBodyComponents.at(this->uuid).mRigidActor, flag, value);
		for (uint64_t child : Application->activeScene->entities.at(this->uuid).childrenUuid) {
			SetFlag(&Application->activeScene->rigidBodyComponents.at(child).mRigidActor, flag, value);
		}
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

	void Collider::UpdatePose(Transform* transform) {
		if (transform) {
			glm::quat quaternion = transform->GetWorldQuaternion();
			physx::PxQuat pxQuaternion(quaternion.x, quaternion.y, quaternion.z, quaternion.w);

			physx::PxTransform* pxTransform = new physx::PxTransform(
				transform->GetWorldPosition().x, transform->GetWorldPosition().y, transform->GetWorldPosition().z,
				pxQuaternion);
			if (this->mRigidActor) {
				this->mRigidActor->setGlobalPose(*pxTransform);
			}

		}
	}

	void Collider::UpdatePose() {
		this->UpdatePose(this->GetGameObject()->GetComponent<Transform>());
	}
}