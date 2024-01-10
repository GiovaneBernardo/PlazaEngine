#include "Engine/Core/PreCompiledHeaders.h"
#include "ModelSerializer.h"
#include "Engine/Application/Serializer/Components/MaterialSerializer.h"
#include "Engine/Application/Serializer/Components/TransformSerializer.h"
#include "Engine/Application/Serializer/Components/MeshRendererSerializer.h"

namespace Plaza {
	void SerializeGameObject(YAML::Emitter& out, Entity* entity) {
		//out << YAML::BeginMap;
		out << YAML::BeginMap;
		out << YAML::Key << "Entity" << entity->uuid;
		out << YAML::Key << "Uuid" << YAML::Value << entity->uuid;
		out << YAML::Key << "Name" << YAML::Value << entity->name;
		out << YAML::Key << "ParentID" << YAML::Value << (entity->parentUuid != 0 ? entity->parentUuid : 0);
		out << YAML::Key << "Components" << YAML::Value << YAML::BeginMap;
		if (entity->GetComponent<Transform>()) {
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap;
			glm::vec3& relativePosition = entity->GetComponent<Transform>()->relativePosition;
			out << YAML::Key << "Position" << YAML::Value << relativePosition;
			glm::vec3 relativeRotation = glm::eulerAngles(entity->GetComponent<Transform>()->rotation);
			out << YAML::Key << "Rotation" << YAML::Value << relativeRotation;
			glm::vec3& scale = entity->GetComponent<Transform>()->scale;
			out << YAML::Key << "Scale" << YAML::Value << scale;

			out << YAML::EndMap;
		}
		MeshRenderer* meshRenderer = entity->GetComponent<MeshRenderer>();
		if (meshRenderer) {
			if (meshRenderer->mesh) {
				out << YAML::Key << "MeshComponent" << YAML::Value << YAML::BeginMap;
				out << YAML::Key << "AiMeshName" << YAML::Value << meshRenderer->aiMeshName;
				out << YAML::Key << "MeshName" << YAML::Value << entity->name;
				out << YAML::Key << "MeshUUID" << YAML::Value << meshRenderer->mesh->uuid;
				out << YAML::Key << "MaterialUuid" << YAML::Value << meshRenderer->material->uuid;
				out << YAML::Key << "MaterialPath" << YAML::Value << meshRenderer->material->filePath;
				ComponentSerializer::MaterialSerializer::Serialize(out, *meshRenderer->material);
				out << YAML::EndMap;
			}
		}
		out << YAML::EndMap;
		out << YAML::EndMap;
	}

	void ModelSerializer::SerializeModel(Entity* mainObject, string filePath, string modelFilePath) {

		uint64_t modelUuid = Plaza::UUID::NewUUID();
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Model" << YAML::Value << mainObject->name;
		out << YAML::Key << "ModelFilePath" << YAML::Value << modelFilePath;
		out << YAML::Key << "ModelUuid" << YAML::Value << modelUuid;
		out << YAML::Key << "ModelScale" << YAML::Value << 1.0f;
		out << YAML::Key << "UseTangent" << YAML::Value << true;
		out << YAML::Key << "MainObject" << YAML::Value << YAML::BeginSeq;
		SerializeGameObject(out, mainObject);
		out << YAML::EndSeq;
		out << YAML::Key << "GameObjects" << YAML::Value << YAML::BeginSeq;
		for (uint64_t childUuuid : mainObject->childrenUuid) {
			SerializeGameObject(out, &Application->activeScene->entities[childUuuid]);
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;
		std::ofstream fout(filePath);
		fout << out.c_str();

	}

	void DeSerializeTexture(Material& material, const auto& textureNode) {
		if (textureNode) {
			material.diffuse.path = textureNode["Path"].as<string>();
			material.diffuse.rgba = textureNode["Rgba"].as<glm::vec4>();
			//glm::vec4& rgba = material.diffuse.rgba;
			//rgba.x = textureNode["Rgba"][0].as<float>();
			//rgba.y = textureNode["Rgba"][1].as<float>();
			//rgba.z = textureNode["Rgba"][2].as<float>();
			//rgba.a = textureNode["Rgba"][3].as<float>();
		}
	}

	void DeSerializeMaterial(const auto& materialNode, Model* model, MeshRenderer* meshRenderer) {
		if (meshRenderer->mesh != nullptr) {
			Material* material = &meshRenderer->mesh->material;
			material->shininess = materialNode["Shininess"].as<float>();
			const auto& textureDiffuseNode = materialNode["texture_diffuse"];
			DeSerializeTexture(*material, textureDiffuseNode);
			const auto& textureSpecularNode = materialNode["texture_specular"];
			DeSerializeTexture(*material, textureSpecularNode);
			const auto& textureNormalNode = materialNode["texture_normal"];
			DeSerializeTexture(*material, textureNormalNode);
			const auto& textureHeightNode = materialNode["texture_height"];
			DeSerializeTexture(*material, textureHeightNode);
		}
	}
	void DeSerializeGameObject(const auto& gameObjectEntry, Model* model) {

		const auto& componentsEntry = gameObjectEntry["Components"];
		Entity* entity = new Entity(gameObjectEntry["Name"].as<string>(), nullptr, false);
		entity->uuid = gameObjectEntry["Uuid"].as<uint64_t>();
		uint64_t parentUuid = gameObjectEntry["ParentID"].as<uint64_t>();
		for (const auto& modelGameObject : model->gameObjects) {
			if (parentUuid == modelGameObject.get()->uuid) {
				entity->parentUuid = modelGameObject.get()->uuid;
			}
		}
		if (entity->parentUuid == 0) {
			entity->parentUuid = Application->activeScene->mainSceneEntity->uuid;
		}
		Transform* newTransform = ComponentSerializer::TransformSerializer::DeSerialize(componentsEntry["TransformComponent"]);
		entity->ReplaceComponent<Transform>(newTransform);
		//entity->RemoveComponent<Transform>();
		//entity->AddComponent<Transform>(entity->transform);
		if (componentsEntry["MeshComponent"]) {
			MeshRenderer* oldMeshRenderer = entity->GetComponent<MeshRenderer>();
			MeshRenderer* newMeshRenderer = new MeshRenderer();
			newMeshRenderer->mesh = new Mesh();
			newMeshRenderer->instanced = true;
			newMeshRenderer->aiMeshName = componentsEntry["MeshComponent"]["AiMeshName"].as<string>();
			DeSerializeMaterial(componentsEntry["MeshComponent"]["MaterialComponent"], model, newMeshRenderer);
			newMeshRenderer->uuid = entity->uuid;
			uint64_t materialUuid = componentsEntry["MeshComponent"]["MaterialUuid"].as<uint64_t>();
			if (componentsEntry["MeshComponent"]["MaterialUuid"])
				newMeshRenderer->material = Application->activeScene->materials.at(componentsEntry["MeshComponent"]["MaterialUuid"].as<uint64_t>()).get();
			model->meshRenderers.emplace(entity->uuid, newMeshRenderer);
		}
		model->transforms.emplace(entity->uuid, newTransform);
		model->gameObjects.push_back(make_shared<Entity>(*entity));

	}


	Model* ModelSerializer::DeSerializeModel(string filePath) {
		Model* model = new Model();
		if (std::filesystem::exists(filePath)) {
			std::ifstream stream(filePath);
			std::stringstream strStream;
			strStream << stream.rdbuf();
			YAML::Node data = YAML::Load(strStream.str());
			model->modelName = data["Model"].as<string>();
			model->uuid = data["ModelUuid"].as<uint64_t>();
			model->scale = data["ModelScale"].as<float>();
			model->useTangent = data["UseTangent"].as<bool>();
			model->modelObjectPath = data["ModelFilePath"].as<string>();
			model->modelPlazaPath = filePath;
			// DeSerialize the model's main object
			DeSerializeGameObject(data["MainObject"][0], model);

			// DeSerialize the model's GameObjects
			const YAML::Node& gameObjects = data["GameObjects"];
			for (const auto& gameObjectEntry : gameObjects) {
				DeSerializeGameObject(gameObjectEntry, model);
			}
		}
		return model;
	}


	uint64_t ModelSerializer::ReadUUID(string filePath) {
		std::ifstream stream(filePath);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());
		return data["ModelUuid"].as<uint64_t>();
	}
}