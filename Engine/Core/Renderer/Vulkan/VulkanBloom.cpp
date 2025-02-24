#include "Engine/Core/PreCompiledHeaders.h"
#include "VulkanBloom.h"
#include "Engine/Core/Scene.h"
#include "Engine/Core/Renderer/Vulkan/Renderer.h"

namespace Plaza {
	uint8_t VulkanBloom::CalculateMipmapLevels(int m_width, int m_height, int m_max_iterations, int m_downscale_limit) {
		uint32_t width = m_width / 2;
		uint32_t height = m_height / 2;
		uint8_t  mip_levels = 1;

		printf("Mip level %d: %d x %d\n", 0, m_width, m_height);
		printf("Mip level %d: %d x %d\n", mip_levels, width, height);

		for (uint8_t i = 0; i < m_max_iterations; ++i) {
			width = width / 2;
			height = height / 2;

			if (width < m_downscale_limit || height < m_downscale_limit) break;

			++mip_levels;

			printf("Mip level %d: %d x %d\n", mip_levels, width, height);
		}

		return mip_levels + 1;
	}

	void VulkanBloom::InitializeDescriptorSets() {
		this->mDownScaleDescriptorSets.resize(this->mMipCount);
		this->mUpScaleDescriptorSets.resize(this->mMipCount);

		bool pingPong = true;
		for (unsigned int i = 0; i < this->mMipCount; ++i) {
			this->mDownScaleDescriptorSets[i].resize(Application::Get()->mRenderer->mMaxFramesInFlight);
			this->mUpScaleDescriptorSets[i].resize(Application::Get()->mRenderer->mMaxFramesInFlight);

			std::array<VkDescriptorPoolSize, 2> poolSizes{};
			poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSizes[0].descriptorCount = static_cast<uint32_t>(Application::Get()->mRenderer->mMaxFramesInFlight);

			poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			poolSizes[1].descriptorCount = static_cast<uint32_t>(Application::Get()->mRenderer->mMaxFramesInFlight) * 2;

			VkDescriptorPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.poolSizeCount = 2;
			poolInfo.pPoolSizes = poolSizes.data();
			poolInfo.maxSets = static_cast<uint32_t>(Application::Get()->mRenderer->mMaxFramesInFlight);

			VkDescriptorPool descriptorPool;
			if (vkCreateDescriptorPool(VulkanRenderer::GetRenderer()->mDevice, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
				throw std::runtime_error("failed to create descriptor pool!");
			}

			std::vector<VkDescriptorSetLayout> layouts(Application::Get()->mRenderer->mMaxFramesInFlight, mComputeShadersScaleDown.mComputeDescriptorSetLayout);
			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = descriptorPool;
			allocInfo.descriptorSetCount = static_cast<uint32_t>(Application::Get()->mRenderer->mMaxFramesInFlight);
			allocInfo.pSetLayouts = layouts.data();

			if (vkAllocateDescriptorSets(VulkanRenderer::GetRenderer()->mDevice, &allocInfo, this->mDownScaleDescriptorSets[i].data()) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate descriptor sets!");
			}

			VkDescriptorPool descriptorPool2;
			if (vkCreateDescriptorPool(VulkanRenderer::GetRenderer()->mDevice, &poolInfo, nullptr, &descriptorPool2) != VK_SUCCESS) {
				throw std::runtime_error("failed to create descriptor pool!");
			}
			allocInfo.descriptorPool = descriptorPool2;
			if (vkAllocateDescriptorSets(VulkanRenderer::GetRenderer()->mDevice, &allocInfo, this->mUpScaleDescriptorSets[i].data()) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate descriptor sets!");
			}

			VkImageLayout inputLayout = VK_IMAGE_LAYOUT_GENERAL;
			VkImageView inputView = pingPong ? this->mTexture1->mImageView : this->mTexture1->mImageView;
			VkSampler inputSampler = pingPong ? this->mTexture1->mSampler : this->mTexture1->mSampler;
			VkImageView outputImageView = VK_NULL_HANDLE; //= pingPong ? this->mTexture1->mImageView : this->mTexture1->mImageView;

			/* Downscale */
			if (i == 0) {
				inputLayout = VK_IMAGE_LAYOUT_GENERAL;
				inputView = this->mTexture1->mImageView;
				inputSampler = this->mTexture1->mSampler;
				//outputImageView = this->mTexture1->mImageView;

				VkImageViewCreateInfo viewInfo = {};
				viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				viewInfo.image = pingPong ? this->mTexture1->mImage : this->mTexture1->mImage; // Your VkImage object
				viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				viewInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT; // Your image format
				viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				viewInfo.subresourceRange.baseMipLevel = 1; // Specify the mip level
				viewInfo.subresourceRange.levelCount = 1;
				viewInfo.subresourceRange.baseArrayLayer = 0;
				viewInfo.subresourceRange.layerCount = 1;

				//vkCreateImageView(VulkanRenderer::GetRenderer()->mDevice, &viewInfo, nullptr, &inputView);
				vkCreateImageView(VulkanRenderer::GetRenderer()->mDevice, &viewInfo, nullptr, &outputImageView);
			}
			else if (i < this->mMipCount - 1) {
				VkImageViewCreateInfo viewInfo = {};
				viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				viewInfo.image = pingPong ? this->mTexture1->mImage : this->mTexture1->mImage; // Your VkImage object
				viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				viewInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT; // Your image format
				viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				viewInfo.subresourceRange.baseMipLevel = i; // Specify the mip level
				viewInfo.subresourceRange.levelCount = 1;
				viewInfo.subresourceRange.baseArrayLayer = 0;
				viewInfo.subresourceRange.layerCount = 1;

				vkCreateImageView(VulkanRenderer::GetRenderer()->mDevice, &viewInfo, nullptr, &inputView);

				viewInfo.image = pingPong ? this->mTexture1->mImage : this->mTexture1->mImage;
				viewInfo.subresourceRange.baseMipLevel = i + 1;
				vkCreateImageView(VulkanRenderer::GetRenderer()->mDevice, &viewInfo, nullptr, &outputImageView);
			}

			if (i < this->mMipCount - 1) {
				for (unsigned int j = 0; j < Application::Get()->mRenderer->mMaxFramesInFlight; ++j) {
					UpdateDescriptorSet(inputLayout, inputView, inputSampler, outputImageView, j, this->mDownScaleDescriptorSets[i][j]);
				}

			}

			/* Upscale */

			if (i == 0) {
				inputLayout = VK_IMAGE_LAYOUT_GENERAL;
				inputView = this->mTexture1->mImageView;//VulkanRenderer::GetRenderer()->mDeferredFinalImageView;
				inputSampler = this->mTexture1->mSampler;
				//outputImageView = this->mTexture1->mImageView;

				VkImageViewCreateInfo viewInfo = {};
				viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				viewInfo.image = pingPong ? this->mTexture1->mImage : this->mTexture1->mImage; // Your VkImage object
				viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				viewInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT; // Your image format
				viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				viewInfo.subresourceRange.baseMipLevel = i; // Specify the mip level
				viewInfo.subresourceRange.levelCount = 1;
				viewInfo.subresourceRange.baseArrayLayer = 0;
				viewInfo.subresourceRange.layerCount = 1;
				viewInfo.image = pingPong ? this->mTexture1->mImage : this->mTexture1->mImage;
				viewInfo.subresourceRange.baseMipLevel = 0;
				vkCreateImageView(VulkanRenderer::GetRenderer()->mDevice, &viewInfo, nullptr, &outputImageView);
			}
			else {
				VkImageViewCreateInfo viewInfo = {};
				viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				viewInfo.image = pingPong ? this->mTexture1->mImage : this->mTexture1->mImage; // Your VkImage object
				viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				viewInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT; // Your image format
				viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				viewInfo.subresourceRange.baseMipLevel = i; // Specify the mip level
				viewInfo.subresourceRange.levelCount = 1;
				viewInfo.subresourceRange.baseArrayLayer = 0;
				viewInfo.subresourceRange.layerCount = 1;

				vkCreateImageView(VulkanRenderer::GetRenderer()->mDevice, &viewInfo, nullptr, &inputView);

				viewInfo.image = pingPong ? this->mTexture1->mImage : this->mTexture1->mImage;
				viewInfo.subresourceRange.baseMipLevel = i - 1;
				vkCreateImageView(VulkanRenderer::GetRenderer()->mDevice, &viewInfo, nullptr, &outputImageView);


			}
			for (unsigned int j = 0; j < Application::Get()->mRenderer->mMaxFramesInFlight; ++j) {
				UpdateDescriptorSet(inputLayout, inputView, inputSampler, outputImageView, j, this->mUpScaleDescriptorSets[i][j]);
			}


			if (i > 0)
				pingPong = !pingPong;
		}
	}

	void VulkanBloom::UpdateDescriptorSet(VkImageLayout inputLayout, VkImageView inputView, VkSampler inputSampler, VkImageView outputImageView, unsigned int frame, VkDescriptorSet& descriptorSet) {
		//VulkanRenderer::GetRenderer()->TransitionImageLayout(VulkanRenderer::GetRenderer()->mFinalSceneImage, VulkanRenderer::GetRenderer()->mSwapChainImageFormat, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = inputLayout; // Assuming this layout is appropriate
		imageInfo.imageView = inputView;//this->mTexture1->mImageView;
		imageInfo.sampler = inputSampler;//this->mTexture1->mSampler;

		mComputeShadersScaleDown.mComputeDescriptorSets.resize(Application::Get()->mRenderer->mMaxFramesInFlight);
		mUniformBuffers.resize(Application::Get()->mRenderer->mMaxFramesInFlight);

		mComputeShadersScaleDown.mDescriptorWrites.resize(3);
		mComputeShadersScaleDown.mDescriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		mComputeShadersScaleDown.mDescriptorWrites[0].dstSet = descriptorSet;
		mComputeShadersScaleDown.mDescriptorWrites[0].dstBinding = 0;
		mComputeShadersScaleDown.mDescriptorWrites[0].dstArrayElement = 0;
		mComputeShadersScaleDown.mDescriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		mComputeShadersScaleDown.mDescriptorWrites[0].descriptorCount = 1;
		mComputeShadersScaleDown.mDescriptorWrites[0].pImageInfo = &imageInfo;

		VkDescriptorImageInfo storageImageInfo{};
		storageImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL; // Assuming this layout is appropriate
		storageImageInfo.imageView = outputImageView;

		mComputeShadersScaleDown.mDescriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		mComputeShadersScaleDown.mDescriptorWrites[1].dstSet = descriptorSet;
		mComputeShadersScaleDown.mDescriptorWrites[1].dstBinding = 1;
		mComputeShadersScaleDown.mDescriptorWrites[1].dstArrayElement = 0;
		mComputeShadersScaleDown.mDescriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		mComputeShadersScaleDown.mDescriptorWrites[1].descriptorCount = 1;
		mComputeShadersScaleDown.mDescriptorWrites[1].pImageInfo = &storageImageInfo;

		VkDescriptorImageInfo sceneImageInfo{};
		sceneImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		sceneImageInfo.imageView = VulkanRenderer::GetRenderer()->mLighting->mDeferredEndTexture.mImageView;//VulkanRenderer::GetRenderer()->mDeferredFinalImageView;
		sceneImageInfo.sampler = this->mTexture1->mSampler;
		mComputeShadersScaleDown.mDescriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		mComputeShadersScaleDown.mDescriptorWrites[2].dstSet = descriptorSet;
		mComputeShadersScaleDown.mDescriptorWrites[2].dstBinding = 2;
		mComputeShadersScaleDown.mDescriptorWrites[2].dstArrayElement = 0;
		mComputeShadersScaleDown.mDescriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		mComputeShadersScaleDown.mDescriptorWrites[2].descriptorCount = 1;
		mComputeShadersScaleDown.mDescriptorWrites[2].pImageInfo = &sceneImageInfo;

		vkUpdateDescriptorSets(VulkanRenderer::GetRenderer()->mDevice, mComputeShadersScaleDown.mDescriptorWrites.size(), mComputeShadersScaleDown.mDescriptorWrites.data(), 0, nullptr);
	}

	void VulkanBloom::UpdateDescriptorSets() {

	}

	void VulkanBloom::Init() {
		this->mMipCount = this->CalculateMipmapLevels(Application::Get()->appSizes->sceneSize.x, Application::Get()->appSizes->sceneSize.y, 16, 10);
		this->mTexture1 = new VulkanTexture();
		this->mTexture1->mMipCount = this->mMipCount;
		this->mTexture1->CreateTextureImage(VulkanRenderer::GetRenderer()->mDevice, VK_FORMAT_R32G32B32A32_SFLOAT, Application::Get()->appSizes->sceneSize.x, Application::Get()->appSizes->sceneSize.y, true, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
		VulkanRenderer::GetRenderer()->TransitionImageLayout(mTexture1->mImage, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT, 1, mTexture1->mMipCount);
		this->mTexture1->CreateTextureSampler(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_FILTER_LINEAR, VK_FILTER_LINEAR);
		this->mTexture1->CreateImageView(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D);

		//this->mTexture1->InitDescriptorSetLayout();

		//this->mTexture2->InitDescriptorSetLayout();

		VulkanRenderer::GetRenderer()->AddTrackerToImage(this->mTexture1->mImage, "Bloom Downscale", this->mTexture1->mSampler,this->mTexture1->GetTextureInfo(), VK_IMAGE_LAYOUT_GENERAL);

		std::array<VkDescriptorSetLayoutBinding, 3> layoutBindings{};
		layoutBindings[0].binding = 0;
		layoutBindings[0].descriptorCount = 1;
		layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layoutBindings[0].pImmutableSamplers = nullptr;
		layoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		layoutBindings[0].stageFlags |= VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;

		layoutBindings[1].binding = 1;
		layoutBindings[1].descriptorCount = 1;
		layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		layoutBindings[1].pImmutableSamplers = nullptr;
		layoutBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		layoutBindings[1].stageFlags |= VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;

		layoutBindings[2].binding = 2;
		layoutBindings[2].descriptorCount = 1;
		layoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layoutBindings[2].pImmutableSamplers = nullptr;
		layoutBindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		layoutBindings[2].stageFlags |= VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 3;
		layoutInfo.pBindings = layoutBindings.data();
		//layoutInfo.flags = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;

		if (vkCreateDescriptorSetLayout(VulkanRenderer::GetRenderer()->mDevice, &layoutInfo, nullptr, &this->mComputeShadersScaleDown.mComputeDescriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create compute descriptor set layout!");
		}

		if (vkCreateDescriptorSetLayout(VulkanRenderer::GetRenderer()->mDevice, &layoutInfo, nullptr, &this->mComputeShadersScaleUp.mComputeDescriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create compute descriptor set layout!");
		}

		UpdateDescriptorSets();
		InitializeDescriptorSets();

		VkPushConstantRange pushConstantRange = {};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT; // Specify the shader stage(s) using the push constant
		pushConstantRange.offset = 0; // Offset of the push constant block
		pushConstantRange.size = sizeof(PushConstant); // Size of the push constant block

		this->mComputeShadersScaleDown.Init(Application::Get()->enginePath + "/Shaders/Vulkan/bloom/bloomDownScale.comp", { pushConstantRange });
		this->mComputeShadersScaleUp.Init(Application::Get()->enginePath + "/Shaders/Vulkan/bloom/bloomUpScale.comp", { pushConstantRange });
	}

	void VulkanBloom::UpdateUniformBuffers(glm::vec2 texelSize, unsigned int mipLevel, bool useThreshold) {
		float mThreshold = 0.3f;
		float mKnee = 0.1f;
		float m_bloom_intensity = 16.0f;
		float m_bloom_dirt_intensity = 1.0f;

	}

	void VulkanBloom::Draw() {
		PLAZA_PROFILE_SECTION("Bloom");
		this->CopySceneTexture();

		/* Bloom: downscale */
		glm::uvec2 mipSize = glm::uvec2(Application::Get()->appSizes->sceneSize.x / 2, Application::Get()->appSizes->sceneSize.y / 2);

		for (uint8_t i = 0; i < mMipCount - 1; ++i) {
			PushConstant pushConstant{};
			pushConstant.u_texel_size = 1.0f / glm::vec2(mipSize);
			pushConstant.u_mip_level = i;
			pushConstant.u_threshold = glm::vec4(mThreshold, mThreshold - mKnee, 2.0f * mKnee, 0.25f * mKnee);
			pushConstant.u_use_threshold = i == 0;

			this->mComputeShadersScaleDown.Dispatch(glm::ceil(float(mipSize.x) / 8), glm::ceil(float(mipSize.y) / 8), 1, &pushConstant, sizeof(PushConstant), this->mDownScaleDescriptorSets[i][Application::Get()->mRenderer->mCurrentFrame]);

			VkImageMemoryBarrier imageMemoryBarrier = {};
			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
			imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
			imageMemoryBarrier.image = this->mTexture1->mImage;
			imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
			imageMemoryBarrier.subresourceRange.levelCount = this->mTexture1->mMipCount;
			imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
			imageMemoryBarrier.subresourceRange.layerCount = 1;

			imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			vkCmdPipelineBarrier(*VulkanRenderer::GetRenderer()->mActiveCommandBuffer,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &imageMemoryBarrier);

			mipSize = mipSize / 2u;
		}

		/* Bloom: upscale */
		for (uint8_t i = mMipCount - 1; i >= 1; --i) {
			mipSize.x = glm::max(1.0, glm::floor(float(Application::Get()->appSizes->sceneSize.x) / glm::pow(2.0, i - 1)));
			mipSize.y = glm::max(1.0, glm::floor(float(Application::Get()->appSizes->sceneSize.y) / glm::pow(2.0, i - 1)));


			PushConstant pushConstant{};
			pushConstant.u_texel_size = 1.0f / glm::vec2(mipSize);
			pushConstant.u_mip_level = i;
			pushConstant.u_threshold = glm::vec4(mThreshold, mThreshold - mKnee, 2.0f * mKnee, 0.25f * mKnee);
			pushConstant.u_use_threshold = i == 0;

			this->mComputeShadersScaleUp.Dispatch(glm::ceil(float(mipSize.x) / 8), glm::ceil(float(mipSize.y) / 8), 1, &pushConstant, sizeof(PushConstant), this->mUpScaleDescriptorSets[i][Application::Get()->mRenderer->mCurrentFrame]);

			VkImageMemoryBarrier imageMemoryBarrier = {};
			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
			imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
			imageMemoryBarrier.image = this->mTexture1->mImage;
			imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
			imageMemoryBarrier.subresourceRange.levelCount = this->mTexture1->mMipCount;
			imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
			imageMemoryBarrier.subresourceRange.layerCount = 1;

			imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			vkCmdPipelineBarrier(*VulkanRenderer::GetRenderer()->mActiveCommandBuffer,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &imageMemoryBarrier);
		}
	}

	void VulkanBloom::CopySceneTexture() {
		PLAZA_PROFILE_SECTION("Copy Scene Texture");
		VkImageSubresourceRange subresourceRange{};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.layerCount = 1;

		VkImageCopy imageCopyRegion{};
		imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageCopyRegion.srcSubresource.mipLevel = 0;
		imageCopyRegion.srcSubresource.baseArrayLayer = 0;
		imageCopyRegion.srcSubresource.layerCount = 1;
		imageCopyRegion.srcOffset = { 0, 0, 0 };

		imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageCopyRegion.dstSubresource.mipLevel = 0;
		imageCopyRegion.dstSubresource.baseArrayLayer = 0;
		imageCopyRegion.dstSubresource.layerCount = 1;
		imageCopyRegion.dstOffset = { 0, 0, 0 };

		imageCopyRegion.extent.width = Application::Get()->appSizes->sceneSize.x;
		imageCopyRegion.extent.height = Application::Get()->appSizes->sceneSize.y;
		imageCopyRegion.extent.depth = 1;

		{
			PLAZA_PROFILE_SECTION("Transition 1");
			VulkanRenderer::GetRenderer()->TransitionImageLayout(VulkanRenderer::GetRenderer()->mLighting->mDeferredEndTexture.mImage, VulkanRenderer::GetRenderer()->mFinalDeferredFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		}
		{
			PLAZA_PROFILE_SECTION("Transition 2");
			VulkanRenderer::GetRenderer()->TransitionImageLayout(this->mTexture1->mImage, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		}

		VkCommandBuffer commandBuffer = VulkanRenderer::GetRenderer()->BeginSingleTimeCommands();
		{
			PLAZA_PROFILE_SECTION("Copy");
			vkCmdCopyImage(
				commandBuffer,
				VulkanRenderer::GetRenderer()->mLighting->mDeferredEndTexture.mImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				this->mTexture1->mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &imageCopyRegion
			);
		}
		VulkanRenderer::GetRenderer()->EndSingleTimeCommands(commandBuffer);

		//VulkanRenderer::GetRenderer()->TransitionImageLayout(VulkanRenderer::GetRenderer()->mDeferredFinalImage, VulkanRenderer::GetRenderer()->mFinalDeferredFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		{
			PLAZA_PROFILE_SECTION("Transition 3");
			VulkanRenderer::GetRenderer()->TransitionImageLayout(this->mTexture1->mImage, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
		}
	}

	void VulkanBloom::BlendBloomWithScene() {

	}
	void VulkanBloom::Terminate() {

	}
}