#include "Engine/Core/PreCompiledHeaders.h"
#include "Mesh.h"
#include "VulkanTexture.h"
namespace Plaza {
	void VulkanMesh::Drawe() {
		if (firstRune) {
			vector<Vertex> convertedVertices;
			convertedVertices.reserve(vertices.size());

			for (unsigned int i = 0; i < vertices.size(); i++) {
				convertedVertices.push_back(Vertex{
					vertices[i],
					(normals.size() > i) ? normals[i] : glm::vec3(0.0f),
					(uvs.size() > i) ? uvs[i] : glm::vec2(0.0f),
					(tangent.size() > i) ? tangent[i] : glm::vec3(0.0f),
				});
			}
			GetVulkanRenderer().CreateVertexBuffer(convertedVertices, mVertexBuffer, mVertexBufferMemory);

			GetVulkanRenderer().CreateIndexBuffer(indices, mIndexBuffer, mIndexBufferMemory);
			firstRune = false;
		}

		if (!mIndexBuffer) {
		}

		VkBuffer vertexBuffers[] = {mVertexBuffer};
		VkDeviceSize offsets[] = {0};
		// vkCmdBindVertexBuffers(*((VulkanRenderer*)(Application::Get()->mRenderer))->mActiveCommandBuffer, 0, 1, {
		// &mVertexBuffer }, offsets);
		// vkCmdBindIndexBuffer(*((VulkanRenderer*)(Application::Get()->mRenderer))->mActiveCommandBuffer, mIndexBuffer,
		// 0, VK_INDEX_TYPE_UINT32);
		// vkCmdDrawIndexed(*((VulkanRenderer*)(Application::Get()->mRenderer))->mActiveCommandBuffer,
		// static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
	}

	void VulkanMesh::Draw() {
		vkCmdBindVertexBuffers(*((VulkanRenderer*)(Application::Get()->mRenderer))->mActiveCommandBuffer, 0, 1,
							   {&mVertexBuffer}, {0});
		vkCmdBindIndexBuffer(*((VulkanRenderer*)(Application::Get()->mRenderer))->mActiveCommandBuffer, mIndexBuffer, 0,
							 VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(*((VulkanRenderer*)(Application::Get()->mRenderer))->mActiveCommandBuffer,
						 static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
		;
	}

	void VulkanMesh::DrawInstances() {
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = sizeof(glm::mat4) * instanceModelMatrices.size();
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		void* data;
		vkMapMemory(GetVulkanRenderer().mDevice, mInstanceBufferMemory, 0, bufferInfo.size, 0, &data);
		memcpy(data, instanceModelMatrices.data(), static_cast<size_t>(bufferInfo.size));
		vkUnmapMemory(GetVulkanRenderer().mDevice, mInstanceBufferMemory);

		VkDeviceSize offsets[] = {0};
		VkCommandBuffer activeCommandBuffer = *GetVulkanRenderer().mActiveCommandBuffer;

		std::vector<VkDescriptorSet> descriptorSets = vector<VkDescriptorSet>();
		descriptorSets.push_back(GetVulkanRenderer().mDescriptorSets[GetVulkanRenderer().mCurrentFrame]);
		// VkDescriptorSet descriptorSets[] = { GetVulkanRenderer().mDescriptorSets[GetVulkanRenderer().mCurrentFrame]
		// };
		int descriptorCount = 1;

		VulkanRenderer::PushConstants pushData;
		// if (!this->material.diffuse->IsTextureEmpty()) {
		//	VulkanTexture* texture = (VulkanTexture*)this->material.diffuse;
		//	if (texture->mIndexHandle < 0)
		//		pushData.diffuseIndex = -1;
		//	else
		//		pushData.diffuseIndex = texture->mIndexHandle;
		// }
		// else {
		//	pushData.color = this->material.diffuse->rgba;
		// }

		vkCmdPushConstants(*VulkanRenderer::GetRenderer()->mActiveCommandBuffer,
						   VulkanRenderer::GetRenderer()->mPipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0,
						   sizeof(VulkanRenderer::PushConstants), &pushData);

		vkCmdBindDescriptorSets(activeCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
								GetVulkanRenderer().mPipelineLayout, 0, descriptorCount, descriptorSets.data(), 0,
								nullptr);
		vector<VkBuffer> verticesBuffer = {mVertexBuffer, mInstanceBuffer};
		vkCmdBindVertexBuffers(activeCommandBuffer, 0, 1, &mVertexBuffer, offsets);
		vkCmdBindVertexBuffers(activeCommandBuffer, 1, 1, &mInstanceBuffer, offsets);
		vkCmdBindIndexBuffer(activeCommandBuffer, mIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(activeCommandBuffer, static_cast<uint32_t>(indices.size()), instanceModelMatrices.size(), 0, 0,
						 0);
		instanceModelMatrices.clear();
	}

	VulkanRenderer& VulkanMesh::GetVulkanRenderer() { return *(VulkanRenderer*)Application::Get()->mRenderer; }

	VulkanMesh::~VulkanMesh() {
		VkDevice device = GetVulkanRenderer().mDevice;
		vkDestroyBuffer(device, mIndexBuffer, nullptr);
		vkFreeMemory(device, mIndexBufferMemory, nullptr);

		vkDestroyBuffer(device, mVertexBuffer, nullptr);
		vkFreeMemory(device, mVertexBufferMemory, nullptr);

		vkDestroyBuffer(device, mInstanceBuffer, nullptr);
		vkFreeMemory(device, mInstanceBufferMemory, nullptr);
	};

	void VulkanMesh::CreateInstanceBuffer() {
		for (int i = 0; i < 32; ++i) {
			this->instanceModelMatrices.push_back(glm::mat4(1.0f));
		}
		VkDeviceSize bufferSize = 32 * sizeof(glm::mat4);

		GetVulkanRenderer().CreateBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
										 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
										 mInstanceBuffer, mInstanceBufferMemory);

		instanceModelMatrices.clear();
	}

	void VulkanMesh::setupMesh() {
		vector<Vertex> convertedVertices;
		convertedVertices.reserve(vertices.size());

		for (unsigned int i = 0; i < vertices.size(); i++) {
			CalculateVertexInBoundingBox(vertices[i]);

			convertedVertices.push_back(Vertex{vertices[i], (normals.size() > i) ? normals[i] : glm::vec3(0.0f),
											   (uvs.size() > i) ? uvs[i] : glm::vec2(0.0f),
											   (tangent.size() > i) ? tangent[i] : glm::vec3(0.0f)});
		}

		GetVulkanRenderer().CreateVertexBuffer(convertedVertices, mVertexBuffer, mVertexBufferMemory);
		GetVulkanRenderer().CreateIndexBuffer(indices, mIndexBuffer, mIndexBufferMemory);
		this->CreateInstanceBuffer();
	}

	void VulkanMesh::Restart() { VulkanRenderer::GetRenderer()->RestartMesh(this); }
} // namespace Plaza
