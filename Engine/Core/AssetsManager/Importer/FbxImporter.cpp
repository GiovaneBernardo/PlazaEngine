#include "Engine/Core/PreCompiledHeaders.h"
#include "AssetsImporter.h"
#include "ThirdParty/glm/gtx/hash.hpp"
#include "Engine/Core/Renderer/Mesh.h"
#include "Engine/Core/AssetsManager/Serializer/AssetsSerializer.h"
#include "Engine/Core/AssetsManager/Loader/AssetsLoader.h"
#include "Editor/GUI/FileExplorer/FileExplorer.h"
#include "Editor/GUI/Utils/Filesystem.h"
#include "Engine/Core/Renderer/Model.h"
#define UFBX_REAL_TYPE float
#include "ThirdParty/ufbx/ufbx/ufbx.h"
#include "ThirdParty/ufbx/ufbx/ufbx.c"
#include "Engine/Core/Scene.h"
#include "Engine/Core/Renderer/Vulkan/Renderer.h"
#include "Engine/ECS/ECSManager.h"

#include <stdio.h>
#ifdef WIN32
#include <d3d11.h>
#include <tchar.h>
#endif
#include <stdio.h>
#include <inttypes.h>
#include <vector>
#include <malloc.h>

struct Vec3Hash {
	std::size_t operator()(const glm::vec3& v) const { return std::hash<glm::vec3>()(v); }
};

namespace Plaza {
	static std::string GetFbxTexturePath(const ufbx_texture* texture, const std::string& materialFolderPath) {
		std::string path = texture->filename.data;
		if (!std::filesystem::exists(path)) {
			path = std::filesystem::path{materialFolderPath}.parent_path().string() + "/" +
				   texture->relative_filename.data;
			if (!std::filesystem::exists(path) || std::filesystem::is_directory(path)) {
				path = materialFolderPath + "/" +
					   std::filesystem::path{texture->absolute_filename.data}.filename().string();
				if (!std::filesystem::exists(path)) {
					path = "";
				}
			}
		}
		return path;
	}

	static Texture* ConvertUFBXTextureToPlazaTexture(const ufbx_material_map& materialMap,
													 const std::string& materialFolderPath,
													 std::unordered_map<std::string, uint64_t>& loadedTextures) {
		Texture* texture;
		if (materialMap.texture_enabled) {
			const std::string texturePath = GetFbxTexturePath(materialMap.texture, materialFolderPath);
			if (texturePath.empty())
				return new Texture();

			if (loadedTextures.find(texturePath) == loadedTextures.end()) {
				texture =
					AssetsLoader::LoadTexture(AssetsManager::GetAssetOrImport(texturePath, Plaza::UUID::NewUUID()));
				loadedTextures.emplace(texturePath, texture->mAssetUuid);
			}
			else {
				texture = AssetsManager::mTextures.at(loadedTextures.at(texturePath));
			}
		}
		else {
			texture = new Texture();
			texture->rgba = glm::vec4(materialMap.value_vec4.x, materialMap.value_vec4.y, materialMap.value_vec4.z,
									  materialMap.value_vec4.w);
		}

		return texture;
	}

	static inline bool IsMapUsed(const ufbx_material_map& map) { return map.texture != nullptr; }

	Material* AssetsImporter::FbxModelMaterialLoader(const ufbx_material* ufbxMaterial,
													 const std::string materialFolderPath,
													 std::unordered_map<std::string, uint64_t>& loadedTextures) {
		Material* material = new Material();
		material->mAssetName = ufbxMaterial->name.data;

		std::vector<ufbx_material_map> usedPBRMaps = std::vector<ufbx_material_map>();
		std::vector<ufbx_material_map> usedFBXMaps = std::vector<ufbx_material_map>();
		for (ufbx_material_map map : ufbxMaterial->pbr.maps) {
			if (IsMapUsed(map))
				usedPBRMaps.push_back(map);
		}
		for (ufbx_material_map map : ufbxMaterial->fbx.maps) {
			if (IsMapUsed(map))
				usedFBXMaps.push_back(map);
		}

		if (ufbxMaterial->features.pbr.enabled) {
			material->diffuse = std::shared_ptr<Texture>(
				ConvertUFBXTextureToPlazaTexture(ufbxMaterial->pbr.base_color, materialFolderPath, loadedTextures));
			material->normal = std::shared_ptr<Texture>(
				ConvertUFBXTextureToPlazaTexture(ufbxMaterial->pbr.normal_map, materialFolderPath, loadedTextures));
			material->roughness = std::shared_ptr<Texture>(
				ConvertUFBXTextureToPlazaTexture(ufbxMaterial->pbr.roughness, materialFolderPath, loadedTextures));
			material->metalness = std::shared_ptr<Texture>(
				ConvertUFBXTextureToPlazaTexture(ufbxMaterial->pbr.metalness, materialFolderPath, loadedTextures));
		}
		else {
			material->diffuse = std::shared_ptr<Texture>(
				ConvertUFBXTextureToPlazaTexture(ufbxMaterial->fbx.diffuse_color, materialFolderPath, loadedTextures));
			material->normal = std::shared_ptr<Texture>(
				ConvertUFBXTextureToPlazaTexture(ufbxMaterial->fbx.normal_map, materialFolderPath, loadedTextures));
			material->roughness = std::shared_ptr<Texture>(
				ConvertUFBXTextureToPlazaTexture(ufbxMaterial->fbx.specular_color, materialFolderPath, loadedTextures));
			material->metalness = std::shared_ptr<Texture>(ConvertUFBXTextureToPlazaTexture(
				ufbxMaterial->fbx.reflection_color, materialFolderPath, loadedTextures));
		}

		if (material->diffuse == nullptr || material->normal == nullptr || material->roughness == nullptr ||
			material->metalness == nullptr)
			material = AssetsManager::GetDefaultMaterial();
		return material;
	}

	int decodeIndex(int idx) { return (idx < 0) ? (-idx - 1) : idx; }

	static inline glm::vec3 ConvertUfbxVec3(ufbx_vec3 vec) { return glm::vec3(vec.x, vec.y, vec.z); }

	static inline glm::vec2 ConvertUfbxVec2(ufbx_vec2 vec) { return glm::vec2(vec.x, vec.y); }

	static inline glm::quat ConvertUfbxQuat(ufbx_quat const& quaternionUfbx) {
		return glm::quat(quaternionUfbx.w, quaternionUfbx.x, quaternionUfbx.y, quaternionUfbx.z);
	};

	static glm::mat4 ConvertUfbxMatrix(ufbx_matrix const& mat4Ufbx) {
		glm::mat4 inverseBindMatrix = glm::identity<glm::mat4>();

		inverseBindMatrix[0][0] = mat4Ufbx.m00;
		inverseBindMatrix[0][1] = mat4Ufbx.m10;
		inverseBindMatrix[0][2] = mat4Ufbx.m20;

		inverseBindMatrix[1][0] = mat4Ufbx.m01;
		inverseBindMatrix[1][1] = mat4Ufbx.m11;
		inverseBindMatrix[1][2] = mat4Ufbx.m21;

		inverseBindMatrix[2][0] = mat4Ufbx.m02;
		inverseBindMatrix[2][1] = mat4Ufbx.m12;
		inverseBindMatrix[2][2] = mat4Ufbx.m22;

		inverseBindMatrix[3][0] = mat4Ufbx.m03;
		inverseBindMatrix[3][1] = mat4Ufbx.m13;
		inverseBindMatrix[3][2] = mat4Ufbx.m23;
		return inverseBindMatrix;
	};

	std::shared_ptr<Mesh> ConvertUFBXMesh(ufbx_mesh* ufbxMesh,
										  std::unordered_map<std::string, uint64_t>& loadedTextures,
										  std::unordered_map<std::filesystem::path, uint64_t>& loadedMaterials,
										  bool noTangent, const glm::vec3& modelImporterScale) {
		if (!ufbxMesh)
			return nullptr;

		std::shared_ptr<Mesh> finalMesh = std::make_shared<Mesh>();
		/* Skinning */
		if (ufbxMesh->skin_deformers.count > 0) {
			ufbx_skin_deformer* skin = ufbxMesh->skin_deformers[0];
			for (unsigned int i = 0; i < skin->clusters.count; ++i) {
				ufbx_skin_cluster* cluster = skin->clusters[i];
				uint64_t parentUuid = 0;
				if (cluster->bone_node->parent)
					parentUuid = cluster->bone_node->bone->element_id;
				finalMesh->uniqueBonesInfo.emplace(cluster->bone_node->bone->element_id,
												   Bone{cluster->bone_node->bone->element_id, parentUuid,
														cluster->bone_node->bone->element_id, cluster->name.data,
														ConvertUfbxMatrix(cluster->geometry_to_bone)});
			}
		}

		uint64_t indices = 0;
		size_t triangleIndicesCount = ufbxMesh->max_face_triangles * 3;
		std::vector<uint32_t> triangleIndices = std::vector<uint32_t>();
		triangleIndices.resize(triangleIndicesCount);

		int faceIndex = 0;
		for (const ufbx_face& face : ufbxMesh->faces) {
			size_t trianglesNumber =
				ufbx_triangulate_face(triangleIndices.data(), triangleIndicesCount, ufbxMesh, face);
			uint32_t materialIndex = 0; // ufbxMesh->face_material[faceIndex];
			if (ufbxMesh->face_material.count > faceIndex)
				materialIndex = ufbxMesh->face_material[faceIndex];
			faceIndex++;
			for (size_t vertexIndex = 0; vertexIndex < trianglesNumber * 3; vertexIndex++) {
				uint32_t index = triangleIndices[vertexIndex];
				// finalMesh->indices.push_back(index);

				glm::vec3 position = ConvertUfbxVec3(ufbxMesh->vertex_position[index]);
				glm::vec3 normal = ConvertUfbxVec3(ufbxMesh->vertex_normal[index]);
				glm::vec2 uv;
				if (ufbxMesh->vertex_uv.exists)
					uv = ConvertUfbxVec2(ufbxMesh->vertex_uv[index]);

				finalMesh->vertices.push_back(position * modelImporterScale);
				finalMesh->normals.push_back(normal);
				finalMesh->uvs.push_back(uv);
				if (ufbxMesh->vertex_tangent.exists) {
					noTangent = false;
					finalMesh->tangent.push_back(ConvertUfbxVec3(ufbxMesh->vertex_tangent[index]));
				}
				else
					finalMesh->tangent.push_back(glm::vec3(0.0f));

				finalMesh->materialsIndices.push_back(materialIndex);

				if (ufbxMesh->skin_deformers.count > 0) {
					ufbx_skin_deformer* skin = ufbxMesh->skin_deformers[0];
					const uint32_t vertex = ufbxMesh->vertex_indices[index];

					ufbx_skin_vertex skinVertex = skin->vertices[vertex];
					uint32_t num_weights = skinVertex.num_weights;
					num_weights = std::min(num_weights, 4u);

					float totalWeight = 0;
					Plaza::BonesHolder holder{};
					for (uint32_t i = 0; i < num_weights; ++i) {
						ufbx_skin_weight skin_weight = skin->weights[skinVertex.weight_begin + i];
						holder.mBones.push_back(skin->clusters[skin_weight.cluster_index]->bone_node->bone->element_id);
						holder.mWeights.push_back(skin_weight.weight);
						totalWeight += skin_weight.weight;
					}

					if (totalWeight > 0) {
						for (uint32_t i = 0; i < num_weights; ++i) {
							holder.mWeights[i] /= totalWeight;
						}
					}
					finalMesh->bonesHolder.push_back(holder);
				}

				indices++;
			}
		}

		if (noTangent) {
			// Calculate tangent
			std::vector<glm::vec3> accumulatedTangents(finalMesh->vertices.size(), glm::vec3(0.0f));
			std::vector<glm::vec3> accumulatedBitangents(finalMesh->vertices.size(), glm::vec3(0.0f));

			// Calculate tangents and bitangents per triangle
			for (size_t i = 0; i < finalMesh->indices.size(); i += 3) {
				uint32_t i0 = finalMesh->indices[i];
				uint32_t i1 = finalMesh->indices[i + 1];
				uint32_t i2 = finalMesh->indices[i + 2];

				glm::vec3& v0 = finalMesh->vertices[i0];
				glm::vec3& v1 = finalMesh->vertices[i1];
				glm::vec3& v2 = finalMesh->vertices[i2];

				glm::vec2& uv0 = finalMesh->uvs[i0];
				glm::vec2& uv1 = finalMesh->uvs[i1];
				glm::vec2& uv2 = finalMesh->uvs[i2];

				// Calculate the edges of the triangle in model space
				glm::vec3 deltaPos1 = v1 - v0;
				glm::vec3 deltaPos2 = v2 - v0;

				// Calculate the differences in UV coordinates
				glm::vec2 deltaUV1 = uv1 - uv0;
				glm::vec2 deltaUV2 = uv2 - uv0;

				// Calculate the tangent and bitangent
				float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
				glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
				glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

				// Accumulate the tangents and bitangents for each vertex of the triangle
				accumulatedTangents[i0] += tangent;
				accumulatedTangents[i1] += tangent;
				accumulatedTangents[i2] += tangent;

				accumulatedBitangents[i0] += bitangent;
				accumulatedBitangents[i1] += bitangent;
				accumulatedBitangents[i2] += bitangent;
			}

			// Normalize and orthogonalize tangents and bitangents for each vertex
			for (size_t i = 0; i < finalMesh->vertices.size(); ++i) {
				glm::vec3& n = finalMesh->normals[i];
				glm::vec3& t = accumulatedTangents[i];
				glm::vec3& b = accumulatedBitangents[i];

				// Gram-Schmidt orthogonalize the tangent
				t = glm::normalize(t - n * glm::dot(n, t));

				// Calculate handedness (flip the tangent direction if necessary)
				if (glm::dot(glm::cross(n, t), b) < 0.0f) {
					t = t * -1.0f;
				}

				// Store the final tangent in the mesh
				finalMesh->tangent[i] = t;
			}

			finalMesh->tangent = accumulatedTangents;
		}

		vector<ufbx_vertex_stream> streams;
		if (!finalMesh->vertices.empty()) {
			streams.push_back({finalMesh->vertices.data(), finalMesh->vertices.size(), sizeof(finalMesh->vertices[0])});
		}
		if (!finalMesh->normals.empty()) {
			streams.push_back({finalMesh->normals.data(), finalMesh->normals.size(), sizeof(finalMesh->normals[0])});
		}
		if (!finalMesh->uvs.empty()) {
			streams.push_back({finalMesh->uvs.data(), finalMesh->uvs.size(), sizeof(finalMesh->uvs[0])});
		}
		if (!finalMesh->bonesHolder.empty()) {
			streams.push_back(
				{finalMesh->bonesHolder.data(), finalMesh->bonesHolder.size(), sizeof(finalMesh->bonesHolder[0])});
		}
		if (!finalMesh->materialsIndices.empty()) {
			streams.push_back({finalMesh->materialsIndices.data(), finalMesh->materialsIndices.size(),
							   sizeof(finalMesh->materialsIndices[0])});
		}

		finalMesh->indices.clear();
		finalMesh->indices.resize(ufbxMesh->num_triangles * 3);
		ufbx_error error;
		size_t num_vertices = ufbx_generate_indices(streams.data(), streams.size(), finalMesh->indices.data(),
													finalMesh->indices.size(), NULL, &error);

		if (finalMesh->vertices.size() > 0)
			finalMesh->vertices.resize(num_vertices);
		if (finalMesh->normals.size() > 0)
			finalMesh->normals.resize(num_vertices);
		if (finalMesh->uvs.size() > 0)
			finalMesh->uvs.resize(num_vertices);
		if (finalMesh->bonesHolder.size() > 0)
			finalMesh->bonesHolder.resize(num_vertices);
		if (finalMesh->materialsIndices.size() > 0)
			finalMesh->materialsIndices.resize(num_vertices);

		return finalMesh;
	}

	std::shared_ptr<Scene> AssetsImporter::ImportFBX(AssetImported asset, std::filesystem::path outPath, Model& model,
													 AssetsImporterSettings settings) {
		// FIX: Remake model importers
		ufbx_load_opts opts = {};
		opts.target_axes = ufbx_axes_right_handed_y_up;
		opts.target_unit_meters = 1.0f;
		opts.target_axes = {
			.right = UFBX_COORDINATE_AXIS_POSITIVE_X,
			.up = UFBX_COORDINATE_AXIS_POSITIVE_Y,
			.front = UFBX_COORDINATE_AXIS_POSITIVE_Z,
		};

		ufbx_error error;
		ufbx_scene* scene = ufbx_load_file(asset.mPath.c_str(), &opts, &error);

		if (!scene) {
			fprintf(stderr, "Failed to load scene: %s\n", error.description.data);
			return nullptr;
		}

		// Entity* mainEntity = new Entity(std::filesystem::path{ asset.mPath }.stem().string(),
		// Scene::GetActiveScene()->mainSceneEntity);
		std::shared_ptr<Scene> modelScene = std::make_shared<Scene>();
		if (scene->root_node->mesh == nullptr) {
			Entity* entity = modelScene->NewEntity(std::filesystem::path{asset.mPath}.stem().string(), nullptr);
			modelScene->mainSceneEntity = modelScene->GetEntity(entity->uuid);
			modelScene->mainSceneEntityUuid = modelScene->mainSceneEntity->uuid;
			modelScene->mainSceneEntity->parentUuid = modelScene->mainSceneEntity->uuid;

			ufbx_transform& transform = scene->root_node->local_transform;
			modelScene->GetComponent<TransformComponent>(entity->uuid)->mLocalPosition =
				ConvertUfbxVec3(transform.translation);
			modelScene->GetComponent<TransformComponent>(entity->uuid)->mLocalRotation =
				ConvertUfbxQuat(transform.rotation);
			modelScene->GetComponent<TransformComponent>(entity->uuid)->mLocalScale = ConvertUfbxVec3(transform.scale);
		}

		std::unordered_map<uint64_t, Entity*> entities = std::unordered_map<uint64_t, Entity*>();
		std::unordered_map<uint64_t, uint64_t> meshIndexEntityMap =
			std::unordered_map<uint64_t, uint64_t>(); // ufbx id, plaza uuid
		std::unordered_map<uint64_t, uint64_t> entityMeshParent =
			std::unordered_map<uint64_t, uint64_t>(); // Entity, Parent

		std::unordered_map<std::string, uint64_t> loadedTextures = std::unordered_map<std::string, uint64_t>();
		std::unordered_map<std::filesystem::path, uint64_t> loadedMaterials =
			std::unordered_map<std::filesystem::path, uint64_t>();
		std::unordered_map<size_t, std::shared_ptr<Mesh>> uniqueMeshes =
			std::unordered_map<size_t, std::shared_ptr<Mesh>>();

		bool noTangent = true;

		for (ufbx_mesh* ufbxMesh : scene->meshes) {
			if (uniqueMeshes.find(ufbxMesh->element_id) != uniqueMeshes.end()) {
				PL_CORE_WARN("Duplicated UFBX Mesh");
				continue;
			}
			std::shared_ptr<Mesh> mesh =
				ConvertUFBXMesh(ufbxMesh, loadedTextures, loadedMaterials, noTangent, mModelImporterScale);
			uniqueMeshes.emplace(ufbxMesh->element_id, mesh);
			model.AddMeshes({mesh});
		}

		for (size_t i = 0; i < scene->nodes.count; ++i) {
			Application::Get()->mRenderer->UpdateMainProgressBar(float(float(i) / float(scene->nodes.count)));

			ufbx_node* node = scene->nodes.data[i];
			ufbx_mesh* ufbxMesh = node->mesh;
			if (!ufbxMesh)
				continue;
			ufbx_transform& transform = node->local_transform;

			std::string name = node->name.data;

			Entity* parent = entities.find(node->parent->element_id) != entities.end()
								 ? entities.at(node->parent->element_id)
								 : modelScene->mainSceneEntity;
			Entity* entity = modelScene->NewEntity(name, parent);
			if (!modelScene->mainSceneEntity) {
				modelScene->mainSceneEntity = modelScene->GetEntity(entity->uuid);
				modelScene->mainSceneEntityUuid = modelScene->mainSceneEntity->uuid;
				modelScene->mainSceneEntity->parentUuid = modelScene->mainSceneEntity->uuid;
			}
			modelScene->GetComponent<TransformComponent>(entity->uuid)->mLocalPosition =
				ConvertUfbxVec3(transform.translation);
			modelScene->GetComponent<TransformComponent>(entity->uuid)->mLocalRotation =
				ConvertUfbxQuat(transform.rotation);
			modelScene->GetComponent<TransformComponent>(entity->uuid)->mLocalScale = ConvertUfbxVec3(transform.scale);

			entities.emplace(entity->uuid, entity);
			meshIndexEntityMap.emplace(ufbxMesh->element_id, entity->uuid);
			ufbx_node* parentNode = node->parent;
			if (parentNode)
				entityMeshParent.emplace(entity->uuid, parentNode->element_id);
			else
				entityMeshParent.emplace(entity->uuid, 0);

			// Mesh* finalMesh = new Mesh();

			/* Material */
			std::vector<Material*> materials = std::vector<Material*>();

			if (settings.mImportMaterials) {
				for (ufbx_material* ufbxMaterial : node->materials) {
					std::filesystem::path materialOutPath =
						Editor::Gui::FileExplorer::currentDirectory + "/" +
						Editor::Utils::Filesystem::GetUnrepeatedName(Editor::Gui::FileExplorer::currentDirectory + "/" +
																	 ufbxMaterial->name.data) +
						Standards::materialExtName;
					bool materialIsNotLoaded = loadedMaterials.find(materialOutPath) == loadedMaterials.end();
					if (materialIsNotLoaded) {
						materialOutPath =
							Editor::Gui::FileExplorer::currentDirectory + "/" +
							Editor::Utils::Filesystem::GetUnrepeatedName(Editor::Gui::FileExplorer::currentDirectory +
																		 "/" + ufbxMaterial->name.data) +
							Standards::materialExtName;
						Material* material = AssetsImporter::FbxModelMaterialLoader(
							ufbxMaterial, std::filesystem::path{asset.mPath}.parent_path().string(), loadedTextures);
						if (material->mAssetUuid == AssetsManager::GetDefaultMaterial()->mAssetUuid) {
							loadedMaterials.emplace(materialOutPath, material->mAssetUuid);
							materials.push_back(material);
							continue;
						}
						material->flip = settings.mFlipTextures;
						loadedMaterials.emplace(materialOutPath, material->mAssetUuid);
						AssetsSerializer::SerializeMaterial(material, materialOutPath,
															Application::Get()->mSettings.mMaterialSerializationMode);
						AssetsManager::AddMaterial(material);
						materials.push_back(material);
					}
					else
						materials.push_back(AssetsManager::GetMaterial(loadedMaterials.find(materialOutPath)->second));
					// materials.push_back(material);
				}
			}
			else
				materials.push_back(AssetsManager::GetDefaultMaterial());

			std::shared_ptr<Mesh> mesh = nullptr;
			if (uniqueMeshes.find(node->mesh->element_id) != uniqueMeshes.end()) {
				mesh = uniqueMeshes.at(node->mesh->element_id);
			}
			else {
				mesh = std::make_shared<Mesh>();
			}

			if (materials.size() <= 0)
				materials.push_back(AssetsManager::GetDefaultMaterial());
			MeshRenderer* meshRenderer = modelScene->NewComponent<MeshRenderer>(entity->uuid);
			meshRenderer->ChangeMesh(mesh.get());
			for (Material* material : materials) {
				meshRenderer->AddMaterial(material);
			}

			Collider* collider = modelScene->NewComponent<Collider>(entity->uuid);
			ColliderShape* shape = new ColliderShape(nullptr, Plaza::ColliderShape::MESH, mesh->uuid);
			collider->AddShape(shape);
		}

		for (auto& [key, value] : entityMeshParent) {
			Entity* entity = &modelScene->entities.at(key);
			if (value != 0 && meshIndexEntityMap.find(value) != meshIndexEntityMap.end()) {
				ECS::EntitySystem::SetParent(modelScene.get(), entity,
											 &modelScene->entities.at(meshIndexEntityMap.at(value)));
			}
		}
		ECS::TransformSystem::UpdateSelfAndChildrenTransform(
			*modelScene->GetComponent<TransformComponent>(modelScene->mainSceneEntityUuid), nullptr, modelScene.get(),
			true, true);

		ufbx_free_scene(scene);
		return modelScene;
	}

	std::vector<Animation> AssetsImporter::ImportAnimationFBX(std::string filePath, AssetsImporterSettings settings) {
		std::vector<Animation> animations = std::vector<Animation>();

		ufbx_load_opts opts = {};
		opts.target_axes = ufbx_axes_right_handed_y_up, opts.target_unit_meters = 1.0f;
		opts.target_axes = {
			.right = UFBX_COORDINATE_AXIS_POSITIVE_X,
			.up = UFBX_COORDINATE_AXIS_POSITIVE_Y,
			.front = UFBX_COORDINATE_AXIS_POSITIVE_Z,
		};

		ufbx_error error;
		ufbx_scene* scene = ufbx_load_file(filePath.c_str(), &opts, &error);

		if (!scene) {
			fprintf(stderr, "Failed to load scene: %s\n", error.description.data);
			return animations;
		}
		// int meshCount = scene->meshes.count;
		// std::set<uint64_t> boneIds = std::set<uint64_t>();
		// std::vector<ufbx_bone*> uniqueBones = std::vector<ufbx_bone*>();

		for (auto& [key, value] : VulkanRenderer::GetRenderer()->mBones) {
			value.mChildren.clear();
		}

		for (const ufbx_skin_deformer* skin : scene->skin_deformers) {
			for (const ufbx_skin_cluster* cluster : skin->clusters) {
				// VulkanRenderer::GetRenderer()->mBones.at(cluster->bone_node->bone->element_id).mOffset =
				// (ConvertUfbxMatrix2(cluster->geometry_to_bone));
				if (cluster->bone_node->parent && cluster->bone_node->parent->bone) {
					VulkanRenderer::GetRenderer()->mBones.at(cluster->bone_node->bone->element_id).mParentId =
						cluster->bone_node->parent->bone->element_id;

					std::vector<uint64_t>& childrenVector =
						VulkanRenderer::GetRenderer()
							->mBones.at(cluster->bone_node->parent->bone->element_id)
							.mChildren;

					if (std::find(childrenVector.begin(), childrenVector.end(), cluster->bone_node->bone->element_id) ==
						childrenVector.end()) {
						childrenVector.push_back(cluster->bone_node->bone->element_id);
					}
				}
			}
		}
		for (const ufbx_anim_stack* stack : scene->anim_stacks) {
			if (stack->time_end - stack->time_begin <= 0.0f) {
				continue;
			}

			ufbx_baked_anim* bake = ufbx_bake_anim(scene, stack->anim, nullptr, nullptr);
			assert(bake != nullptr);
			if (bake == nullptr)
				continue;
			Animation& animation = animations.emplace_back(Animation());
			animation.mAssetName = stack->name.data;

			for (ufbx_bone* bone : scene->bones) {
				if (bone->is_root)
					animation.SetRootBone(&VulkanRenderer::GetRenderer()->mBones.at(bone->element_id));
			}
			if (!animation.GetRootBone()) {
				if (VulkanRenderer::GetRenderer()->mBones.find(scene->bones[0]->element_id) !=
					VulkanRenderer::GetRenderer()->mBones.end())
					animation.SetRootBone(&VulkanRenderer::GetRenderer()->mBones.at(scene->bones[0]->element_id));
				else
					animation.SetRootBoneUuid(scene->bones[0]->element_id);
			}
			// animation.mRootParentTransform = ConvertUfbxMatrix2(scene->root_node->geometry_to_world);
			animation.mStartTime = (float)bake->playback_time_begin;
			animation.mEndTime = (float)bake->playback_time_end;
			for (const ufbx_baked_node& bakeNode : bake->nodes) {
				ufbx_node* sceneNode = scene->nodes[bakeNode.typed_id];

				// Translation:
				std::map<float, glm::vec3> positions = std::map<float, glm::vec3>();
				for (const ufbx_baked_vec3& keyframe : bakeNode.translation_keys) {
					positions[keyframe.time] = glm::vec3(keyframe.value.x, keyframe.value.y, keyframe.value.z);
				}

				// Rotation:
				std::map<float, glm::quat> rotations = std::map<float, glm::quat>();
				for (const ufbx_baked_quat& keyframe : bakeNode.rotation_keys) {
					rotations[keyframe.time] = ConvertUfbxQuat(keyframe.value);
				}
				if (!sceneNode->bone)
					continue;
				uint64_t boneUuid = sceneNode->bone->element_id;
				unsigned int maxCount = glm::max(positions.size(), rotations.size());
				std::map<float, Bone::Keyframe> keyframes = std::map<float, Bone::Keyframe>();
				for (const auto& [key, value] : positions) {
					keyframes[key].position = value;
				}
				for (const auto& [key, value] : rotations) {
					keyframes[key].orientation = value;
				}
				for (const auto& [key, value] : keyframes) {
					animation.mKeyframes[boneUuid].push_back(
						Bone::Keyframe{key, ConvertUfbxVec3(ufbx_evaluate_baked_vec3(bakeNode.translation_keys, key)),
									   ConvertUfbxQuat(ufbx_evaluate_baked_quat(bakeNode.rotation_keys, key)),
									   ConvertUfbxVec3(ufbx_evaluate_baked_vec3(bakeNode.scale_keys, key))});
				}
				if (animation.mKeyframes[boneUuid].size() == 0) {
					animation.mKeyframes[boneUuid].push_back(
						Bone::Keyframe{0.0f, glm::vec3(0.0f), glm::quat(glm::vec3(0.0f)), glm::vec3(1.0f)});
				}
			}

			ufbx_free_baked_anim(bake);
		}

		ufbx_free_scene(scene);
		return animations;
	}
} // namespace Plaza
