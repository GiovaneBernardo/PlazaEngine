#include "Engine/Core/PreCompiledHeaders.h"
#include "VulkanShadows.h"
#include "VulkanPlazaInitializator.h"
#include <bitset>
#include "Renderer.h"

namespace Plaza {
	void VulkanShadows::CreateDescriptorPool(VkDevice device) {
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = 32;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = 32;

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = 32;

		if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &this->mDescriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	void VulkanShadows::CreateDescriptorSetLayout(VkDevice device) {
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

		std::array<VkDescriptorSetLayoutBinding, 2> bindings = {
			uboLayoutBinding, plvk::descriptorSetLayoutBinding(1, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr,
															   VK_SHADER_STAGE_VERTEX_BIT)};
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();
		// layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;

		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &this->mDescriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}

		// Debug Pass
		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = this->mCascades.size();
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		bindings = {samplerLayoutBinding};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();
		// layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &this->mDebugDepthDescriptorLayout) !=
			VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	void VulkanShadows::CreateDescriptorSet(VkDevice device) {
		VkDescriptorSetAllocateInfo allocInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
		allocInfo.descriptorPool = this->mDescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &mDescriptorSetLayout;

		mDescriptorSets.resize(Application::Get()->mRenderer->mMaxFramesInFlight);
		for (unsigned int i = 0; i < Application::Get()->mRenderer->mMaxFramesInFlight; ++i) {
			if (vkAllocateDescriptorSets(device, &allocInfo, &this->mDescriptorSets[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate descriptor sets!");
			}

			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = mUniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(ShadowsUniformBuffer);

			std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = this->mDescriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0,
								   nullptr);

			for (uint32_t j = 0; j < mCascades.size(); ++j) {
				this->mCascades[j].mDescriptorSets.resize(Application::Get()->mRenderer->mMaxFramesInFlight);
				vkAllocateDescriptorSets(device, &allocInfo, &this->mCascades[j].mDescriptorSets[i]);
				VkDescriptorImageInfo cascadeImageInfo{};
				cascadeImageInfo.sampler = this->mShadowsSampler;
				cascadeImageInfo.imageView = this->mShadowDepthImageViews[i];
				cascadeImageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

				std::vector<VkWriteDescriptorSet> writeDescriptorSets{};
				writeDescriptorSets.push_back(plvk::writeDescriptorSet(this->mCascades[j].mDescriptorSets[i], 0, 0,
																	   VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, nullptr,
																	   &bufferInfo));

				VkDescriptorBufferInfo bonesBufferInfo = plvk::descriptorBufferInfo(
					VulkanRenderer::GetRenderer()->mBoneMatricesBuffers[i], 0, 1024 * 16 * sizeof(glm::mat4));
				writeDescriptorSets.push_back(plvk::writeDescriptorSet(this->mCascades[j].mDescriptorSets[i], 1, 0,
																	   VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, nullptr,
																	   &bonesBufferInfo));

				vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()),
									   writeDescriptorSets.data(), 0, NULL);
			}
		}

		// Debug Pass
		allocInfo.pSetLayouts = &mDebugDepthDescriptorLayout;
		if (vkAllocateDescriptorSets(device, &allocInfo, &this->mDebugDepthDescriptorSet) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}
		// vkAllocateDescriptorSets(device, &allocInfo, &this->mDebugDepthDescriptorSet);

		std::vector<VkDescriptorImageInfo> imageInfos = std::vector<VkDescriptorImageInfo>();
		imageInfos.resize(9);
		for (size_t i = 0; i < imageInfos.size(); ++i) {
			imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			imageInfos[i].imageView = this->mCascades[i].mImageView;
			imageInfos[i].sampler = this->mShadowsSampler;
		}

		std::array<VkWriteDescriptorSet, 1> writeDescriptorSets{};
		writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[0].dstSet = this->mDebugDepthDescriptorSet;
		writeDescriptorSets[0].dstBinding = 1;
		writeDescriptorSets[0].dstArrayElement = 0;
		writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSets[0].pImageInfo = imageInfos.data();
		writeDescriptorSets[0].descriptorCount = imageInfos.size();
		vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0,
							   NULL);
	}

	void VulkanShadows::InitializeBuffers(VulkanRenderer& renderer) {
		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = this->mShadowResolution;
		imageInfo.extent.height = this->mShadowResolution;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 9;
		imageInfo.format = mDepthFormat;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		// imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateImage(renderer.mDevice, &imageInfo, nullptr, &this->mShadowDepthImage) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create depth image!");
		}

		// Allocate memory and bind image
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(renderer.mDevice, this->mShadowDepthImage, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex =
			renderer.FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkDeviceMemory shadowsDepthMapMemory;
		if (vkAllocateMemory(renderer.mDevice, &allocInfo, nullptr, &shadowsDepthMapMemory) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate depth image memory!");
		}

		vkBindImageMemory(renderer.mDevice, this->mShadowDepthImage, shadowsDepthMapMemory, 0);

		// Transition image layout
		renderer.TransitionImageLayout(this->mShadowDepthImage, mDepthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
									   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
									   VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 9);
		// renderer.TransitionImageLayout(this->mShadowDepthImage, mDepthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
		// VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_DEPTH_BIT, 9);

		mShadowDepthImageViews.resize(2);
		for (unsigned int i = 0; i < 2; ++i) {
			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = this->mShadowDepthImage;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			viewInfo.format = mDepthFormat;
			viewInfo.subresourceRange = {};
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = this->mCascadeCount;

			if (vkCreateImageView(VulkanRenderer::GetRenderer()->mDevice, &viewInfo, nullptr,
								  &this->mShadowDepthImageViews[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create texture image view!");
			}
		}
		this->mCascades.resize(9);
		for (unsigned int i = 0; i < this->mCascades.size(); ++i) {
			this->mCascades[i] = *new Cascade();
			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = this->mShadowDepthImage;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			viewInfo.format = mDepthFormat;
			viewInfo.subresourceRange = {};
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = i;
			viewInfo.subresourceRange.layerCount = 1;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			if (vkCreateImageView(renderer.mDevice, &viewInfo, nullptr, &this->mCascades[i].mImageView) != VK_SUCCESS) {
				throw std::runtime_error("failed to create texture image view!");
			}

			// VkFramebufferCreateInfo framebufferInfo{};
			// framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			// framebufferInfo.renderPass = this->mRenderPass;
			// framebufferInfo.attachmentCount = 1;
			// framebufferInfo.pAttachments = &this->mCascades[i].mImageView;
			// framebufferInfo.width = this->mShadowResolution;
			// framebufferInfo.height = this->mShadowResolution;
			// framebufferInfo.layers = this->mCascadeCount;
			// if (vkCreateFramebuffer(renderer.mDevice, &framebufferInfo, nullptr, &this->mCascades[i].mFramebuffer) !=
			// VK_SUCCESS) { 	throw std::runtime_error("Failed to create framebuffer!");
			// }
		}

		this->mFramebuffers.resize(Application::Get()->mRenderer->mMaxFramesInFlight);
		for (int i = 0; i < Application::Get()->mRenderer->mMaxFramesInFlight; ++i) {
			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = this->mRenderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = &this->mShadowDepthImageViews[i];
			framebufferInfo.width = this->mShadowResolution;
			framebufferInfo.height = this->mShadowResolution;
			framebufferInfo.layers = 1; // this->mCascadeCount;

			if (vkCreateFramebuffer(renderer.mDevice, &framebufferInfo, nullptr, &this->mFramebuffers[i]) !=
				VK_SUCCESS) {
				throw std::runtime_error("Failed to create framebuffer!");
			}
		}

		// Debug pass
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = Application::Get()->appSizes->sceneSize.x;
		imageInfo.extent.height = Application::Get()->appSizes->sceneSize.y;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		if (vkCreateImage(renderer.mDevice, &imageInfo, nullptr, &this->mDebugDepthImage) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create depth image!");
		}

		// Allocate memory and bind image
		VkMemoryRequirements memRequirements2;
		vkGetImageMemoryRequirements(renderer.mDevice, this->mDebugDepthImage, &memRequirements2);

		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements2.size;
		allocInfo.memoryTypeIndex =
			renderer.FindMemoryType(memRequirements2.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkDeviceMemory shadowsDepthMapMemory2;
		if (vkAllocateMemory(renderer.mDevice, &allocInfo, nullptr, &shadowsDepthMapMemory2) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate depth image memory!");
		}

		vkBindImageMemory(renderer.mDevice, this->mDebugDepthImage, shadowsDepthMapMemory2, 0);

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = this->mDebugDepthImage;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		if (vkCreateImageView(VulkanRenderer::GetRenderer()->mDevice, &viewInfo, nullptr,
							  &this->mDebugDepthImageView) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture image view!");
		}

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = this->mDepthDebugRenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &this->mDebugDepthImageView;
		framebufferInfo.width = Application::Get()->appSizes->sceneSize.x;
		framebufferInfo.height = Application::Get()->appSizes->sceneSize.y;
		framebufferInfo.layers = 1;
		// framebufferInfo.flags = VK_IMAGE_USAGE_SAMPLED_BIT;
		if (vkCreateFramebuffer(renderer.mDevice, &framebufferInfo, nullptr, &this->mDepthDebugFramebuffer) !=
			VK_SUCCESS) {
			throw std::runtime_error("Failed to create framebuffer!");
		}
	}

	void VulkanShadows::InitializeRenderPass(VulkanRenderer& renderer) {
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = samplerInfo.addressModeU;
		samplerInfo.addressModeW = samplerInfo.addressModeU;
		samplerInfo.mipLodBias = 0.0f;
		// samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 1.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 1.0f;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

		vkCreateSampler(renderer.mDevice, &samplerInfo, nullptr, &this->mShadowsSampler);

		VkFormat depthFormat = mDepthFormat;

		/*
				VkAttachmentDescription colorAttachment{};
		colorAttachment.format = VK_FORMAT_B8G8R8A8_UNORM;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = FindDepthFormat();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		*/

		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = VK_FORMAT_R8G8B8A8_SRGB;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = depthFormat;
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 0;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 0;
		subpass.pColorAttachments = nullptr;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
		std::vector<VkSubpassDescription> subpasses = std::vector<VkSubpassDescription>();
		for (int i = 0; i < 1; ++i) {
			subpasses.push_back(subpass);
		}

		std::vector<VkSubpassDependency> dependencies = std::vector<VkSubpassDependency>();
		dependencies.resize(2);
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		// Add dependencies for each pair of consecutive subpasses
		// for (int i = 0; i < 4 - 1; ++i) {
		//	VkSubpassDependency dependency{};
		//	dependency.srcSubpass = i;
		//	dependency.dstSubpass = i + 1;
		//	dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		//	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		//	dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		//	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		//	dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		//
		//	dependencies.push_back(dependency);
		//}

		std::array<VkAttachmentDescription, 1> attachments = {depthAttachment};
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = subpasses.size();
		renderPassInfo.pSubpasses = subpasses.data();
		renderPassInfo.dependencyCount = dependencies.size();
		renderPassInfo.pDependencies = dependencies.data();

		std::vector<uint32_t> viewMasks(this->mCascadeCount, 0);
		for (int i = 0; i < this->mCascadeCount; ++i) {
			viewMasks[i] = 0;
		}
		uint32_t viewMask = 0;
		viewMask |= 1u << this->mCascadeCount;
		viewMask -= 1;
		/*
			Bit mask that specifies correlation between views
			An implementation may use this for optimizations (concurrent render)
		*/
		const uint32_t correlationMask = 0;

		VkRenderPassMultiviewCreateInfo renderPassMultiviewCI{};
		renderPassMultiviewCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO;
		renderPassMultiviewCI.subpassCount = 1;
		renderPassMultiviewCI.pViewMasks = &viewMask;
		renderPassMultiviewCI.correlationMaskCount = 0;
		std::vector<int32_t> viewOffsets(dependencies.size(), 0);
		renderPassMultiviewCI.pViewOffsets = viewOffsets.data();
		// renderPassMultiviewCI.dependencyCount = dependencies.size();
		// renderPassMultiviewCI.dep = 2;

		renderPassInfo.pNext = &renderPassMultiviewCI;
		if (vkCreateRenderPass(renderer.mDevice, &renderPassInfo, nullptr, &this->mRenderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}

		// Debug pass
		std::array<VkSubpassDependency, 1> debugDependencies;
		debugDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		debugDependencies[0].dstSubpass = 0;
		debugDependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		debugDependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		debugDependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		debugDependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		debugDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = VK_NULL_HANDLE;
		attachments = {colorAttachment};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = debugDependencies.size();
		renderPassInfo.pDependencies = debugDependencies.data();

		renderPassInfo.pNext = nullptr;

		if (vkCreateRenderPass(renderer.mDevice, &renderPassInfo, nullptr, &this->mDepthDebugRenderPass) !=
			VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}

	void VulkanShadows::Init() {
		float mult = 1.0f;
		shadowCascadeLevels = vector{Application::Get()->activeCamera->farPlane / (9000.0f * mult),
									 Application::Get()->activeCamera->farPlane / (3000.0f * mult),
									 Application::Get()->activeCamera->farPlane / (1000.0f * mult),
									 Application::Get()->activeCamera->farPlane / (500.0f * mult),
									 Application::Get()->activeCamera->farPlane / (100.0f * mult),
									 Application::Get()->activeCamera->farPlane / (35.0f * mult),
									 Application::Get()->activeCamera->farPlane / (10.0f * mult),
									 Application::Get()->activeCamera->farPlane / (2.0f * mult),
									 Application::Get()->activeCamera->farPlane / (1.0f * mult)};

		int bufferSize = sizeof(ShadowsUniformBuffer);

		mUniformBuffers.resize(Application::Get()->mRenderer->mMaxFramesInFlight);
		mUniformBuffersMemory.resize(Application::Get()->mRenderer->mMaxFramesInFlight);
		mUniformBuffersMapped.resize(Application::Get()->mRenderer->mMaxFramesInFlight);
		for (unsigned int i = 0; i < Application::Get()->mRenderer->mMaxFramesInFlight; ++i) {
			VulkanRenderer::GetRenderer()->CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
														VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
															VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
														mUniformBuffers[i], mUniformBuffersMemory[i]);
			vkMapMemory(VulkanRenderer::GetRenderer()->mDevice, mUniformBuffersMemory[i], 0, bufferSize, 0,
						&mUniformBuffersMapped[i]);
		}

		// this->InitializeRenderPass(*VulkanRenderer::GetRenderer());
		// this->InitializeBuffers(*VulkanRenderer::GetRenderer());
		//
		//
		// this->CreateDescriptorPool(VulkanRenderer::GetRenderer()->mDevice);
		// this->CreateDescriptorSetLayout(VulkanRenderer::GetRenderer()->mDevice);
		// this->CreateDescriptorSet(VulkanRenderer::GetRenderer()->mDevice);
		//
		// mShadowsShader = new VulkanShaders(VulkanShadersCompiler::Compile(Application::Get()->enginePath +
		// "/Shaders/shadows/cascadedShadowDepthShaders.vert"),
		// VulkanShadersCompiler::Compile(Application::Get()->enginePath +
		// "/Shaders/shadows/cascadedShadowDepthShaders.frag"),
		// VulkanShadersCompiler::Compile(Application::Get()->enginePath +
		// "/Shaders/shadows/cascadedShadowDepthShaders.geom"));
		//
		// VkPushConstantRange pushConstantRange{};
		// pushConstantRange.stageFlags = VK_SHADER_STAGE_ALL;
		// pushConstantRange.offset = 0;
		// pushConstantRange.size = sizeof(VulkanShadows::PushConstants);
		//
		// VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		// pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		// pipelineLayoutInfo.setLayoutCount = 1;
		// pipelineLayoutInfo.pSetLayouts = &this->mDescriptorSetLayout;
		// pipelineLayoutInfo.pushConstantRangeCount = 1;
		// pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		// mShadowsShader->Init(VulkanRenderer::GetRenderer()->mDevice, this->mRenderPass, this->mShadowResolution,
		// this->mShadowResolution, this->mDescriptorSetLayout, pipelineLayoutInfo, std::vector<VkPushConstantRange> {
		// pushConstantRange });
		//
		//// Debug
		// mDepthDebugShaders = new VulkanShaders(VulkanShadersCompiler::Compile(Application::Get()->enginePath +
		// "/Shaders/shadows/cascadedShadowDepthDebugShaders.vert"),
		// VulkanShadersCompiler::Compile(Application::Get()->enginePath +
		// "/Shaders/shadows/cascadedShadowDepthDebugShaders.frag"),
		// VulkanShadersCompiler::Compile(Application::Get()->enginePath +
		// "/Shaders/shadows/cascadedShadowDepthShaders.geom")); pipelineLayoutInfo.pSetLayouts =
		// &this->mDebugDepthDescriptorLayout; mDepthDebugShaders->Init(VulkanRenderer::GetRenderer()->mDevice,
		// this->mDepthDebugRenderPass, Application::Get()->appSizes->sceneSize.x,
		// Application::Get()->appSizes->sceneSize.y, this->mDebugDepthDescriptorLayout, pipelineLayoutInfo,
		// std::vector<VkPushConstantRange> { pushConstantRange }, false);
	}

	void VulkanShadows::RenderToShadowMap() {
		UpdateUniformBuffer(0);
		UpdateUniformBuffer(1);
	}

	void VulkanShadows::Terminate() {}

	using std::min;
	glm::mat4 VulkanShadows::GetLightSpaceMatrix(const float nearPlane, const float farPlane, const float ratio,
												 const glm::mat4& viewMatrix, const glm::vec3& lightDirection) {
		const auto proj =
			glm::perspective(glm::radians(Application::Get()->activeCamera->Zoom), ratio, nearPlane, farPlane);
		const auto corners = Application::Get()->activeCamera->getFrustumCornersWorldSpace(proj, viewMatrix);

		glm::vec3 center = glm::vec3(0, 0, 0);
		for (const auto& v : corners) {
			center += glm::vec3(v);
		}
		center /= corners.size();

		const float LARGE_CONSTANT = std::abs(std::numeric_limits<float>::min());
		auto lightView = glm::lookAt(center + lightDirection, center, glm::vec3(0.0f, 1.0f, 0.0f));
		float minX = std::numeric_limits<float>::max();
		float maxX = std::numeric_limits<float>::lowest();
		float minY = std::numeric_limits<float>::max();
		float maxY = std::numeric_limits<float>::lowest();
		float minZ = std::numeric_limits<float>::max();
		float maxZ = std::numeric_limits<float>::lowest();
		for (const auto& v : corners) {
			const auto trf = lightView * v;
			minX = std::min(minX, trf.x);
			maxX = std::max(maxX, trf.x);
			minY = std::min(minY, trf.y);
			maxY = std::max(maxY, trf.y);
			minZ = std::min(minZ, trf.z);
			maxZ = std::max(maxZ, trf.z);
		}

		// Tune this parameter according to the scene
		// lightView = glm::lookAt(center - lightDir * (minZ), center, glm::vec3(0.0f, 1.0f, 0.0f));

		constexpr float zMult = 22.0f;
		if (minZ < 0) {
			minZ *= zMult;
		}
		else {
			minZ /= zMult;
		}
		if (maxZ < 0) {
			maxZ /= zMult;
		}
		else {
			maxZ *= zMult;
		}

		// const glm::mat4 lightProjection = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.01f,
		// maxExtents.z - minExtents.z);
		const glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
		return lightProjection * lightView;
	}
	std::vector<glm::mat4> VulkanShadows::GetLightSpaceMatrices(std::vector<float> shadowCascadeLevels,
																const glm::vec3& lightDirection) {
		PLAZA_PROFILE_SECTION("GetLightSpaceMatrices");
		std::vector<glm::mat4> ret = std::vector<glm::mat4>();
		ret.resize(shadowCascadeLevels.size() + 1);
		float ratio =
			(float)Application::Get()->appSizes->sceneSize.x / (float)Application::Get()->appSizes->sceneSize.y;
		const glm::mat4 viewMatrix = Application::Get()->activeCamera->GetViewMatrix();
		for (size_t i = 0; i < shadowCascadeLevels.size() + 1; ++i) {
			if (i == 0) {
				ret[i] = VulkanShadows::GetLightSpaceMatrix(Application::Get()->editorCamera->nearPlane - 1.0f,
															shadowCascadeLevels[i], ratio, viewMatrix, lightDirection);
			}
			else if (i < shadowCascadeLevels.size()) {
				ret[i] = VulkanShadows::GetLightSpaceMatrix(shadowCascadeLevels[i - 1] - 1.0f, shadowCascadeLevels[i],
															ratio, viewMatrix, lightDirection);
			}
		}
		return ret;
	}

	void VulkanShadows::UpdateUniformBuffer(unsigned int frameIndex) {
		PLAZA_PROFILE_SECTION("Update Shadows Uniform Buffers");
		// memcpy(&this->mUbo, GetLightSpaceMatrices(this->shadowCascadeLevels, this->mUbo).data(),
		// sizeof(this->mUbo));//this->mUbo.lightSpaceMatrices = GetLightSpaceMatrices(this->shadowCascadeLevels,
		// this->mUbo).data();
		this->mUbo.resize(Application::Get()->mRenderer->mMaxFramesInFlight);
		std::vector<glm::mat4> mats = VulkanShadows::GetLightSpaceMatrices(this->shadowCascadeLevels, mLightDirection);
		for (int i = 0; i < 9; ++i) {
			this->mUbo[frameIndex].lightSpaceMatrices[i] = mats[i];
		}
		{
			PLAZA_PROFILE_SECTION("memcpy");
			// memcpy(mUniformBuffersMapped[frameIndex], &this->mUbo[frameIndex], sizeof(this->mUbo[frameIndex]));
		}
	}

	void VulkanShadows::UpdateAndPushConstants(VkCommandBuffer commandBuffer, unsigned int cascadeIndex) {
		this->pushConstants.cascadeIndex = cascadeIndex;
		vkCmdPushConstants(commandBuffer, this->mShadowsShader->mPipelineLayout, VK_SHADER_STAGE_ALL, 0,
						   sizeof(VulkanShadows::PushConstants), &this->pushConstants);
	}
} // namespace Plaza
