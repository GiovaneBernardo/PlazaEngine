#pragma once
//#include "Engine/Core/Renderer/OpenGL/OpenGLMesh.h"
#include "Engine/Core/Renderer/Mesh.h"
#include "Engine/Components/Rendering/Material.h"
#include "Engine/Shaders/Shader.h"
namespace Plaza {
	class RenderGroup {
	public:
		static GLenum renderMode;
		uint64_t uuid = -1;
		Mesh* mesh = nullptr;
		bool mImportedMesh = false;
		//Material* material = nullptr;
		std::vector<Material*> materials = std::vector<Material*>();
		std::vector<uint64_t> materialsUuid = std::vector<uint64_t>();
		vector<glm::mat4> instanceModelMatrices = vector<glm::mat4>();
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
			//InitializeInstanceBuffer();
		}
		
		RenderGroup(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> mat) {
			this->uuid = Plaza::UUID::NewUUID();
			this->mesh = mesh.get();
			//this->material = mat.get();
			//InitializeInstanceBuffer();
		}

		void AddInstance(glm::mat4 model) {
			instanceModelMatrices.push_back(model);
			//this->instanceMaterialIndices.push_back(material);
		}

		void AddCascadeInstance(glm::mat4 model, unsigned int cascadeIndex) {
			if (cascadeIndex >= mCascadeInstances.size()) {
				mCascadeInstances.resize(cascadeIndex + 1);
			}
			mCascadeInstances[cascadeIndex].push_back(model);
		}

		void AddMaterial(Material* newMaterial) {
			this->materials.push_back(newMaterial);
			this->materialsUuid.push_back(newMaterial->uuid);
		}

		void ChangeMaterial(Material* newMaterial, unsigned int index = 0);
		void ChangeMesh(Mesh* mesh);
		static bool SceneHasRenderGroup(Mesh* mesh, Material* material);
		static bool SceneHasRenderGroup(RenderGroup* renderGroup);
		//bool SceneHasRenderGroupWithMaterial(Material* material);


		//void BindTextures(Shader& shader) {
		//	constexpr const char* shininessUniform = "shininess";
		//	constexpr const char* textureDiffuseRGBAUniform = "texture_diffuse_rgba";
		//	constexpr const char* textureDiffuseUniform = "texture_diffuse";
		//	constexpr const char* textureSpecularRGBAUniform = "texture_specular_rgba";
		//	constexpr const char* textureSpecularUniform = "texture_specular";
		//	constexpr const char* textureNormalUniform = "texture_normal";
		//	constexpr const char* textureHeightUniform = "texture_height";
		//	constexpr const char* textureMetalnessUniform = "texture_metalness";
		//	constexpr const char* textureRoughnessUniform = "texture_roughness";
		//
		//	shader.setFloat("intensity", material->intensity);
		//
		//	shader.setBool("usingNormal", mesh->usingNormal);
		//	if (material->shininess != 64.0f) {
		//		shader.setFloat(shininessUniform, material->shininess);
		//	}
		//
		//	if (!material->diffuse->IsTextureEmpty()) {
		//		const glm::vec4& diffuseRGBA = material->diffuse->rgba;
		//		if (diffuseRGBA != mesh->infVec) {
		//			shader.setVec4(textureDiffuseRGBAUniform, diffuseRGBA);
		//		}
		//		else {
		//			constexpr GLint textureDiffuseUnit = 0;
		//			glActiveTexture(GL_TEXTURE0 + textureDiffuseUnit);
		//			glBindTexture(GL_TEXTURE_2D, material->diffuse->GetTextureID());
		//			shader.setVec4(textureDiffuseRGBAUniform, glm::vec4(300, 300, 300, 300));
		//		}
		//	}
		//
		//	if (!material->specular->IsTextureEmpty()) {
		//		const glm::vec4& specularRGBA = material->specular->rgba;
		//		if (specularRGBA != glm::vec4(INFINITY)) {
		//			shader.setVec4(textureSpecularRGBAUniform, specularRGBA);
		//		}
		//		else {
		//			constexpr GLint textureSpecularUnit = 1;
		//			glActiveTexture(GL_TEXTURE0 + textureSpecularUnit);
		//			glBindTexture(GL_TEXTURE_2D, material->specular->GetTextureID());
		//			shader.setVec4(textureSpecularRGBAUniform, glm::vec4(300, 300, 300, 300));
		//		}
		//	}
		//
		//	if (!material->normal->IsTextureEmpty()) {
		//		constexpr GLint textureNormalUnit = 2;
		//		glActiveTexture(GL_TEXTURE0 + textureNormalUnit);
		//		glBindTexture(GL_TEXTURE_2D, material->normal->GetTextureID());
		//	}
		//
		//	if (!material->height->IsTextureEmpty()) {
		//		constexpr GLint textureHeightUnit = 3;
		//		glActiveTexture(GL_TEXTURE0 + textureHeightUnit);
		//		glBindTexture(GL_TEXTURE_2D, material->height->GetTextureID());
		//	}
		//
		//	if (!material->metalness->IsTextureEmpty()) {
		//		constexpr GLint textureHeightUnit = 4;
		//		glActiveTexture(GL_TEXTURE0 + textureHeightUnit);
		//		glBindTexture(GL_TEXTURE_2D, material->metalness->GetTextureID());
		//	}
		//
		//	if (!material->roughness->IsTextureEmpty()) {
		//		constexpr GLint textureHeightUnit = 5;
		//		glActiveTexture(GL_TEXTURE0 + textureHeightUnit);
		//		glBindTexture(GL_TEXTURE_2D, material->roughness->GetTextureID());
		//	}
		//}

		//void Draw(Shader& shader) {
		//	// draw mesh
		//	glBindVertexArray(mesh->VAO);
		//	glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh->indices.size()), GL_UNSIGNED_INT, 0);
		//	glBindVertexArray(0);
		//
		//	// always good practice to set everything back to defaults once configured.
		//	glActiveTexture(GL_TEXTURE0);
		//	Time::drawCalls += 1;
		//}

		//void DrawInstancedToShadowMap(Shader& shader) {
		//	if (instanceModelMatrices.size() > 0 && renderMode != GL_TRIANGLE_STRIP && mesh) {
		//		// Setup instance buffer
		//		glBindBuffer(GL_ARRAY_BUFFER, mesh->instanceBuffer);
		//		glBufferData(GL_ARRAY_BUFFER, instanceModelMatrices.size() * sizeof(glm::mat4), &instanceModelMatrices[0], GL_STATIC_DRAW);
		//		// draw mesh
		//		glBindVertexArray(mesh->VAO);
		//		//glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
		//		glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(mesh->indices.size()), GL_UNSIGNED_INT, 0, instanceModelMatrices.size());
		//		glBindVertexArray(0);
		//		// always good practice to set everything back to defaults once configured.
		//		Time::drawCalls += 1;
		//		glBindBuffer(GL_ARRAY_BUFFER, 0);
		//	}
		//}

		//void DrawInstanced(Shader& shader) {
		//	if (instanceModelMatrices.size() > 0 && mesh) {
		//		//if (this->mesh->meshType == MeshType::Triangle) {
		//		BindTextures(shader);
		//		// Setup instance buffer
		//
		//		glBindBuffer(GL_ARRAY_BUFFER, mesh->instanceBuffer);
		//		glBufferData(GL_ARRAY_BUFFER, instanceModelMatrices.size() * sizeof(glm::mat4), &instanceModelMatrices[0], GL_STATIC_DRAW);
		//		// draw mesh
		//		glBindVertexArray(mesh->VAO);
		//		//glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
		//		glDrawElementsInstanced(RenderGroup::renderMode, static_cast<unsigned int>(mesh->indices.size()), GL_UNSIGNED_INT, 0, instanceModelMatrices.size());
		//		glBindVertexArray(0);
		//		//instanceModelMatrices.resize(0);
		//	//}
		//	//else if (this->mesh->meshType == MeshType::HeightField) {
		//	//	glBindBuffer(GL_ARRAY_BUFFER, mesh->instanceBuffer);
		//	//	glBufferData(GL_ARRAY_BUFFER, instanceModelMatrices.size() * sizeof(glm::mat4), &instanceModelMatrices[0], GL_STATIC_DRAW);
		//	//	// draw mesh
		//	//	glBindVertexArray(mesh->VAO);
		//	//	//glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
		//	//	glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(mesh->indices.size()), GL_UNSIGNED_INT, 0, instanceModelMatrices.size());
		//	//	glBindVertexArray(0);
		//	//}
		//		Time::drawCalls += 1;
		//		Time::addInstanceCalls += instanceModelMatrices.size();
		//		instanceModelMatrices.clear();
		//	}
		//}
	};
}

inline GLenum RenderGroup::renderMode = GL_TRIANGLES;