#include "Engine/Core/PreCompiledHeaders.h"
#include "ColliderSerializer.h"
#include "Engine/Core/Physics.h"
namespace Plaza {
	void ComponentSerializer::ColliderSerializer::Serialize(YAML::Emitter& out, Collider& collider) {
		out << YAML::Key << "ColliderComponent" << YAML::BeginMap;

		out << YAML::Key << "Uuid" << YAML::Value << collider.mUuid;
		out << YAML::Key << "Shapes" << YAML::Value << YAML::BeginSeq;
		if (collider.mShapes.size() == 0) {
			std::cout << "000qweqewq \n";
		}
		vector<ColliderShape*> mShapes = collider.mShapes;
		for (ColliderShape* shape : mShapes) {
			out << YAML::BeginMap;
			out << YAML::Key << "Shape" << YAML::Value << shape->mEnum;
			out << YAML::Key << "MeshUuid" << YAML::Value << shape->mMeshUuid;
			out << YAML::EndMap;
		}
		out << YAML::EndSeq; // Shapes

		out << YAML::EndMap; // Collider Component
	}

	Collider* ComponentSerializer::ColliderSerializer::DeSerialize(YAML::Node data) {
		Collider* collider = new Collider(data["Uuid"].as<uint64_t>(), nullptr);
		Transform* transform = &Application->activeScene->transformComponents.at(collider->mUuid);
		for (auto shapeDeserialized : data["Shapes"]) {
			if (AssetsManager::HasMesh(shapeDeserialized["MeshUuid"].as<uint64_t>()) || shapeDeserialized["Shape"].as<int>() != ColliderShape::ColliderShapeEnum::MESH) {
				if (shapeDeserialized["MeshUuid"].as<uint64_t>())
					collider->CreateShape(ColliderShape::ColliderShapeEnum(shapeDeserialized["Shape"].as<int>()), transform, AssetsManager::GetMesh(shapeDeserialized["MeshUuid"].as<uint64_t>()));
				else
					collider->CreateShape(ColliderShape::ColliderShapeEnum(shapeDeserialized["Shape"].as<int>()), transform, 0);
			}
		}
		return collider;
	}
}