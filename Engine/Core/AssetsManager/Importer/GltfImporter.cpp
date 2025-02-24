#include "Engine/Core/PreCompiledHeaders.h"
#include "AssetsImporter.h"
#include "ThirdParty/glm/gtx/hash.hpp"
#include "Engine/Core/Renderer/Mesh.h"
//#include "ThirdParty/OpenFBX/src/ofbx.h"


#ifdef WIN32
#include <d3d11.h>
#include <tchar.h>
#endif
#include <stdio.h>
#include <inttypes.h>
#include <vector>

struct Vec3Hash {
	std::size_t operator()(const glm::vec3& v) const {
		// Use glm's hash function for vec3
		return std::hash<glm::vec3>()(v);
	}
};


namespace Plaza {

	Entity* AssetsImporter::ImportGLTF(AssetImported asset, std::filesystem::path outPath, AssetsImporterSettings settings) {
		return nullptr;
		//FILE* fileOpen = fopen(asset.mPath.c_str(), "rb");

		//if (!fileOpen) return nullptr;

		//fseek(fileOpen, 0, SEEK_END);
		//long fileSize = ftell(fileOpen);
		//fseek(fileOpen, 0, SEEK_SET);
		//auto* content = new ofbx::u8[fileSize];
		//fread(content, 1, fileSize, fileOpen);

		//ofbx::LoadFlags flags =
		//	//		ofbx::LoadFlags::IGNORE_MODELS |
		//	ofbx::LoadFlags::IGNORE_BLEND_SHAPES |
		//	ofbx::LoadFlags::IGNORE_CAMERAS |
		//	ofbx::LoadFlags::IGNORE_LIGHTS |
		//	//		ofbx::LoadFlags::IGNORE_TEXTURES |
		//	ofbx::LoadFlags::IGNORE_SKIN |
		//	ofbx::LoadFlags::IGNORE_BONES |
		//	ofbx::LoadFlags::IGNORE_PIVOTS |
		//	//		ofbx::LoadFlags::IGNORE_MATERIALS |
		//	ofbx::LoadFlags::IGNORE_POSES |
		//	ofbx::LoadFlags::IGNORE_VIDEOS |
		//	ofbx::LoadFlags::IGNORE_LIMBS |
		//	//		ofbx::LoadFlags::IGNORE_MESHES |
		//	ofbx::LoadFlags::IGNORE_ANIMATIONS;

		//ofbx::IScene* loadedScene = ofbx::load((ofbx::u8*)content, fileSize, (ofbx::u16)flags);
		//delete[] content;
		//fclose(fileOpen);
		//FILE* fp = fopen(asset.mPath.c_str(), "wb");
		//if (!fp) return nullptr;
		//int indicesOffset = 0;
		//int meshCount = loadedScene->getMeshCount();

		//// output unindexed geometry
		//Entity* mainEntity = nullptr;
		//std::unordered_map<ofbx::u64, uint64_t> meshIndexEntityMap = std::unordered_map<ofbx::u64, uint64_t>();
		//for (int meshIndex = 0; meshIndex < meshCount; ++meshIndex) {
		//	const ofbx::Mesh& mesh = *loadedScene->getMesh(meshIndex);
		//	Entity* entity;
		//	if (!mainEntity)
		//	{
		//		entity = new Entity(mesh.name, Scene::GetActiveScene()->mainSceneEntity);
		//		mainEntity = entity;
		//	}else
		//		entity = new Entity(mesh.name, mainEntity);

		//	meshIndexEntityMap.emplace(mesh.id, entity->uuid);
		//	ofbx::Object* parent = mesh.getParent();
		//	if (parent) {
		//		auto parentIt = meshIndexEntityMap.find(parent->id);
		//		if (parentIt != meshIndexEntityMap.end()) {
		//			entity->ChangeParent(Scene::GetActiveScene()->GetEntity(entity->GetParent().uuid), Scene::GetActiveScene()->GetEntity(parentIt->second));
		//		}
		//	}


		//	const ofbx::GeometryData& geom = mesh.getGeometryData();
		//	const ofbx::Vec3Attributes positions = geom.getPositions();
		//	const ofbx::Vec3Attributes normals = geom.getNormals();
		//	const ofbx::Vec2Attributes uvs = geom.getUVs();


		//	Mesh* finalMesh = new Mesh();
		//	finalMesh->vertices.resize(positions.count);
		//	for (unsigned int i = 0; i < positions.count; ++i) {
		//		ofbx::Vec3 vector = positions.get(i);
		//		finalMesh->vertices.push_back(glm::vec3(vector.x, vector.y, vector.z));
		//	}
		//	finalMesh->normals.resize(normals.count);
		//	for (unsigned int i = 0; i < normals.count; ++i) {
		//		ofbx::Vec3 vector = normals.get(i);
		//		finalMesh->normals.push_back(glm::vec3(vector.x, vector.y, vector.z));
		//	}
		//	finalMesh->uvs.resize(uvs.count);
		//	for (unsigned int i = 0; i < uvs.count; ++i) {
		//		ofbx::Vec2 vector = uvs.get(i);
		//		finalMesh->uvs.push_back(glm::vec2(vector.x, vector.y));
		//	}

		//	/* Indices */
		//			// Each mesh may have several partitions (materials)
		//	for (int partitionIdx = 0; partitionIdx < geom.getPartitionCount(); ++partitionIdx) {
		//		const ofbx::GeometryPartition& partition = geom.getPartition(partitionIdx);

		//		// Iterate over each polygon in the partition
		//		for (int polygonIdx = 0; polygonIdx < partition.polygon_count; ++polygonIdx) {
		//			const ofbx::GeometryPartition::Polygon& polygon = partition.polygons[polygonIdx];

		//			// Iterate over each vertex in the polygon
		//			for (int i = polygon.from_vertex; i < polygon.from_vertex + polygon.vertex_count; ++i) {
		//				// Add the vertex index to the indices vector
		//				finalMesh->indices.push_back(i + indicesOffset);
		//			}
		//		}

		//		// Update the indices offset for the next partition
		//		indicesOffset += geom.getPositions().count;
		//	}

		//	MeshRenderer* meshRenderer = new MeshRenderer(finalMesh, AssetsManager::GetDefaultMaterial());
		//	entity->AddComponent<MeshRenderer>(meshRenderer);
		//}
		//fclose(fp);

		//return mainEntity;
		// 
		// 











		//Assimp::Importer importer;
		//const aiScene* scene = importer.ReadFile(asset.mPath,
		//	aiProcess_JoinIdenticalVertices 
		//);
		//if (nullptr == scene) {
		//	cout << "ERROR::ASSIMP::" << importer.GetErrorString() << endl;
		//	return nullptr;
		//}
		//
		//return Scene::GetActiveScene()->GetEntity(AssetsImporter::ProcessNode(scene->mRootNode, scene, Scene::GetActiveScene()->GetEntity(Scene::GetActiveScene()->mainSceneEntity->uuid))->uuid);


		//if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, asset.mPath.c_str())) {
		//	throw std::runtime_error(warn + err);
		//}
		//Entity* mainEntity = nullptr; //= new Entity();
		//
		//const std::vector<float>& positions = attrib.vertices;
		//for (const auto& shape : shapes) {
		//	Entity* newEntity;
		//	if (!mainEntity)
		//	{
		//		newEntity = new Entity(shape.name, Scene::GetActiveScene()->mainSceneEntity);
		//		mainEntity = newEntity;
		//	}
		//	else
		//		newEntity = new Entity(shape.name, mainEntity, true);
		//
		//	std::vector<glm::vec3> vertices{};
		//	std::vector<glm::vec3> normals{};
		//	std::vector<glm::vec2> uvs{};
		//	std::vector<unsigned int> indices{};
		//
		//	std::unordered_map<glm::vec3, uint32_t> uniqueVertices = std::unordered_map<glm::vec3, uint32_t>();
		//	for (size_t v = 0; v < shape.mesh.indices.size(); ++v) {
		//		tinyobj::index_t index = shape.mesh.indices[v];
		//		glm::vec3 vertex = glm::vec3(
		//			attrib.vertices[3 * index.vertex_index + 0],
		//			attrib.vertices[3 * index.vertex_index + 1],
		//			attrib.vertices[3 * index.vertex_index + 2]
		//		) * mModelImporterScale;
		//
		//		bool isVertexUnique = uniqueVertices.count(vertex) == 0;
		//		if (isVertexUnique) {
		//			uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
		//
		//			if (attrib.vertices.size() > 0) {
		//				vertices.push_back(vertex);
		//			}
		//			if (attrib.normals.size() > 0) {
		//				normals.push_back(glm::vec3(
		//					attrib.normals[3 * index.normal_index + 0],
		//					attrib.normals[3 * index.normal_index + 1],
		//					attrib.normals[3 * index.normal_index + 2]
		//				));
		//			}
		//			if (attrib.texcoords.size() > 0) {
		//				uvs.push_back(glm::vec2(
		//					attrib.texcoords[2 * index.texcoord_index + 0],
		//					attrib.texcoords[2 * index.texcoord_index + 1]
		//				));
		//			}
		//		}
		//		indices.push_back(uniqueVertices[vertex]);
		//	}
		//
		//	Mesh& mesh = Application::Get()->mRenderer->CreateNewMesh(vertices, normals, uvs, std::vector<glm::vec3>(), std::vector<glm::vec3>(), indices, *AssetsManager::GetDefaultMaterial(), false);// new Mesh();
		//	MeshRenderer* meshRenderer = new MeshRenderer(mesh, AssetsManager::GetDefaultMaterial());
		//	newEntity->AddComponent<MeshRenderer>(meshRenderer);
		//}
		//AssetsSerializer::SerializePrefab(mainEntity, asset.mPath);
		//return Scene::GetActiveScene()->GetEntity(mainEntity->uuid);
	}
}