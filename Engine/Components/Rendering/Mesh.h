#pragma once
#include "Engine/Core/PreCompiledHeaders.h"
#include "Engine/Components/Component.h"
#include "Engine/Components/Rendering/Texture.h"
#include "Engine/Components/Rendering/Material.h"


using namespace std;
using namespace Engine;
namespace Engine {
	struct Vertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 texCoords;
		glm::vec3 tangent;
		glm::vec3 bitangent;
		~Vertex() = default;
		Vertex(const glm::vec3& pos)
			: Vertex(pos, glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f), glm::vec3(0.0f)) {}

		Vertex(const glm::vec3& pos, const glm::vec3& norm, const glm::vec2& tex, const glm::vec3& tan, const glm::vec3& bitan)
			: position(pos), normal(norm), texCoords(tex), tangent(tan), bitangent(bitan) {}
	};

	class Mesh {
	public:
		unsigned int instanceBuffer;
		vector<glm::mat4> instanceModelMatrices = vector<glm::mat4>();
		uint64_t uuid;
		uint64_t meshId;
		float farthestVertex = 0.0f;
		std::string id;
		vector<Vertex> vertices;
		vector<unsigned int> indices;
		vector<Texture> textures;
		Material material = DefaultMaterial();
		bool usingNormal;
		glm::vec4 infVec = glm::vec4(INFINITY);
		unsigned int VAO;
		Mesh(const Mesh&) = default;
		~Mesh() = default;
		Mesh(vector<Vertex> vertices, vector<unsigned int> indices, Material material) {
			this->vertices = vertices;
			this->indices = indices;
			this->material = material;
			this->uuid = Engine::UUID::NewUUID();
			if (this->meshId == 0)
				this->meshId = Engine::UUID::NewUUID();
			setupMesh();
		}

		Mesh(vector<Vertex> vertices, vector<unsigned int> indices) {
			this->vertices = vertices;
			this->indices = indices;
			this->uuid = Engine::UUID::NewUUID();
			if (this->meshId == 0)
				this->meshId = Engine::UUID::NewUUID();
			setupMesh();
		}

		Mesh() {
			uuid = Engine::UUID::NewUUID();
		}

		void BindTextures(Shader& shader) {
			constexpr const char* shininessUniform = "shininess";
			constexpr const char* textureDiffuseRGBAUniform = "texture_diffuse_rgba";
			constexpr const char* textureDiffuseUniform = "texture_diffuse";
			constexpr const char* textureSpecularRGBAUniform = "texture_specular_rgba";
			constexpr const char* textureSpecularUniform = "texture_specular";
			constexpr const char* textureNormalUniform = "texture_normal";
			constexpr const char* textureHeightUniform = "texture_height";

			shader.setBool("usingNormal", usingNormal);
			if (material.shininess != 64.0f) {
				shader.setFloat(shininessUniform, material.shininess);
			}

			if (!material.diffuse.IsTextureEmpty()) {
				const glm::vec4& diffuseRGBA = material.diffuse.rgba;
				if (diffuseRGBA != infVec) {
					shader.setVec4(textureDiffuseRGBAUniform, diffuseRGBA);
				}
				else {
					constexpr GLint textureDiffuseUnit = 0;
					glActiveTexture(GL_TEXTURE0 + textureDiffuseUnit);
					glBindTexture(GL_TEXTURE_2D, material.diffuse.id);
					shader.setInt(textureDiffuseUniform, textureDiffuseUnit);
					shader.setVec4(textureDiffuseRGBAUniform, glm::vec4(300, 300, 300, 300));
				}
			}

			if (!material.specular.IsTextureEmpty()) {
				const glm::vec4& specularRGBA = material.specular.rgba;
				if (specularRGBA != glm::vec4(INFINITY)) {
					shader.setVec4(textureSpecularRGBAUniform, specularRGBA);
				}
				else {
					constexpr GLint textureSpecularUnit = 1;
					glUniform1i(glGetUniformLocation(shader.ID, textureSpecularUniform), textureSpecularUnit);
					glActiveTexture(GL_TEXTURE0 + textureSpecularUnit);
					glBindTexture(GL_TEXTURE_2D, material.specular.id);
					shader.setVec4(textureSpecularRGBAUniform, glm::vec4(300, 300, 300, 300));
				}
			}

			if (!material.normal.IsTextureEmpty()) {
				constexpr GLint textureNormalUnit = 2;
				glUniform1i(glGetUniformLocation(shader.ID, textureNormalUniform), textureNormalUnit);
				glActiveTexture(GL_TEXTURE0 + textureNormalUnit);
				glBindTexture(GL_TEXTURE_2D, material.normal.id);
			}

			if (!material.height.IsTextureEmpty()) {
				constexpr GLint textureHeightUnit = 3;
				glUniform1i(glGetUniformLocation(shader.ID, textureHeightUniform), textureHeightUnit);
				glActiveTexture(GL_TEXTURE0 + textureHeightUnit);
				glBindTexture(GL_TEXTURE_2D, material.height.id);
			}
		}

		void Draw(Shader& shader) {
			// draw mesh
			glBindVertexArray(VAO);
			glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);

			// always good practice to set everything back to defaults once configured.
			glActiveTexture(GL_TEXTURE0);
			Time::drawCalls += 1;
		}

		void AddInstance(Shader& shader, glm::mat4 model) {
			//Time::addInstanceCalls += 1;
			instanceModelMatrices.push_back(model);
		}

		void DrawInstancedToShadowMap(Shader& shader) {
			if (instanceModelMatrices.size() > 0) {
				// Setup instance buffer
				glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
				glBufferData(GL_ARRAY_BUFFER, instanceModelMatrices.size() * sizeof(glm::mat4), &instanceModelMatrices[0], GL_STATIC_DRAW);
				// draw mesh
				glBindVertexArray(VAO);
				//glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
				glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0, instanceModelMatrices.size());
				glBindVertexArray(0);

				// always good practice to set everything back to defaults once configured.
				Time::drawCalls += 1;
				glBindBuffer(GL_ARRAY_BUFFER, 0);
			}
		}

		void DrawInstanced(Shader& shader) {
			if (instanceModelMatrices.size() > 0) {
				BindTextures(shader);
				// Setup instance buffer
				glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
				glBufferData(GL_ARRAY_BUFFER, instanceModelMatrices.size() * sizeof(glm::mat4), &instanceModelMatrices[0], GL_STATIC_DRAW);
				// draw mesh
				glBindVertexArray(VAO);
				//glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
				glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0, instanceModelMatrices.size());
				glBindVertexArray(0);

				// always good practice to set everything back to defaults once configured.
				glActiveTexture(GL_TEXTURE0);
				glActiveTexture(GL_TEXTURE0 + 1);
				glBindTexture(GL_TEXTURE_2D, 0);
				glActiveTexture(GL_TEXTURE0 + 2);
				glBindTexture(GL_TEXTURE_2D, 0);
				glActiveTexture(GL_TEXTURE0 + 3);
				glBindTexture(GL_TEXTURE_2D, 0);
				Time::drawCalls += 1;
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				Time::addInstanceCalls += instanceModelMatrices.size();
				instanceModelMatrices.clear();
				//instanceModelMatrices.resize(0);
			}
		}

		void Terminate() {
			glDeleteVertexArrays(1, &VAO);
			glDeleteBuffers(1, &VBO);
		}

	private:
		unsigned int VBO, EBO;
		void setupMesh() {
			// create buffers/arrays
			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);
			glGenBuffers(1, &EBO);

			glBindVertexArray(VAO);
			// load data into vertex buffers
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			// A great thing about structs is that their memory layout is sequential for all its items.
			// The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
			// again translates to 3/2 floats which translates to a byte array.
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

			// set the vertex attribute pointers
			// vertex Positions
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

			// vertex normals
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
			// vertex texture coords
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
			//vertex tangent
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
			// vertex bitangent
			glEnableVertexAttribArray(4);
			glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));


			// Generate instanced array
			glGenBuffers(1, &instanceBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
			// set attribute pointers for matrix (4 times vec4)
			glEnableVertexAttribArray(7);
			glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);


			glEnableVertexAttribArray(8);
			glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));

			glEnableVertexAttribArray(9);
			glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));


			glEnableVertexAttribArray(10);
			glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));


			glVertexAttribDivisor(7, 1);
			glVertexAttribDivisor(8, 1);
			glVertexAttribDivisor(9, 1);
			glVertexAttribDivisor(10, 1);
			glBindVertexArray(0);


			for (Vertex vertex : vertices) {
				glm::vec3 absoluteVertex = glm::vec3(glm::abs(vertex.position.x), glm::abs(vertex.position.y), glm::abs(vertex.position.z));
				float absoluteSum = absoluteVertex.x + absoluteVertex.y + absoluteVertex.z;
				if (absoluteSum > farthestVertex)
					farthestVertex = absoluteSum;
			}
		}

		Material DefaultMaterial() {
			Material material;
			material.diffuse.rgba = glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
			material.specular.rgba = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
			material.shininess = 2.0f;
			return material;
		}
	};
}