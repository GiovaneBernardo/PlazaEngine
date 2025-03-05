#pragma once
#include <variant>
// #include "Engine/Core/Scene.h"
#include "Engine/Components/Component.h"
#include <vector>
#include <string>
#include "Engine/Application/Application.h"
#include "Engine/Core/ComponentMapStructure.h"
#include "Engine/Core/Engine.h"
#include <bitset>

// #include "Engine/Core/Scene.h"

namespace cereal {
	template <class Archive, size_t N> void save(Archive& archive, const std::bitset<N>& bits) {
		std::string bitString = bits.to_string();
		archive(bitString);
	}

	template <class Archive, size_t N> void load(Archive& archive, std::bitset<N>& bits) {
		std::string bitString;
		archive(bitString);
		bits = std::bitset<N>(bitString);
	}
} // namespace cereal
namespace Plaza {
#define MAX_COMPONENTS 128
	typedef std::bitset<MAX_COMPONENTS> ComponentMask;
	class Scene;
	struct PLAZA_API Entity {
	  public:
		uint64_t uuid = 0;
		ComponentMask mComponentMask;
		uint64_t parentUuid = 0;
		uint64_t prefabUuid = 0;		   // The uuid of the prefab it belongs to
		uint64_t equivalentPrefabUuid = 0; // The uuid of the entity in the prefab it belongs to
		bool isPrefab = false;
		std::vector<uint64_t> childrenUuid;
		std::string name;
		bool changingName = false;

		Entity() {}
		Entity(std::string objName, Entity* parent = nullptr, uint64_t newUuid = 0) : name(objName), uuid(newUuid) {}
		Entity(const Entity&) = default;

		void Rename(std::string newName) {}

		template <class Archive> void serialize(Archive& archive) {
			archive(PL_SER(uuid), PL_SER(parentUuid), PL_SER(prefabUuid), PL_SER(equivalentPrefabUuid),
					PL_SER(childrenUuid), PL_SER(name), PL_SER(mComponentMask));
		}

	  private:
	};

} // namespace Plaza

/*
#include <iostream>
#include <random>
#include <filesystem>
#include <typeinfo>
#include <list>
#include <string>
#include "ThirdParty/glm/glm.hpp"
#include "ThirdParty/glm/gtc/matrix_transform.hpp"
#include "ThirdParty/glm/gtc/type_ptr.hpp"
#include "ThirdParty/uuid_v4/uuid_v4.h"
#include "Engine/Utils/glmUtils.h"
#include "Engine/Components/Component.h"
#include "Engine/Components/Core/Transform.h"
#include <random>
#include <unordered_map>
#include "Engine/Components/Rendering/Mesh.h"
//UUIDv4::UUID uuid = uuidGenerator.getUUID();
#include "Engine/Components/Component.h"

namespace Plaza {
	class Transform;
	class Entity;


	extern Entity* sceneObject;
	class Entity {
	public:
		std::vector<Entity*> children;
		Entity* parent = nullptr;
		Transform* transform = nullptr;// = new Transform();
		std::string name = "";
		int id;
		uint64_t uuid;
		uint64_t modelUuid = 0;
		std::string scene = "";

		void DeleteChildren(Entity* entity) {
			for (Entity* entity : entity->children) {
				DeleteChildren(entity);
			}

			delete(entity);
		}

		Entity(std::string objName, Entity* parent = sceneObject, bool addToScene = true);
		Entity(const Entity&) = default;
		~Entity() = default;
		void Delete();




		vector<shared_ptr<Component>> components;
		template<typename T>
		T* AddComponent(T* component, bool addToComponentsList = true);
		Component* AddComponent(Component* component, bool addToComponentsList = true);

		Component* cachedComponent = nullptr;
		template<typename T>
		T* GetComponent() {
			for (shared_ptr<Component> component : components) {
				if (typeid(*component) == typeid(T)) {
					return static_cast<T*>(component.get());
				}
			}
			return nullptr;
		}

		template<typename T>
		T* ReplaceComponent(T* oldComponent, T* newComponent) {
			for (size_t i = 0; i < components.size(); i++) {
				if (components[i].get() == oldComponent) {
					components.erase(components.begin() + i);
					newComponent->uuid = this->uuid;
					components.push_back(shared_ptr<Component>(newComponent));
					return newComponent;
				}
			}
			return nullptr; // Old component not found in the list
		}

		template <typename T>
		void RemoveComponent() {
			static_assert(std::is_base_of<Component, T>::value, "T must be a subclass of Component");

			auto it = std::remove_if(this->components.begin(), this->components.end(),
				[](const std::shared_ptr<Component>& comp) {
					return dynamic_cast<T*>(comp.get()) != nullptr;
				});

			this->components.erase(it, this->components.end());
		}
	};
}
//extern std::list<Entity*> gameObjects;

*/
