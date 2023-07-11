#pragma  once

#include <iostream>
#include <random>
#include <filesystem>
#include <typeinfo>
#include <list>
#include <string>
#include "Engine/Vendor/glm/glm.hpp"
#include "Engine/Vendor/glm/gtc/matrix_transform.hpp"
#include "Engine/Vendor/glm/gtc/type_ptr.hpp"
#include "Engine/Components/Core/Mesh.h"
#include "Engine/Vendor/uuid_v4/uuid_v4.h"
#include "Engine/Utils/glmUtils.h"
#include "Engine/Components/Core/Component.h"
#include "Engine/Components/Core/Transform.h"
#include <random>
#include <unordered_map>
//UUIDv4::UUID uuid = uuidGenerator.getUUID();

class Transform;
class GameObject;

class GameObjectList : public std::vector<GameObject*> {
public:
	void push_back(GameObject* obj);
	GameObject* find(std::string findName);
};

extern GameObjectList gameObjects;

extern std::unordered_map<std::string, GameObject*> gameObjectsMap;
extern GameObject* sceneObject;

class GameObject {
public:
	std::vector<GameObject*> children;
	GameObject* parent = nullptr;
	Transform* transform = nullptr;// = new Transform();
	std::string name = "";
	int id;

	GameObject(std::string objName, GameObject* parent = sceneObject);

	std::vector<Component*> components;
	template<typename T>
	T* AddComponent(T* component) {
		components.push_back(component);
		component->gameObject = this;
		return component;
	}

	template<typename T>
	T* GetComponent() {
		for (Component* component : components) {
			if (typeid(*component) == typeid(T)) {
				return static_cast<T*>(component);
			}
		}
		return nullptr;
	}
};

//extern std::list<GameObject*> gameObjects;

class MeshRenderer;
extern std::vector<MeshRenderer*> meshRenderers;
class MeshRenderer : public Component {
public:
	Engine::Mesh* mesh;
	Transform* transform;
	MeshRenderer(Engine::Mesh* initialMesh) : mesh(initialMesh) {
		this->mesh = new Engine::Mesh(initialMesh->vertices, initialMesh->indices, initialMesh->material);
		meshRenderers.emplace_back(this);
	}
	~MeshRenderer() = default;
};
