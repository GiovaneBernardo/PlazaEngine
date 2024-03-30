#pragma once
#include <Engine/Core/Renderer/Picking.h>
#include "VulkanPostEffects.h"

namespace Plaza {
	class VulkanPicking : public Picking {
	public:
		struct PushConstants {
			glm::mat4 projection;
			glm::mat4 view;
			glm::mat4 model;
			unsigned int uuid1;
			unsigned int uuid2;
		} pushData;
		VulkanMesh* mSkyboxMesh = nullptr;
		std::vector<std::string> mSkyboxPaths = std::vector<std::string>(6);
		VkFormat mSkyboxFormat = VK_FORMAT_R8G8B8A8_UNORM;

		VulkanPostEffects* mRenderPickingTexturePostEffects = nullptr;
		VulkanPostEffects* mOutlinePostEffects = nullptr;
		std::vector<VkFramebuffer> mFramebuffers = std::vector<VkFramebuffer>();
		std::vector<VkDescriptorSet> mDescriptorSets = std::vector<VkDescriptorSet>();
		VkDescriptorSetLayout mDescriptorSetLayout = VK_NULL_HANDLE;
		VkPipelineLayoutCreateInfo mPipelineLayoutInfo{};
		VkCommandBuffer mCommandBuffer = VK_NULL_HANDLE;
		void Init() override;
		void DrawSelectedObjectsUuid() override;
		void DrawOutline() override;
		uint64_t ReadPickingTexture(glm::vec2 position) override;
		uint64_t DrawAndRead(glm::vec2 position) override;
		void Terminate() override;

		VkImageView mPickingTextureImageView = VK_NULL_HANDLE;
	private:
		VkFramebuffer mFramebuffer = VK_NULL_HANDLE;
		VkBuffer mStagingBuffer = VK_NULL_HANDLE;
		VkDeviceMemory mStagingBufferMemory = VK_NULL_HANDLE;

		VkImage mPickingTextureImage = VK_NULL_HANDLE;

		VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
		VkSampler mSkyboxSampler = VK_NULL_HANDLE;

		void InitializePicking();
		void InitializeOutline();
		void InitializeFramebuffer();
		void InitializeImageSampler();
		void InitializeImageView();
		void InitializeDescriptorPool();
		void InitializeDescriptorSets();
		void InitializeRenderPass();
		void UpdateAndPushConstants(VkCommandBuffer commandBuffer);
		void DrawMeshToPickingTexture(const MeshRenderer& meshRenderer, VkCommandBuffer& commandBuffer);
	};
}