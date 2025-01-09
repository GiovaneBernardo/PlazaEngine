#include "Engine/Core/PreCompiledHeaders.h"
#include "ComponentsConverter.h"

namespace Plaza {
	SerializableMesh ComponentsConverter::ConvertMesh(Mesh* mesh) {
		SerializableMesh  serializedMesh{};
		serializedMesh.assetUuid = mesh->uuid; //Plaza::UUID::NewUUID();
		serializedMesh.assetType = AssetType::MESH;
		serializedMesh.verticesCount = mesh->vertices.size();
		serializedMesh.vertices = mesh->vertices;
		serializedMesh.normalsCount = mesh->normals.size();
		serializedMesh.normals = mesh->normals;
		serializedMesh.tangentCount = mesh->tangent.size();
		serializedMesh.tangent = mesh->tangent;
		serializedMesh.uvsCount = mesh->uvs.size();
		serializedMesh.uvs = mesh->uvs;
		serializedMesh.indicesCount = mesh->indices.size();
		serializedMesh.indices = mesh->indices;
		serializedMesh.materialsIndices = mesh->materialsIndices;
		serializedMesh.bonesHoldersCount = mesh->bonesHolder.size();
		serializedMesh.bonesHolders = mesh->bonesHolder;
		serializedMesh.uniqueBonesCount = mesh->uniqueBonesInfo.size();
		serializedMesh.uniqueBones.resize(mesh->uniqueBonesInfo.size());
		for (const auto& [key, value] : mesh->uniqueBonesInfo) {
			serializedMesh.uniqueBones.push_back(value);
		}
		return serializedMesh;
	}
}