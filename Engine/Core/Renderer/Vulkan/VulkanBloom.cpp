#include "Engine/Core/PreCompiledHeaders.h"
#include "VulkanBloom.h"
namespace Plaza {
	uint8_t VulkanBloom::CalculateMipmapLevels(int m_width, int m_height, int m_max_iterations, int m_downscale_limit)
	{
		uint32_t width = m_width / 2;
		uint32_t height = m_height / 2;
		uint8_t  mip_levels = 1;

		printf("Mip level %d: %d x %d\n", 0, m_width, m_height);
		printf("Mip level %d: %d x %d\n", mip_levels, width, height);

		for (uint8_t i = 0; i < m_max_iterations; ++i)
		{
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
			this->mDownScaleDescriptorSets[i].resize(Application->mRenderer->mMaxFramesInFlight);
			this->mUpScaleDescriptorSets[i].resize(Application->mRenderer->mMaxFramesInFlight);

			std::array<VkDescriptorPoolSize, 2> poolSizes{};
			poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSizes[0].descriptorCount = static_cast<uint32_t>(Application->mRenderer->mMaxFramesInFlight);

			poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			poolSizes[1].descriptorCount = static_cast<uint32_t>(Application->mRenderer->mMaxFramesInFlight) * 2;

			VkDescriptorPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.poolSizeCount = 2;
			poolInfo.pPoolSizes = poolSizes.data();
			poolInfo.maxSets = static_cast<uint32_t>(Application->mRenderer->mMaxFramesInFlight);

			VkDescriptorPool descriptorPool;
			if (vkCreateDescriptorPool(VulkanRenderer::GetRenderer()->mDevice, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
				throw std::runtime_error("failed to create descriptor pool!");
			}

			std::vector<VkDescriptorSetLayout> layouts(Application->mRenderer->mMaxFramesInFlight, mComputeShadersScaleDown.mComputeDescriptorSetLayout);
			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = descriptorPool;
			allocInfo.descriptorSetCount = static_cast<uint32_t>(Application->mRenderer->mMaxFramesInFlight);
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
				inputLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				inputView = this->mTexture1->mImageView;//VulkanRenderer::GetRenderer()->mDeferredFinalImageView;
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
				viewInfo.subresourceRange.baseMipLevel = i + 1;
				vkCreateImageView(VulkanRenderer::GetRenderer()->mDevice, &viewInfo, nullptr, &outputImageView);
			}

			for (unsigned int j = 0; j < Application->mRenderer->mMaxFramesInFlight; ++j) {
				UpdateDescriptorSet(inputLayout, inputView, inputSampler, outputImageView, j, this->mDownScaleDescriptorSets[i][j]);
			}

			/* Upscale */
			if (i == 0) {
				inputLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
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
			for (unsigned int j = 0; j < Application->mRenderer->mMaxFramesInFlight; ++j) {
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

		mComputeShadersScaleDown.mComputeDescriptorSets.resize(Application->mRenderer->mMaxFramesInFlight);
		mUniformBuffers.resize(Application->mRenderer->mMaxFramesInFlight);

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
		sceneImageInfo.imageView = VulkanRenderer::GetRenderer()->mDeferredFinalImageView;
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
		this->mMipCount = this->CalculateMipmapLevels(Application->appSizes->sceneSize.x, Application->appSizes->sceneSize.y, 16, 10);
		this->mTexture1 = new VulkanTexture();
		this->mTexture1->mMipLevels = this->mMipCount;
		this->mTexture1->CreateTextureImage(VulkanRenderer::GetRenderer()->mDevice, VK_FORMAT_R32G32B32A32_SFLOAT, Application->appSizes->sceneSize.x, Application->appSizes->sceneSize.y, true);
		this->mTexture1->CreateTextureSampler(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_FILTER_LINEAR, VK_FILTER_LINEAR);
		this->mTexture1->CreateImageView(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D);
		//this->mTexture1->InitDescriptorSetLayout();

		this->mTexture2 = new VulkanTexture();
		this->mTexture2->mMipLevels = this->mMipCount;
		this->mTexture2->CreateTextureImage(VulkanRenderer::GetRenderer()->mDevice, VK_FORMAT_R32G32B32A32_SFLOAT, Application->appSizes->sceneSize.x, Application->appSizes->sceneSize.y, true);
		this->mTexture2->CreateTextureSampler(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_FILTER_NEAREST, VK_FILTER_NEAREST);
		this->mTexture2->CreateImageView(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);
		//this->mTexture2->InitDescriptorSetLayout();

		VulkanRenderer::GetRenderer()->AddTrackerToImage(this->mTexture1->mImageView, "Bloom Downscale", this->mTexture1->mSampler, VK_IMAGE_LAYOUT_GENERAL);
		VulkanRenderer::GetRenderer()->AddTrackerToImage(this->mTexture2->mImageView, "Bloom Upscale", this->mTexture2->mSampler, VK_IMAGE_LAYOUT_GENERAL);

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

		this->mComputeShadersScaleDown.Init(Application->enginePath + "\\Shaders\\Vulkan\\bloom\\bloomDownScale.comp", 1, pushConstantRange);
		this->mComputeShadersScaleUp.Init(Application->enginePath + "\\Shaders\\Vulkan\\bloom\\bloomUpScale.comp", 1, pushConstantRange);
	}

	void VulkanBloom::UpdateUniformBuffers(glm::vec2 texelSize, unsigned int mipLevel, bool useThreshold) {
		float mThreshold = 0.3f;
		float mKnee = 0.1f;
		float m_bloom_intensity = 16.0f;
		float m_bloom_dirt_intensity = 1.0f;

	}

	void VulkanBloom::Draw() {
		/* Bloom: downscale */
		glm::uvec2 mipSize = glm::uvec2(Application->appSizes->sceneSize.x / 2, Application->appSizes->sceneSize.y / 2);
		//VulkanRenderer::GetRenderer()->TransitionImageLayout(VulkanRenderer::GetRenderer()->mFinalSceneImage, VulkanRenderer::GetRenderer()->mSwapChainImageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		//VulkanRenderer::GetRenderer()->TransitionImageLayout(VulkanRenderer::GetRenderer()->mFinalSceneImage, VulkanRenderer::GetRenderer()->mSwapChainImageFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		 //UpdateDescriptorSet(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VulkanRenderer::GetRenderer()->mFinalSceneImageView, VulkanRenderer::GetRenderer()->mTextureSampler, this->mTexture1->mImageView, Application->mRenderer->mCurrentFrame);
		// UpdateDescriptorSet(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VulkanRenderer::GetRenderer()->mFinalSceneImageView, VulkanRenderer::GetRenderer()->mTextureSampler, this->mTexture2->mImageView, Application->mRenderer->mCurrentFrame);
		//UpdateDescriptorSet(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VulkanRenderer::GetRenderer()->mFinalSceneImageView, VulkanRenderer::GetRenderer()->mTextureSampler, this->mTexture2->mImageView, i);
		std::cout << "Start \n";
		//VulkanRenderer::GetRenderer()->TransitionImageLayout(VulkanRenderer::GetRenderer()->mDeferredFinalImage, VulkanRenderer::GetRenderer()->mSwapChainImageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		this->CopySceneTexture();

		bool useFirstImage = false;
		for (uint8_t i = 0; i < mMipCount - 1; ++i)
		{
			//glBindImageTexture(0, mFinalTexturePair->texture1.id, i + 1, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
			//UpdateUniformBuffers(1.0f / glm::vec2(mipSize), i, i == 0);

			PushConstant pushConstant{};
			pushConstant.u_texel_size = 1.0f / glm::vec2(mipSize);
			pushConstant.u_mip_level = i;
			pushConstant.u_threshold = glm::vec4(mThreshold, mThreshold - mKnee, 2.0f * mKnee, 0.25f * mKnee);
			pushConstant.u_use_threshold = i == 0;

			this->mComputeShadersScaleDown.Dispatch(glm::ceil(float(mipSize.x) / 8), glm::ceil(float(mipSize.y) / 8), 1, &pushConstant, sizeof(PushConstant), this->mDownScaleDescriptorSets[i][Application->mRenderer->mCurrentFrame]);

			VkImageMemoryBarrier imageMemoryBarrier = {};
			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL; // Previous layout used by the image
			imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL; // New layout to transition the image to
			imageMemoryBarrier.image = VulkanRenderer::GetRenderer()->mDeferredFinalImage;//this->mTexture2->mImage; // The image to transition
			imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // Aspect of the image to transition
			imageMemoryBarrier.subresourceRange.baseMipLevel = 0; // Base mip level
			imageMemoryBarrier.subresourceRange.levelCount = this->mTexture2->mMipLevels; // Number of mip levels
			imageMemoryBarrier.subresourceRange.baseArrayLayer = 0; // Base array layer
			imageMemoryBarrier.subresourceRange.layerCount = 1; // Number of array layers

			// Specify the access masks and pipeline stages for the transition
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT; // Access mask for the compute shader write
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // Access mask for subsequent shader read
			imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // Queue family indices
			imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			// Insert pipeline barrier
			vkCmdPipelineBarrier(*VulkanRenderer::GetRenderer()->mActiveCommandBuffer,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, // Compute shader stage
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, // Subsequent shader stage
				0, // Dependency flags
				0, nullptr, // Memory barriers
				0, nullptr, // Buffer memory barriers
				1, &imageMemoryBarrier); // Image memory barriers

			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL; // Previous layout used by the image
			imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL; // New layout to transition the image to
			imageMemoryBarrier.image = this->mTexture1->mImage; // The image to transition
			imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // Aspect of the image to transition
			imageMemoryBarrier.subresourceRange.baseMipLevel = 0; // Base mip level
			imageMemoryBarrier.subresourceRange.levelCount = this->mTexture1->mMipLevels; // Number of mip levels
			imageMemoryBarrier.subresourceRange.baseArrayLayer = 0; // Base array layer
			imageMemoryBarrier.subresourceRange.layerCount = 1; // Number of array layers

			// Specify the access masks and pipeline stages for the transition
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT; // Access mask for the compute shader write
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // Access mask for subsequent shader read
			imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // Queue family indices
			imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			// Insert pipeline barrier
			vkCmdPipelineBarrier(*VulkanRenderer::GetRenderer()->mActiveCommandBuffer,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, // Compute shader stage
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, // Subsequent shader stage
				0, // Dependency flags
				0, nullptr, // Memory barriers
				0, nullptr, // Buffer memory barriers
				1, &imageMemoryBarrier); // Image memory barriers

			mipSize = mipSize / 2u;
		}

		/* Bloom: upscale */
		//mBloomUpScaleShader->use();
		//mBloomUpScaleShader->setFloat("u_bloom_intensity", m_bloom_intensity);
		//glBindTextureUnit(0, mFinalTexturePair->texture1.id);

		for (uint8_t i = mMipCount - 1; i >= 1; --i)
		{
			mipSize.x = glm::max(1.0, glm::floor(float(Application->appSizes->sceneSize.x) / glm::pow(2.0, i - 1)));
			mipSize.y = glm::max(1.0, glm::floor(float(Application->appSizes->sceneSize.y) / glm::pow(2.0, i - 1)));


			PushConstant pushConstant{};
			pushConstant.u_texel_size = 1.0f / glm::vec2(mipSize);
			pushConstant.u_mip_level = i;
			pushConstant.u_threshold = glm::vec4(mThreshold, mThreshold - mKnee, 2.0f * mKnee, 0.25f * mKnee);
			pushConstant.u_use_threshold = i == 0;

			this->mComputeShadersScaleUp.Dispatch(glm::ceil(float(mipSize.x) / 8), glm::ceil(float(mipSize.y) / 8), 1, &pushConstant, sizeof(PushConstant), this->mUpScaleDescriptorSets[i][Application->mRenderer->mCurrentFrame]);

			VkImageMemoryBarrier imageMemoryBarrier = {};
			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL; // Previous layout used by the image
			imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL; // New layout to transition the image to
			imageMemoryBarrier.image = VulkanRenderer::GetRenderer()->mDeferredFinalImage; // The image to transition
			imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // Aspect of the image to transition
			imageMemoryBarrier.subresourceRange.baseMipLevel = 0; // Base mip level
			imageMemoryBarrier.subresourceRange.levelCount = this->mTexture2->mMipLevels; // Number of mip levels
			imageMemoryBarrier.subresourceRange.baseArrayLayer = 0; // Base array layer
			imageMemoryBarrier.subresourceRange.layerCount = 1; // Number of array layers

			// Specify the access masks and pipeline stages for the transition
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT; // Access mask for the compute shader write
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // Access mask for subsequent shader read
			imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // Queue family indices
			imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			// Insert pipeline barrier
			vkCmdPipelineBarrier(*VulkanRenderer::GetRenderer()->mActiveCommandBuffer,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, // Compute shader stage
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, // Subsequent shader stage
				0, // Dependency flags
				0, nullptr, // Memory barriers
				0, nullptr, // Buffer memory barriers
				1, &imageMemoryBarrier); // Image memory barriers

			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL; // Previous layout used by the image
			imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL; // New layout to transition the image to
			imageMemoryBarrier.image = this->mTexture1->mImage; // The image to transition
			imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // Aspect of the image to transition
			imageMemoryBarrier.subresourceRange.baseMipLevel = 0; // Base mip level
			imageMemoryBarrier.subresourceRange.levelCount = this->mTexture1->mMipLevels; // Number of mip levels
			imageMemoryBarrier.subresourceRange.baseArrayLayer = 0; // Base array layer
			imageMemoryBarrier.subresourceRange.layerCount = 1; // Number of array layers

			// Specify the access masks and pipeline stages for the transition
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT; // Access mask for the compute shader write
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // Access mask for subsequent shader read
			imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // Queue family indices
			imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			// Insert pipeline barrier
			vkCmdPipelineBarrier(*VulkanRenderer::GetRenderer()->mActiveCommandBuffer,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, // Compute shader stage
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, // Subsequent shader stage
				0, // Dependency flags
				0, nullptr, // Memory barriers
				0, nullptr, // Buffer memory barriers
				1, &imageMemoryBarrier); // Image memory barriers
			//mBloomUpScaleShader->setVec2("u_texel_size", 1.0f / glm::vec2(mipSize));
			//mBloomUpScaleShader->setInt("u_mip_level", i);
			//
			//glBindImageTexture(0, mFinalTexturePair->texture1.id, i - 1, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

			//((**    this->mComputeShadersScaleDown.Dispatch(glm::ceil(float(mipSize.x) / 8), glm::ceil(float(mipSize.y) / 8), 1);

			//glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
		}

		//VulkanRenderer::GetRenderer()->TransitionImageLayout(VulkanRenderer::GetRenderer()->mFinalSceneImage, VulkanRenderer::GetRenderer()->mSwapChainImageFormat, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	}

	void VulkanBloom::CopySceneTexture() {
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

		imageCopyRegion.extent.width = Application->appSizes->sceneSize.x;
		imageCopyRegion.extent.height = Application->appSizes->sceneSize.y;
		imageCopyRegion.extent.depth = 1;

		//VulkanRenderer::GetRenderer()->TransitionImageLayout(VulkanRenderer::GetRenderer()->mDeferredFinalImage, VulkanRenderer::GetRenderer()->mFinalDeferredFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		VulkanRenderer::GetRenderer()->TransitionImageLayout(this->mTexture1->mImage, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		vkCmdCopyImage(
			*VulkanRenderer::GetRenderer()->mActiveCommandBuffer,
			VulkanRenderer::GetRenderer()->mDeferredFinalImage, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			this->mTexture1->mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &imageCopyRegion
		);

		//VulkanRenderer::GetRenderer()->TransitionImageLayout(VulkanRenderer::GetRenderer()->mDeferredFinalImage, VulkanRenderer::GetRenderer()->mFinalDeferredFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		VulkanRenderer::GetRenderer()->TransitionImageLayout(this->mTexture1->mImage, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
	}

	void VulkanBloom::BlendBloomWithScene() {

	}
	void VulkanBloom::Terminate() {

	}
}