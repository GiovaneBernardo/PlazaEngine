#include "AssetsImporter.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "ThirdParty/tinyobjloader/tiny_obj_loader.h"
#include "ThirdParty/glm/gtx/hash.hpp"
#include "Engine/Core/Renderer/Mesh.h"
#include "Engine/Core/AssetsManager/Serializer/AssetsSerializer.h"
#include "Engine/Core/AssetsManager/Loader/AssetsLoader.h"
#include "Editor/GUI/FileExplorer/FileExplorer.h"
#include "Editor/GUI/Utils/Filesystem.h"
#include "Engine/Components/Core/Prefab.h"
#include "Engine/Core/Renderer/Model.h"
#include "Engine/Core/Scene.h"

struct Vec3Hash {
	std::size_t operator()(const glm::vec3& v) const { return std::hash<glm::vec3>()(v); }
};

namespace Plaza {
	Material* ObjModelMaterialLoader(const tinyobj::material_t* tinyobjMaterial, const std::string materialFolderPath,
									 std::unordered_map<std::string, uint64_t>& loadedTextures) {
		Material* material = new Material();
		material->mAssetName = tinyobjMaterial->name;

		const std::string diffusePath = materialFolderPath + "/" + tinyobjMaterial->diffuse_texname;
		if (!tinyobjMaterial->diffuse_texname.empty() && loadedTextures.find(diffusePath) == loadedTextures.end()) {
			material->diffuse = std::shared_ptr<Texture>(
				AssetsLoader::LoadTexture(AssetsManager::GetAssetOrImport(diffusePath, Plaza::UUID::NewUUID())));
			loadedTextures.emplace(diffusePath, material->diffuse->mAssetUuid);
		}
		else if (!tinyobjMaterial->diffuse_texname.empty())
			material->diffuse = std::shared_ptr<Texture>(AssetsManager::mTextures.at(loadedTextures.at(diffusePath)));

		const std::string normalPath = materialFolderPath + "/" + tinyobjMaterial->normal_texname;
		if (!tinyobjMaterial->normal_texname.empty() && loadedTextures.find(normalPath) == loadedTextures.end()) {
			material->normal = std::shared_ptr<Texture>(
				AssetsLoader::LoadTexture(AssetsManager::GetAssetOrImport(normalPath, Plaza::UUID::NewUUID())));
			loadedTextures.emplace(normalPath, material->normal->mAssetUuid);
		}
		else if (!tinyobjMaterial->normal_texname.empty())
			material->normal = std::shared_ptr<Texture>(AssetsManager::mTextures.at(loadedTextures.at(normalPath)));

		const std::string roughnessPath = materialFolderPath + "/" + tinyobjMaterial->roughness_texname;
		if (!tinyobjMaterial->roughness_texname.empty() && loadedTextures.find(roughnessPath) == loadedTextures.end()) {
			material->roughness = std::shared_ptr<Texture>(
				AssetsLoader::LoadTexture(AssetsManager::GetAssetOrImport(roughnessPath, Plaza::UUID::NewUUID())));
			loadedTextures.emplace(roughnessPath, material->roughness->mAssetUuid);
		}
		else if (!tinyobjMaterial->roughness_texname.empty())
			material->roughness =
				std::shared_ptr<Texture>(AssetsManager::mTextures.at(loadedTextures.at(roughnessPath)));

		const std::string metalnessPath = materialFolderPath + "/" + tinyobjMaterial->metallic_texname;
		if (!tinyobjMaterial->metallic_texname.empty() && loadedTextures.find(metalnessPath) == loadedTextures.end()) {
			material->metalness = std::shared_ptr<Texture>(
				AssetsLoader::LoadTexture(AssetsManager::GetAssetOrImport(metalnessPath, Plaza::UUID::NewUUID())));
			loadedTextures.emplace(metalnessPath, material->metalness->mAssetUuid);
		}
		else if (!tinyobjMaterial->metallic_texname.empty())
			material->metalness =
				std::shared_ptr<Texture>(AssetsManager::mTextures.at(loadedTextures.at(metalnessPath)));

		//		material->shininess = tinyobjMaterial->shininess;
		material->diffuse->rgba.x = tinyobjMaterial->diffuse[0];
		material->diffuse->rgba.y = tinyobjMaterial->diffuse[1];
		material->diffuse->rgba.z = tinyobjMaterial->diffuse[2];

		return material;
	}

	std::shared_ptr<Scene> AssetsImporter::ImportOBJ(AssetImported asset, std::filesystem::path outPath, Model& model,
													 const AssetsImporterSettings& settings) {
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		const std::string parentPath = std::filesystem::path{asset.mPath}.parent_path().string();

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, asset.mPath.c_str(), parentPath.c_str(),
							  true)) {
			throw std::runtime_error(warn + err);
		}

		Entity* mainEntity = nullptr;
		std::shared_ptr<Scene> modelScene = std::make_shared<Scene>();
		modelScene->InitMainEntity();

		std::unordered_map<std::string, uint64_t> loadedTextures = std::unordered_map<std::string, uint64_t>();
		std::unordered_map<std::filesystem::path, uint64_t> loadedMaterials =
			std::unordered_map<std::filesystem::path, uint64_t>();
		std::unordered_map<size_t, std::shared_ptr<Mesh>> uniqueMeshes =
			std::unordered_map<uint64_t, std::shared_ptr<Mesh>>();

		unsigned int index = 0;
		const std::vector<float>& positions = attrib.vertices;
		for (const auto& shape : shapes) {
			if (index == 0)
				model.mAssetName = shape.name;
			Entity* newEntity;
			if (!mainEntity) {
				newEntity = modelScene->NewEntity(shape.name, modelScene->mainSceneEntity);
				mainEntity = newEntity;
			}
			else
				newEntity = modelScene->NewEntity(shape.name, mainEntity);

			std::vector<glm::vec3> vertices{};
			std::vector<glm::vec3> normals{};
			std::vector<glm::vec3> tangents{};
			std::vector<glm::vec2> uvs{};
			std::vector<unsigned int> indices{};

			std::size_t meshHash = 0;

			std::unordered_map<int, uint32_t> uniqueVertices = std::unordered_map<int, uint32_t>();
			for (size_t v = 0; v < shape.mesh.indices.size(); ++v) {
				tinyobj::index_t index = shape.mesh.indices[v];
				glm::vec3 vertex =
					glm::vec3(attrib.vertices[3 * index.vertex_index + 0], attrib.vertices[3 * index.vertex_index + 1],
							  attrib.vertices[3 * index.vertex_index + 2]) *
					mModelImporterScale;
				meshHash = CombineHashes(meshHash, std::hash<glm::vec3>()(vertex));

				std::size_t vertexHash = 0;
				vertexHash = CombineHashes(vertexHash, std::hash<size_t>()(index.vertex_index));
				vertexHash = CombineHashes(vertexHash, std::hash<size_t>()(index.texcoord_index));
				vertexHash = CombineHashes(vertexHash, std::hash<size_t>()(index.normal_index));
				bool isVertexUnique = uniqueVertices.count(vertexHash) == 0;
				if (isVertexUnique) {
					uniqueVertices[vertexHash] = static_cast<uint32_t>(vertices.size());

					if (attrib.vertices.size() > 0) {
						vertices.push_back(vertex);
					}
					if (attrib.normals.size() > 0) {
						normals.push_back(glm::vec3(attrib.normals[3 * index.normal_index + 0],
													attrib.normals[3 * index.normal_index + 1],
													attrib.normals[3 * index.normal_index + 2]));
					}
					if (attrib.texcoords.size() > 0) {
						uvs.push_back(glm::vec2(attrib.texcoords[2 * max(index.texcoord_index, 0) + 0],
												attrib.texcoords[2 * max(index.texcoord_index, 0) + 1]));
					}
				}
				indices.push_back(uniqueVertices[vertexHash]);
				meshHash = CombineHashes(meshHash, std::hash<unsigned int>()(uniqueVertices[vertexHash]));
			}
			if (vertices.size() > 0) {
				meshHash = CombineHashes(meshHash, std::hash<size_t>()(vertices.size()));
				meshHash = CombineHashes(meshHash, std::hash<size_t>()(normals.size()));
				meshHash = CombineHashes(meshHash, std::hash<size_t>()(indices.size()));

				Material* material = AssetsManager::GetDefaultMaterial();
				if (shape.mesh.material_ids.size() > 0 && materials.size() >= shape.mesh.material_ids[0]) {
					tinyobj::material_t tinyobjMaterial = materials.at(shape.mesh.material_ids[0]);
					std::string diffusePath = parentPath + "/" + tinyobjMaterial.diffuse_texname;

					material = ObjModelMaterialLoader(
						&tinyobjMaterial, std::filesystem::path{asset.mPath}.parent_path().string(), loadedTextures);

					std::filesystem::path materialOutPath =
						Editor::Gui::FileExplorer::currentDirectory + "/" +
						Editor::Utils::Filesystem::GetUnrepeatedName(Editor::Gui::FileExplorer::currentDirectory + "/" +
																	 material->mAssetName) +
						Standards::materialExtName;

					if (loadedMaterials.find(materialOutPath) == loadedMaterials.end()) {
						loadedMaterials.emplace(materialOutPath, material->mAssetUuid);
						AssetsSerializer::SerializeMaterial(material, materialOutPath,
															Application::Get()->mSettings.mMaterialSerializationMode);
						AssetsManager::AddMaterial(material);
					}
					else
						material = AssetsManager::GetMaterial(loadedMaterials.find(materialOutPath)->second);
				}

				std::vector<unsigned int> materials{0};

				MeshRenderer* meshRenderer = modelScene->NewComponent<MeshRenderer>(newEntity->uuid);

				Mesh* mesh = nullptr;
				if (uniqueMeshes.find(meshHash) == uniqueMeshes.end()) {
					mesh = new Mesh(); // Application::Get()->mRenderer->CreateNewMesh(vertices, normals, uvs, tangents,
									   // indices, materials, true);
					mesh->vertices = vertices;
					mesh->normals = normals;
					mesh->uvs = uvs;
					mesh->tangent = tangents;
					mesh->indices = indices;
					mesh->materialsIndices = materials;
					uniqueMeshes.emplace(meshHash, std::make_shared<Mesh>(*mesh));
				}
				else {
					mesh = uniqueMeshes[meshHash].get();
				}
				meshRenderer->ChangeMesh(mesh);

				meshRenderer->AddMaterial(material);
				// meshRenderer->mMaterials.push_back(material);
				// modelScene->NewComponent<MeshRenderer>(newEntity->uuid);

				Collider* collider = modelScene->NewComponent<Collider>(newEntity->uuid);
				ColliderShape* shape = new ColliderShape(nullptr, Plaza::ColliderShape::MESH, mesh->uuid);
				collider->AddShape(shape);
			}
			index++;
		}

		for (auto& [key, mesh] : uniqueMeshes) {
			model.AddMeshes({mesh});
		}
		return modelScene;
	}
} // namespace Plaza
