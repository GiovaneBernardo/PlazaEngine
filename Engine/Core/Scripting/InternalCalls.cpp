#include "Engine/Core/PreCompiledHeaders.h"
#include "Mono.h"
#include "Engine/Core/Input/Input.h"
#include "Engine/Core/Input/Cursor.h"

#include "Editor/Filewatcher.h"
namespace Plaza {

#define PL_ADD_INTERNAL_CALL(name) mono_add_internal_call("Plaza.InternalCalls::" #name, (void*)name)

	enum Axis {
		X,
		Y,
		Z
	};

	static bool HasComponent(uint64_t uuid, MonoReflectionType* componentType) {
		MonoType* monoType = mono_reflection_type_get_type(componentType);
		return Mono::mEntityHasComponentFunctions.at(monoType)(*Application->activeScene->GetEntity(uuid));
		//return Application->activeScene->entities.at(uuid).HasComponent<type>();
	}

	static uint64_t FindEntityByNameCall(MonoString* name) {
		char* nameCStr = mono_string_to_utf8(name);

		Entity* entity = Application->activeScene->GetEntityByName(nameCStr);
		mono_free(nameCStr);
		if (!entity)
			return 0;
		return entity->uuid;
	}

	static uint64_t Instantiate(uint64_t uuid) {
		Entity* entityToInstantiate = &Application->activeScene->entities.at(uuid);
		Entity* instantiatedEntity = new Entity(entityToInstantiate->name, &Application->activeScene->entities.at(entityToInstantiate->parentUuid));
		instantiatedEntity = &Application->activeScene->entities.at(instantiatedEntity->uuid);

		instantiatedEntity->GetComponent<Transform>()->SetRelativePosition(entityToInstantiate->GetComponent<Transform>()->relativePosition + glm::vec3(1.0f, 1.0f, 1.0f));
		instantiatedEntity->GetComponent<Transform>()->SetRelativeRotation(entityToInstantiate->GetComponent<Transform>()->rotation);

		if (entityToInstantiate->HasComponent<MeshRenderer>()) {
			MeshRenderer* meshRendererToInstantiate = entityToInstantiate->GetComponent<MeshRenderer>();
			MeshRenderer* newMeshRenderer = new MeshRenderer();
			newMeshRenderer->uuid = instantiatedEntity->uuid;
			newMeshRenderer->instanced = true;
			newMeshRenderer->mesh = shared_ptr<Mesh>(meshRendererToInstantiate->mesh);
			instantiatedEntity->AddComponent<MeshRenderer>(newMeshRenderer);
		}

		if (entityToInstantiate->HasComponent<Collider>()) {
			Collider* newCollider = new Collider(*entityToInstantiate->GetComponent<Collider>());
			newCollider->uuid = instantiatedEntity->uuid;
			instantiatedEntity->AddComponent<Collider>(newCollider);
		}

		if (entityToInstantiate->HasComponent<RigidBody>()) {
			RigidBody* newRigidBody = new RigidBody(*entityToInstantiate->GetComponent<RigidBody>());
			newRigidBody->uuid = instantiatedEntity->uuid;
			instantiatedEntity->AddComponent<RigidBody>(newRigidBody);
			instantiatedEntity->GetComponent<RigidBody>()->Init();
		}
		return instantiatedEntity->uuid;
	}

#pragma region Input

	static bool InputIsKeyDown(int keyCode) {
		if (Application->focusedMenu == "Scene") {
			return glfwGetKey(Application->Window->glfwWindow, keyCode) == GLFW_PRESS;
		}
	}

	static bool IsKeyReleased(int keyCode) {
		if (Application->focusedMenu == "Scene") {
			return glfwGetKey(Application->Window->glfwWindow, keyCode) == GLFW_RELEASE;
		}
	}

	static bool InputIsMouseDown(int button) {
		if (Application->focusedMenu == "Scene") {
			return glfwGetMouseButton(Application->Window->glfwWindow, button) == GLFW_PRESS;
		}
	}

	static void GetMouseDelta(glm::vec2* out) {
		*out = glm::vec2(Input::Cursor::deltaX, Input::Cursor::deltaY);
	}

	static void CursorHide(bool val) {
		Editor::Filewatcher::mMainThreadQueue.push_back([val]() {
			if (val && glfwGetInputMode(Application->Window->glfwWindow, GLFW_CURSOR) != GLFW_CURSOR_HIDDEN) {
				glfwSetInputMode(Application->Window->glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}
			else if (!val && glfwGetInputMode(Application->Window->glfwWindow, GLFW_CURSOR) != GLFW_CURSOR_NORMAL)
				glfwSetInputMode(Application->Window->glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			Input::Cursor::show = !val;
			});

	}

#pragma region Entity
	static uint64_t EntityGetParent(uint64_t uuid) {
		auto it = Application->activeScene->entities.find(uuid);
		if (it != Application->activeScene->entities.end()) {
			return it->second.uuid;
		}
	}
	static void EntitySetParent(uint64_t uuid, uint64_t parentUuid) {
		auto it = Application->activeScene->entities.find(uuid);
		if (it != Application->activeScene->entities.end()) {
			if (parentUuid == 0) {
				it->second.ChangeParent(it->second.GetParent(), Application->activeScene->entities.at(Application->activeScene->mainSceneEntity->uuid));
			}
			else {
				auto parentIt = Application->activeScene->entities.find(parentUuid);
				if (parentIt != Application->activeScene->entities.end())
					it->second.ChangeParent(it->second.GetParent(), parentIt->second);
			}
		}
	}
#pragma endregion Entity

#pragma region Components

#pragma region Transform Component
	static void SetPosition(uint64_t uuid, glm::vec3* vec3) {
		Application->activeScene->transformComponents.at(uuid).SetRelativePosition(*vec3);
	}
	static void GetPositionCall(uint64_t uuid, glm::vec3* out) {
		*out = Application->activeScene->transformComponents.at(uuid).relativePosition;
	}

	static void SetRotation(uint64_t uuid, glm::vec3* vec3) {
		Application->activeScene->transformComponents.at(uuid).SetRelativeRotation(glm::radians(*vec3));
	}
	static void GetRotationCall(uint64_t uuid, glm::vec3* out) {
		*out = glm::degrees(Application->activeScene->transformComponents.at(uuid).rotation);
	}

	static void MoveTowards(uint64_t uuid, glm::vec3 vector3) {
		auto transformIt = Application->activeScene->transformComponents.find(uuid);
		if (transformIt != Application->activeScene->transformComponents.end())
			transformIt->second.MoveTowards(vector3);

	}


#pragma endregion Transform Component

#pragma region Mesh Renderer Component
	static void MeshRenderer_GetVertices(uint64_t uuid, glm::vec3** out, int* size) {
		auto meshRendererIt = Application->activeScene->meshRendererComponents.find(uuid);
		if (meshRendererIt != Application->activeScene->meshRendererComponents.end()) {
			vector<glm::vec3> vertices = meshRendererIt->second.mesh.get()->vertices;
			*size = static_cast<int>(vertices.size());
			*out = vertices.data();
		}
		else {
			*size = 0;
			*out = nullptr;
		}
	}

	static void MeshRenderer_SetVertices(uint64_t uuid, glm::vec3* vertices, int size) {
		auto meshRendererIt = Application->activeScene->meshRendererComponents.find(uuid);
		if (meshRendererIt != Application->activeScene->meshRendererComponents.end()) {
			// Assuming you have a method to convert an array of glm::vec3 to your desired vector type.
			shared_ptr<Mesh> oldMesh = meshRendererIt->second.mesh;
			Mesh* newMesh = new Mesh(*oldMesh);
			newMesh->meshId = Plaza::UUID::NewUUID();
			newMesh->temporaryMesh = true;
			if (oldMesh->temporaryMesh) {
				newMesh->meshId = oldMesh->meshId;
				*Application->activeScene->meshes[newMesh->meshId].get() = *newMesh;
			}
			else {
				Application->activeScene->meshes.emplace(newMesh->meshId, make_shared<Mesh>(*newMesh));
			}
			Application->activeScene->meshRendererComponents.at(uuid).mesh = Application->activeScene->meshes.at(newMesh->meshId);
			vector<glm::vec3>& meshVertices = Application->activeScene->entities.at(uuid).GetComponent<MeshRenderer>()->mesh->vertices;
			meshVertices.clear();

			// Reserve space for the new vertices (optional but can improve performance)
			meshVertices.reserve(size);

			// Copy the provided vertices into the meshVertices vector
			for (int i = 0; i < size; ++i) {
				meshVertices.push_back(vertices[i]);
			}
			Application->activeScene->entities.at(uuid).GetComponent<MeshRenderer>()->mesh->Restart();
			delete newMesh;
		}
	}

	static void MeshRenderer_GetIndices(uint64_t uuid, unsigned int** out, int* size) {
		auto meshRendererIt = Application->activeScene->meshRendererComponents.find(uuid);
		if (meshRendererIt != Application->activeScene->meshRendererComponents.end()) {
			vector<unsigned int> indices = meshRendererIt->second.mesh.get()->indices;
			*size = static_cast<int>(indices.size());
			*out = indices.data();
		}
		else {
			*size = 0;
			*out = nullptr;
		}
	}

	static void MeshRenderer_SetIndices(uint64_t uuid, unsigned int* indices, int size) {
		auto meshRendererIt = Application->activeScene->meshRendererComponents.find(uuid);
		if (meshRendererIt != Application->activeScene->meshRendererComponents.end()) {
			// Assuming you have a method to convert an array of glm::vec3 to your desired vector type.
			shared_ptr<Mesh> oldMesh = meshRendererIt->second.mesh;
			Mesh* newMesh = new Mesh(*oldMesh);
			newMesh->meshId = Plaza::UUID::NewUUID();
			newMesh->temporaryMesh = true;
			if (oldMesh->temporaryMesh) {
				newMesh->meshId = oldMesh->meshId;
				*Application->activeScene->meshes[newMesh->meshId].get() = *newMesh;
			}
			else {
				Application->activeScene->meshes.emplace(newMesh->meshId, make_shared<Mesh>(*newMesh));
			}
			Application->activeScene->meshRendererComponents.at(uuid).mesh = Application->activeScene->meshes.at(newMesh->meshId);
			vector<unsigned int>& meshIndices = Application->activeScene->entities.at(uuid).GetComponent<MeshRenderer>()->mesh->indices;
			meshIndices.clear();

			// Reserve space for the new vertices (optional but can improve performance)
			meshIndices.reserve(size);

			// Copy the provided vertices into the meshVertices vector
			for (int i = 0; i < size; ++i) {
				meshIndices.push_back(indices[i]);
			}
			Application->activeScene->entities.at(uuid).GetComponent<MeshRenderer>()->mesh->Restart();
			delete newMesh;
		}
	}

	static void MeshRenderer_GetNormals(uint64_t uuid, glm::vec3** out, int* size) {
		auto meshRendererIt = Application->activeScene->meshRendererComponents.find(uuid);
		if (meshRendererIt != Application->activeScene->meshRendererComponents.end()) {
			vector<glm::vec3> vertices = meshRendererIt->second.mesh.get()->normals;
			*size = static_cast<int>(vertices.size());
			*out = vertices.data();
		}
		else {
			*size = 0;
			*out = nullptr;
		}
	}

	static void MeshRenderer_SetNormals(uint64_t uuid, glm::vec3* normals, int size) {
		auto meshRendererIt = Application->activeScene->meshRendererComponents.find(uuid);
		if (meshRendererIt != Application->activeScene->meshRendererComponents.end()) {
			// Assuming you have a method to convert an array of glm::vec3 to your desired vector type.
			shared_ptr<Mesh> oldMesh = meshRendererIt->second.mesh;
			Mesh* newMesh = new Mesh(*oldMesh);
			newMesh->meshId = Plaza::UUID::NewUUID();
			newMesh->temporaryMesh = true;
			if (oldMesh->temporaryMesh) {
				newMesh->meshId = oldMesh->meshId;
				*Application->activeScene->meshes[newMesh->meshId].get() = *newMesh;
			}
			else {
				Application->activeScene->meshes.emplace(newMesh->meshId, make_shared<Mesh>(*newMesh));
			}
			Application->activeScene->meshRendererComponents.at(uuid).mesh = Application->activeScene->meshes.at(newMesh->meshId);
			vector<glm::vec3>& meshNormals = Application->activeScene->entities.at(uuid).GetComponent<MeshRenderer>()->mesh->normals;
			meshNormals.clear();

			// Reserve space for the new vertices (optional but can improve performance)
			meshNormals.reserve(size);

			// Copy the provided vertices into the meshVertices vector
			for (int i = 0; i < size; ++i) {
				meshNormals.push_back(normals[i]);
			}
			Application->activeScene->entities.at(uuid).GetComponent<MeshRenderer>()->mesh->Restart();
			delete newMesh;
		}
	}

#pragma endregion Mesh Renderer Component

#pragma region RigidBody
	static void RigidBody_LockAngular(uint64_t uuid, Axis axis, bool value) {
		auto it = Application->activeScene->rigidBodyComponents.find(uuid);
		if (it != Application->activeScene->rigidBodyComponents.end()) {
			physx::PxRigidDynamicLockFlag::Enum flag;
			if (axis == Axis::X)
				flag = physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X;
			else if (axis == Axis::Y)
				flag = physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y;
			else if (axis == Axis::Z)
				flag = physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z;
			it->second.SetRigidDynamicLockFlags(flag, value);
		}
	}

	struct Angular {
		bool X, Y, Z;
	};

	static Angular* RigidBody_IsAngularLocked(uint64_t uuid) {
		Angular* angular = new Angular();
		auto it = Application->activeScene->rigidBodyComponents.find(uuid);
		if (it != Application->activeScene->rigidBodyComponents.end()) {
			if (it->second.rigidDynamicLockFlags.isSet(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X))
				angular->X = true;

			if (it->second.rigidDynamicLockFlags.isSet(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y))
				angular->Y = true;

			if (it->second.rigidDynamicLockFlags.isSet(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z))
				angular->Z = true;
		}
		return angular;
	}
#pragma endregion RigidBody

#pragma endregion Components

	void InternalCalls::Init() {
		//PL_ADD_INTERNAL_CALL(GetPositionCall);
		mono_add_internal_call("Plaza.InternalCalls::FindEntityByNameCall", FindEntityByNameCall);
		mono_add_internal_call("Plaza.InternalCalls::Instantiate", Instantiate);


		mono_add_internal_call("Plaza.InternalCalls::InputIsKeyDown", InputIsKeyDown);
		mono_add_internal_call("Plaza.InternalCalls::IsKeyReleased", IsKeyReleased);
		mono_add_internal_call("Plaza.InternalCalls::InputIsMouseDown", InputIsMouseDown);
		mono_add_internal_call("Plaza.InternalCalls::GetMouseDelta", GetMouseDelta);
		mono_add_internal_call("Plaza.InternalCalls::CursorHide", CursorHide);

		mono_add_internal_call("Plaza.InternalCalls::EntityGetParent", EntityGetParent);
		mono_add_internal_call("Plaza.InternalCalls::EntitySetParent", EntitySetParent);
		mono_add_internal_call("Plaza.InternalCalls::HasComponent", HasComponent);

		mono_add_internal_call("Plaza.InternalCalls::GetPositionCall", GetPositionCall);
		mono_add_internal_call("Plaza.InternalCalls::SetPosition", SetPosition);
		mono_add_internal_call("Plaza.InternalCalls::GetRotationCall", GetRotationCall);
		mono_add_internal_call("Plaza.InternalCalls::SetRotation", SetRotation);

		mono_add_internal_call("Plaza.InternalCalls::MoveTowards", MoveTowards);

		mono_add_internal_call("Plaza.InternalCalls::MeshRenderer_GetVertices", MeshRenderer_GetVertices);
		mono_add_internal_call("Plaza.InternalCalls::MeshRenderer_SetVertices", MeshRenderer_SetVertices);
		mono_add_internal_call("Plaza.InternalCalls::MeshRenderer_GetIndices", MeshRenderer_GetIndices);
		mono_add_internal_call("Plaza.InternalCalls::MeshRenderer_SetIndices", MeshRenderer_SetIndices);
		mono_add_internal_call("Plaza.InternalCalls::MeshRenderer_GetNormals", MeshRenderer_GetNormals);
		mono_add_internal_call("Plaza.InternalCalls::MeshRenderer_SetNormals", MeshRenderer_SetNormals);

		mono_add_internal_call("Plaza.InternalCalls::RigidBody_LockAngular", RigidBody_LockAngular);
		mono_add_internal_call("Plaza.InternalCalls::RigidBody_IsAngularLocked", RigidBody_IsAngularLocked);

		//mono_add_internal_call("Plaza.InternalCalls::MeshRenderer_GetUvs", MeshRenderer_GetUvs);
		//mono_add_internal_call("Plaza.InternalCalls::MeshRenderer_SetUvs", MeshRenderer_SetUvs);
		//mono_add_internal_call("Plaza.InternalCalls::MeshRenderer_GetIndices", MeshRenderer_GetIndices);
		//mono_add_internal_call("Plaza.InternalCalls::MeshRenderer_SetIndices", MeshRenderer_SetIndices);
		//mono_add_internal_call("Plaza.InternalCalls::MeshRenderer_GetNormals", MeshRenderer_GetNormals);
		//mono_add_internal_call("Plaza.InternalCalls::MeshRenderer_SetNormals", MeshRenderer_SetNormals);
	}

}