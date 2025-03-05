#pragma once
#include "VulkanComputeShaders.h"
#include "VulkanTexture.h"

namespace Plaza {
	class VulkanBloom {
	  public:
		int mMipCount = 5;
		float mThreshold = 2.0f;
		float mKnee = 0.5f;
		float mBloomIntensity = 16.0f;
		float mBloomDirtIntensity = 1.0f;

		void Init();
		void Draw();
		void CopySceneTexture();
		void BlendBloomWithScene();
		void Terminate();
		void UpdateDescriptorSets();
		void UpdateUniformBuffers(glm::vec2 texelSize, unsigned int mipLevel, bool useThreshold);
		void UpdateDescriptorSet(VkImageLayout inputLayout, VkImageView inputView, VkSampler inputSampler,
								 VkImageView outputImageView, unsigned int frame, VkDescriptorSet& descriptorSet);
		void InitializeDescriptorSets();
		VulkanTexture* mTexture1 = nullptr;
		VulkanTexture* mTexture2 = nullptr;

	  private:
		uint8_t CalculateMipmapLevels(int m_width, int m_height, int m_max_iterations, int m_downscale_limit);
		std::vector<std::vector<VkDescriptorSet>> mDownScaleDescriptorSets =
			std::vector<std::vector<VkDescriptorSet>>();
		std::vector<std::vector<VkDescriptorSet>> mUpScaleDescriptorSets = std::vector<std::vector<VkDescriptorSet>>();

		struct PushConstant {
			glm::vec4 u_threshold;
			glm::vec2 u_texel_size;
			int u_mip_level;
			bool u_use_threshold;
		};
		VulkanComputeShaders mComputeShadersScaleDown;
		VulkanComputeShaders mComputeShadersScaleUp;
		VulkanComputeShaders mComputeShadersBlend;

		std::vector<VkBuffer> mUniformBuffers = std::vector<VkBuffer>();
		std::vector<VkDeviceMemory> mUniformBuffersMemory = std::vector<VkDeviceMemory>();
		std::vector<void*> mUniformBuffersMapped = std::vector<void*>();
	};
} // namespace Plaza
