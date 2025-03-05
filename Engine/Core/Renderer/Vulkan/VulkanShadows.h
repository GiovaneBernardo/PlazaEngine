#pragma once
#include "Engine/Core/Renderer/Shadows.h"
#include "Engine/Core/Renderer/Vulkan/VulkanShaders.h"
#include "VulkanTexture.h"

namespace Plaza {
	class VulkanShadows : public Shadows {
	  public:
		unsigned int mCascadeCount = 9;

		struct PushConstants {
			glm::vec4 position;
			unsigned int cascadeIndex;
		};
		VulkanShadows::PushConstants pushConstants{};

		struct Cascade {
			std::vector<VkDescriptorSet> mDescriptorSets = std::vector<VkDescriptorSet>();
			VkImageView mImageView;
			VkFramebuffer mFramebuffer;
		};
		std::vector<Cascade> mCascades = std::vector<Cascade>();
		struct ShadowsUniformBuffer {
			glm::mat4 lightSpaceMatrices[32];
		};
		std::vector<ShadowsUniformBuffer> mUbo = std::vector<ShadowsUniformBuffer>();

		struct ShadowDepthBuffer {
			VulkanTexture texture;
			VkFramebuffer mFramebuffer;
		};

		VkFormat mDepthFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;

		std::vector<ShadowDepthBuffer> shadowsBuffers = std::vector<ShadowDepthBuffer>();
		std::vector<float> shadowCascadeLevels;

		unsigned int mDepthDebugLayer = 0;
		VulkanShaders* mDepthDebugShaders = nullptr;
		VkRenderPass mDepthDebugRenderPass = VK_NULL_HANDLE;
		VkFramebuffer mDepthDebugFramebuffer = VK_NULL_HANDLE;
		VkPipelineLayout mDebugDepthPipelineLayout = VK_NULL_HANDLE;
		VkPipeline mDebugDepthPipeline = VK_NULL_HANDLE;
		VkImageView mDebugDepthImageView = VK_NULL_HANDLE;
		VkDescriptorSetLayout mDebugDepthDescriptorLayout = VK_NULL_HANDLE;
		VkDescriptorSet mDebugDepthDescriptorSet = VK_NULL_HANDLE;
		VkImage mDebugDepthImage = VK_NULL_HANDLE;

		unsigned int shadowBufferCount = 5;
		VulkanShaders* mShadowsShader = nullptr;
		;
		std::vector<VkBuffer> mUniformBuffers = std::vector<VkBuffer>();
		std::vector<VkDeviceMemory> mUniformBuffersMemory = std::vector<VkDeviceMemory>();
		std::vector<void*> mUniformBuffersMapped = std::vector<void*>();

		VkRenderPass mRenderPass = VK_NULL_HANDLE;
		VkSampler mShadowsSampler = VK_NULL_HANDLE;
		VkImage mShadowDepthImage = VK_NULL_HANDLE;
		std::vector<VkImageView> mShadowDepthImageViews = std::vector<VkImageView>();
		std::vector<VkFramebuffer> mFramebuffers = std::vector<VkFramebuffer>();

		std::vector<VkDescriptorSet> mDescriptorSets = std::vector<VkDescriptorSet>();
		VkDescriptorSetLayout mDescriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
		void Init() override;
		void RenderToShadowMap() override;
		void Terminate() override;
		void UpdateUniformBuffer(unsigned int frameIndex);

		void InitializeBuffers(VulkanRenderer& renderer);
		void InitializeRenderPass(VulkanRenderer& renderer);
		void CreateDescriptorPool(VkDevice device);
		void CreateDescriptorSetLayout(VkDevice device);
		void CreateDescriptorSet(VkDevice device);
		void UpdateAndPushConstants(VkCommandBuffer commandBuffer, unsigned int cascadeIndex);

		static glm::mat4 GetLightSpaceMatrix(const float nearPlane, const float farPlane, const float ratio,
											 const glm::mat4& viewMatrix, const glm::vec3& lightDirection);
		static std::vector<glm::mat4> GetLightSpaceMatrices(std::vector<float> shadowCascadeLevels,
															const glm::vec3& lightDirection);
	};
} // namespace Plaza
