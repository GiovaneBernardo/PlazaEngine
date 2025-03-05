#pragma once
// #include "Engine/Core/Renderer/OpenGL/OpenGLMesh.h"
#include "Engine/Components/Rendering/Material.h"
#include "Engine/Core/Renderer/Mesh.h"
#include "Engine/Core/Engine.h"

namespace Plaza {
	struct Asset;
	struct Material;
	class PLAZA_API RenderGroup {
	  public:
		static inline GLenum renderMode = GL_TRIANGLES;
		uint64_t uuid = -1;
		Mesh* mesh = nullptr;
		bool mImportedMesh = false;
		// Material* material = nullptr;
		std::vector<Material*> materials = std::vector<Material*>();
		std::vector<uint64_t> materialsUuid = std::vector<uint64_t>();
		vector<glm::mat4> instanceModelMatrices = vector<glm::mat4>();
		std::vector<unsigned int> instanceMaterialOffsets = std::vector<unsigned int>();
		vector<unsigned int> instanceMaterialIndices = vector<unsigned int>();
		vector<vector<glm::mat4>> mCascadeInstances = vector<vector<glm::mat4>>();

		uint64_t mBufferSize = 8;
		uint64_t mCount = 0;

		std::vector<VkBuffer> mInstanceBuffers = std::vector<VkBuffer>();
		std::vector<VkDeviceMemory> mInstanceBufferMemories = std::vector<VkDeviceMemory>();
		void InitializeInstanceBuffer();
		void ResizeInstanceBuffer(uint64_t newSize = 0);

		RenderGroup(Mesh* mesh, std::vector<Material*> mats) {
			this->uuid = Plaza::UUID::NewUUID();
			this->mesh = mesh;
			this->materials = mats;
			// InitializeInstanceBuffer();
		}

		RenderGroup(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> mat) {
			this->uuid = Plaza::UUID::NewUUID();
			this->mesh = mesh.get();
			// this->material = mat.get();
			// InitializeInstanceBuffer();
		}

		inline void AddInstance(const glm::mat4& model) {
			instanceModelMatrices.push_back(model);
			// this->instanceMaterialIndices.push_back(material);
		}

		void AddCascadeInstance(glm::mat4 model, unsigned int cascadeIndex) {
			if (cascadeIndex >= mCascadeInstances.size()) {
				mCascadeInstances.resize(cascadeIndex + 1);
			}
			mCascadeInstances[cascadeIndex].push_back(model);
		}

		void AddMaterial(Material* newMaterial) {
			this->materials.push_back(newMaterial);
			this->materialsUuid.push_back(newMaterial->mAssetUuid);
		}

		void ChangeMaterial(Material* newMaterial, unsigned int index = 0);
		void ChangeMesh(Mesh* mesh);
		static bool SceneHasRenderGroup(Mesh* mesh, Material* material);
		static bool SceneHasRenderGroup(RenderGroup* renderGroup);
	};
} // namespace Plaza
