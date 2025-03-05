#include <Engine/Core/PreCompiledHeaders.h>
#include "VulkanSkybox.h"
#include "VulkanPlazaPipeline.h"
#include "Editor/DefaultAssets/Models/DefaultModels.h"
#include "VulkanPlazaInitializator.h"
#include "Renderer.h"

namespace Plaza {
	void VulkanSkybox::InitializeImageSampler() {
		// VkSamplerCreateInfo samplerInfo{};
		// samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		// samplerInfo.magFilter = VK_FILTER_LINEAR;
		// samplerInfo.minFilter = VK_FILTER_LINEAR;
		// samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		// samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		// samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		// VkPhysicalDeviceProperties properties{};
		// vkGetPhysicalDeviceProperties(VulkanRenderer::GetRenderer()->mPhysicalDevice, &properties);
		// samplerInfo.unnormalizedCoordinates = VK_FALSE;
		// samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		// samplerInfo.mipLodBias = 0.0f;
		// samplerInfo.minLod = 0.0f;
		// samplerInfo.maxLod = 0.0f;
		// samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
		// if (vkCreateSampler(VulkanRenderer::GetRenderer()->mDevice, &samplerInfo, nullptr, &mSkyboxTexture->mSampler)
		// != VK_SUCCESS) { 	throw std::runtime_error("failed to create texture sampler!");
		// }
	}

	void VulkanSkybox::InitializeIrradiance() {
		VkDescriptorSetLayout irradianceGeneratorDescriptorSetLayout{};
		VkDescriptorSetLayoutBinding environmentLayoutBinding = plvk::descriptorSetLayoutBinding(
			2, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nullptr, VK_SHADER_STAGE_FRAGMENT_BIT);

		std::array<VkDescriptorSetLayoutBinding, 1> bindings = {environmentLayoutBinding};
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();
		layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
		if (vkCreateDescriptorSetLayout(VulkanRenderer::GetRenderer()->mDevice, &layoutInfo, nullptr,
										&irradianceGeneratorDescriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}

		VkDescriptorSetAllocateInfo converterAllocInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
		converterAllocInfo.descriptorPool = VulkanRenderer::GetRenderer()->mDescriptorPool;
		converterAllocInfo.descriptorSetCount = 1;
		converterAllocInfo.pSetLayouts = &irradianceGeneratorDescriptorSetLayout;

		VkDescriptorSet irradianceGeneratorDescriptorSet{};
		if (vkAllocateDescriptorSets(VulkanRenderer::GetRenderer()->mDevice, &converterAllocInfo,
									 &irradianceGeneratorDescriptorSet) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		/* Initialize Irradiance texture */
		mIrradianceTexture = new VulkanTexture();
		mIrradianceTexture->CreateTextureImage(
			VulkanRenderer::GetRenderer()->mDevice, mSkyboxFormat, mIrradianceSize, mIrradianceSize, false,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
				VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED, 6,
			VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT, true, VK_SHARING_MODE_EXCLUSIVE, true);
		// VulkanRenderer::GetRenderer()->TransitionImageLayout(mIrradianceTexture->mImage, this->mSkyboxFormat,
		// VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 6,
		// mIrradianceTexture->mMipLevels);
		// VulkanRenderer::GetRenderer()->TransitionImageLayout(mIrradianceTexture->mImage, this->mSkyboxFormat,
		// VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 6,
		// mIrradianceTexture->mMipLevels);
		mIrradianceTexture->CreateImageView(mSkyboxFormat, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_CUBE, 6);

		VkPushConstantRange pushConstantRange = {};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_ALL;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(ConverterPushConstants);

		VkPipelineLayoutCreateInfo irradianceGeneratorPipelineLayoutInfo{};
		irradianceGeneratorPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		irradianceGeneratorPipelineLayoutInfo.setLayoutCount = 1;
		irradianceGeneratorPipelineLayoutInfo.pSetLayouts = &irradianceGeneratorDescriptorSetLayout;
		irradianceGeneratorPipelineLayoutInfo.pushConstantRangeCount = 1;
		irradianceGeneratorPipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		VulkanPlazaPipeline irradianceGeneratorPipeline = VulkanPlazaPipeline();

		VkSamplerCreateInfo irradianceSamplerInfo{};
		irradianceSamplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		irradianceSamplerInfo.magFilter = VK_FILTER_LINEAR;
		irradianceSamplerInfo.minFilter = VK_FILTER_LINEAR;
		irradianceSamplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		irradianceSamplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		irradianceSamplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(VulkanRenderer::GetRenderer()->mPhysicalDevice, &properties);
		// irradianceSamplerInfo.unnormalizedCoordinates = VK_FALSE;
		irradianceSamplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		irradianceSamplerInfo.mipLodBias = 0.0f;
		irradianceSamplerInfo.minLod = 0.0f;
		irradianceSamplerInfo.maxLod = mIrradianceTexture->mMipCount;
		// irradianceSamplerInfo.compareOp = VK_COMPARE_OP_NEVER;
		if (vkCreateSampler(VulkanRenderer::GetRenderer()->mDevice, &irradianceSamplerInfo, nullptr,
							&mIrradianceTexture->mSampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}

		// Render pass
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependency.srcStageMask =
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstStageMask =
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 1> attachments = {colorAttachment};
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;
		if (vkCreateRenderPass(VulkanRenderer::GetRenderer()->mDevice, &renderPassInfo, nullptr,
							   &irradianceGeneratorPipeline.mShaders->mRenderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_FALSE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_NOT_EQUAL;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
			plvk::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
		VkPipelineRasterizationStateCreateInfo rasterizationState =
			plvk::pipelineRasterizationStateCreateInfo(VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL, 1.0f, VK_FALSE, 0.0f,
													   0.0f, 0.0f, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
		VkPipelineColorBlendAttachmentState blendAttachmentState = plvk::pipelineColorBlendAttachmentState(VK_TRUE);
		std::vector<VkPipelineColorBlendAttachmentState> blendAttachments{blendAttachmentState};
		VkPipelineColorBlendStateCreateInfo colorBlendState =
			plvk::pipelineColorBlendStateCreateInfo(1, blendAttachments.data());
		VkPipelineDepthStencilStateCreateInfo depthStencilState =
			plvk::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);
		VkPipelineViewportStateCreateInfo viewportState = plvk::pipelineViewportStateCreateInfo(1, 1);
		VkPipelineMultisampleStateCreateInfo multisampleState =
			plvk::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
		std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
		VkPipelineDynamicStateCreateInfo dynamicState = plvk::pipelineDynamicStateCreateInfo(dynamicStateEnables);
		irradianceGeneratorPipeline.mShaders->mVertexShaderPath = VulkanShadersCompiler::Compile(
			Application::Get()->enginePath + "/Shaders/Vulkan/skybox/equirectangularToCubemap.vert");
		irradianceGeneratorPipeline.mShaders->mFragmentShaderPath = VulkanShadersCompiler::Compile(
			Application::Get()->enginePath + "/Shaders/Vulkan/skybox/irradianceGenerator.frag");
		irradianceGeneratorPipeline.mShaders->InitializeFull(
			VulkanRenderer::GetRenderer()->mDevice, irradianceGeneratorPipelineLayoutInfo, true, 32, 32, {},
			vertexInputInfo, inputAssemblyState, viewportState, rasterizationState, multisampleState, colorBlendState,
			dynamicState, irradianceGeneratorPipeline.mShaders->mRenderPass, depthStencilState);

		VkDescriptorImageInfo skyboxSamplerInfo = plvk::descriptorImageInfo(
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mSkyboxTexture->mImageView, mIrradianceTexture->mSampler);
		std::vector<VkWriteDescriptorSet> descriptorWrites = std::vector<VkWriteDescriptorSet>();
		descriptorWrites.push_back(plvk::writeDescriptorSet(
			irradianceGeneratorDescriptorSet, 2, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &skyboxSamplerInfo));

		vkUpdateDescriptorSets(VulkanRenderer::GetRenderer()->mDevice, static_cast<uint32_t>(descriptorWrites.size()),
							   descriptorWrites.data(), 0, nullptr);

		// VulkanRenderer::GetRenderer()->AddTrackerToImage(mIrradianceTexture->mImageView, "Irradiance Map",
		// VulkanRenderer::GetRenderer()->mTextureSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		VkCommandBuffer commandBuffer = VulkanRenderer::GetRenderer()->BeginSingleTimeCommands();
		ConverterPushConstants converterPushConstants{};
		converterPushConstants.first = false;
		converterPushConstants.deltaPhi = (2.0f * float(3.14159265358979323846)) / 180.0f;
		converterPushConstants.deltaTheta = (0.5f * float(3.14159265358979323846)) / 64.0f;

		std::vector<glm::mat4> mats = {
			// POSITIVE_X
			glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
						glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// NEGATIVE_X
			glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
						glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// POSITIVE_Y
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// NEGATIVE_Y
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// POSITIVE_Z
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// NEGATIVE_Z
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
		};
		for (uint32_t i = 0; i < 6; i++) {
			VkImageViewCreateInfo layerImageViewCreateInfo = {};
			layerImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			layerImageViewCreateInfo.image = mIrradianceTexture->mImage;
			layerImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			layerImageViewCreateInfo.format = mSkyboxFormat;
			layerImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			layerImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
			layerImageViewCreateInfo.subresourceRange.levelCount = 1;
			layerImageViewCreateInfo.subresourceRange.baseArrayLayer = i; // Target specific layer
			layerImageViewCreateInfo.subresourceRange.layerCount = 1;

			VkImageView layerImageView;
			vkCreateImageView(VulkanRenderer::GetRenderer()->mDevice, &layerImageViewCreateInfo, nullptr,
							  &layerImageView);

			std::vector<VkImageView> frameBufferAttachments{layerImageView};
			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = irradianceGeneratorPipeline.mShaders->mRenderPass;
			framebufferInfo.attachmentCount = frameBufferAttachments.size();
			framebufferInfo.pAttachments = frameBufferAttachments.data();
			framebufferInfo.width = mIrradianceSize;
			framebufferInfo.height = mIrradianceSize;
			framebufferInfo.layers = 1;

			VkFramebuffer framebuffer;
			if (vkCreateFramebuffer(VulkanRenderer::GetRenderer()->mDevice, &framebufferInfo, nullptr, &framebuffer) !=
				VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}

			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = irradianceGeneratorPipeline.mShaders->mRenderPass;
			renderPassInfo.framebuffer = framebuffer;
			renderPassInfo.renderArea.offset = {0, 0};
			renderPassInfo.renderArea.extent = {mIrradianceSize, mIrradianceSize};

			VkClearValue clearColor = {0.3f, 0.3f, 0.3f, 1.0f};
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
							  irradianceGeneratorPipeline.mShaders->mPipeline);

			VkRect2D scissor = {};
			scissor.offset = {0, 0};
			scissor.extent = {mIrradianceSize, mIrradianceSize};

			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = mIrradianceSize;
			viewport.height = mIrradianceSize;
			viewport.y = 0;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
									irradianceGeneratorPipeline.mShaders->mPipelineLayout, 0, 1,
									&irradianceGeneratorDescriptorSet, 0, nullptr);
			converterPushConstants.mvp =
				glm::perspective((float)(glm::pi<double>() / 2.0), 1.0f, 0.1f, 512.0f) * mats[i];
			vkCmdPushConstants(commandBuffer, irradianceGeneratorPipeline.mShaders->mPipelineLayout,
							   VK_SHADER_STAGE_ALL, 0, sizeof(ConverterPushConstants), &converterPushConstants);
			vkCmdDraw(commandBuffer, 36, 1, 0, 0);

			vkCmdEndRenderPass(commandBuffer);
		}

		VulkanRenderer::GetRenderer()->EndSingleTimeCommands(commandBuffer);

		VulkanRenderer::GetRenderer()->TransitionImageLayout(
			mIrradianceTexture->mImage, this->mSkyboxFormat, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 6, mIrradianceTexture->mMipCount);
		mIrradianceTexture->GenerateMipmaps(mIrradianceTexture->mImage, mIrradianceTexture->mWidth,
											mIrradianceTexture->mHeight, mIrradianceTexture->mMipCount,
											mIrradianceTexture->GetFormat(), 6);
	}

	void VulkanSkybox::InitializeImageView() {
		const uint32_t faceSize = 512;
		mSkyboxTexture = new VulkanTexture();
		mSkyboxTexture->CreateTextureImage(VulkanRenderer::GetRenderer()->mDevice, mSkyboxFormat, faceSize, faceSize,
										   false,
										   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
											   VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
										   VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED, 6,
										   VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT, true, VK_SHARING_MODE_EXCLUSIVE, true);
		mSkyboxTexture->CreateImageView(mSkyboxFormat, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_CUBE, 6);
		mSkyboxTexture->CreateTextureSampler(VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_MIPMAP_MODE_LINEAR,
											 VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);

		int texWidth, texHeight, texChannels;
		float* pixels = stbi_loadf(mSkyboxPaths[0].c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		uint32_t imageSize = texWidth * texHeight * 4 * 4;

		VulkanTexture mTemporaryEquirectangularTexture = VulkanTexture();
		// mTemporaryEquirectangularTexture.CreateTextureImage(VulkanRenderer::GetRenderer()->mDevice, mSkyboxPaths[0],
		// mSkyboxFormat, false); mTemporaryEquirectangularTexture.CreateImageView(mSkyboxFormat,
		// VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, 1);

		VkBuffer temporaryStagingBuffer;
		VkDeviceMemory temporaryStagingBufferMemory;
		{
			/* Temporary loaded equirectangular skybox*/
			VkImageCreateInfo imageInfo = {};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = faceSize;
			imageInfo.extent.height = faceSize;
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = 1;
			imageInfo.format = mSkyboxFormat;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
							  VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			imageInfo.extent.width = texWidth;
			imageInfo.extent.height = texHeight;
			imageInfo.arrayLayers = 1;
			imageInfo.flags = 0;
			imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
							  VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			if (vkCreateImage(VulkanRenderer::GetRenderer()->mDevice, &imageInfo, nullptr,
							  &mTemporaryEquirectangularTexture.mImage) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create depth image!");
			}
			VkMemoryRequirements memRequirements;
			vkGetImageMemoryRequirements(VulkanRenderer::GetRenderer()->mDevice,
										 mTemporaryEquirectangularTexture.mImage, &memRequirements);
			VkMemoryAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = VulkanRenderer::GetRenderer()->FindMemoryType(
				memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			VkDeviceMemory temporaryMemory;
			if (vkAllocateMemory(VulkanRenderer::GetRenderer()->mDevice, &allocInfo, nullptr, &temporaryMemory) !=
				VK_SUCCESS) {
				throw std::runtime_error("Failed to allocate depth image memory!");
			}
			vkBindImageMemory(VulkanRenderer::GetRenderer()->mDevice, mTemporaryEquirectangularTexture.mImage,
							  temporaryMemory, 0);

			VulkanRenderer::GetRenderer()->CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
														VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
															VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
														temporaryStagingBuffer, temporaryStagingBufferMemory);

			void* data;
			vkMapMemory(VulkanRenderer::GetRenderer()->mDevice, temporaryStagingBufferMemory, 0, imageSize, 0, &data);
			memcpy(data, pixels, static_cast<size_t>(imageSize));
			vkUnmapMemory(VulkanRenderer::GetRenderer()->mDevice, temporaryStagingBufferMemory);

			VulkanRenderer::GetRenderer()->TransitionImageLayout(
				mTemporaryEquirectangularTexture.mImage, this->mSkyboxFormat, VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 1);
			VulkanRenderer::GetRenderer()->CopyBufferToImage(
				temporaryStagingBuffer, mTemporaryEquirectangularTexture.mImage, texWidth, texHeight, 0, 1);
			VulkanRenderer::GetRenderer()->TransitionImageLayout(
				mTemporaryEquirectangularTexture.mImage, this->mSkyboxFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 1);

			vkDestroyBuffer(VulkanRenderer::GetRenderer()->mDevice, temporaryStagingBuffer, nullptr);
			vkFreeMemory(VulkanRenderer::GetRenderer()->mDevice, temporaryStagingBufferMemory, nullptr);

			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = mTemporaryEquirectangularTexture.mImage;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = this->mSkyboxFormat;
			viewInfo.subresourceRange = {};
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(VulkanRenderer::GetRenderer()->mDevice, &viewInfo, nullptr,
								  &mTemporaryEquirectangularTexture.mImageView) != VK_SUCCESS) {
				throw std::runtime_error("failed to create texture image view!");
			}
		}

		stbi_image_free(pixels);
		// Transition image layout
		// renderer.TransitionImageLayout(this->mShadowDepthImage, mDepthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
		// VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT, 9);
		// renderer.TransitionImageLayout(this->mShadowDepthImage, mDepthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
		// VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_DEPTH_BIT, 9);

		// this->mSkyboxImageViews.resize(Application::Get()->mRenderer->mMaxFramesInFlight);
		// for (unsigned int i = 0; i < Application::Get()->mRenderer->mMaxFramesInFlight; ++i) {
		//	VkImageViewCreateInfo viewInfo{};
		//	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		//	viewInfo.image = this->mSkyboxImage;
		//	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		//	viewInfo.format = this->mSkyboxFormat;
		//	viewInfo.subresourceRange = {};
		//	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		//	viewInfo.subresourceRange.baseMipLevel = 0;
		//	viewInfo.subresourceRange.levelCount = mipCount;
		//	viewInfo.subresourceRange.baseArrayLayer = 0;
		//	viewInfo.subresourceRange.layerCount = 6;
		//
		//	if (vkCreateImageView(VulkanRenderer::GetRenderer()->mDevice, &viewInfo, nullptr,
		//&this->mSkyboxImageViews[i]) != VK_SUCCESS) { 		throw std::runtime_error("failed to create texture image
		//view!");
		//	}
		// }

		/* Convert from equirectangular to cubemap */
		VkDescriptorSetLayout converterDescriptorSetLayout{};
		VkDescriptorSetLayoutBinding skyboxLayoutBinding = plvk::descriptorSetLayoutBinding(
			1, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nullptr, VK_SHADER_STAGE_FRAGMENT_BIT);

		std::array<VkDescriptorSetLayoutBinding, 1> bindings = {skyboxLayoutBinding};
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();
		layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;

		if (vkCreateDescriptorSetLayout(VulkanRenderer::GetRenderer()->mDevice, &layoutInfo, nullptr,
										&converterDescriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}

		// Descriptor set
		VkDescriptorSetAllocateInfo converterAllocInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
		converterAllocInfo.descriptorPool = VulkanRenderer::GetRenderer()->mDescriptorPool;
		converterAllocInfo.descriptorSetCount = 1;
		converterAllocInfo.pSetLayouts = &converterDescriptorSetLayout;

		VkDescriptorSet converterDescriptorSet{};
		if (vkAllocateDescriptorSets(VulkanRenderer::GetRenderer()->mDevice, &converterAllocInfo,
									 &converterDescriptorSet) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		// VkDescriptorBufferInfo uboInfo = plvk::descriptorBufferInfo(uboBuffer, 0, sizeof(ConverterUbo));

		// vkUpdateDescriptorSets(VulkanRenderer::GetRenderer()->mDevice, 1,
		// &plvk::writeDescriptorSet(converterDescriptorSet, 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, nullptr,
		// &uboInfo), 0, nullptr);

		std::vector<VkWriteDescriptorSet> descriptorWrites = std::vector<VkWriteDescriptorSet>();
		VkDescriptorImageInfo samplerInfo = plvk::descriptorImageInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
																	  mTemporaryEquirectangularTexture.mImageView,
																	  VulkanRenderer::GetRenderer()->mTextureSampler);
		descriptorWrites.push_back(plvk::writeDescriptorSet(
			converterDescriptorSet, 1, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &samplerInfo));

		// VkDescriptorImageInfo skyboxSamplerInfo = plvk::descriptorImageInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		// view2, VulkanRenderer::GetRenderer()->mTextureSampler);
		// descriptorWrites.push_back(plvk::writeDescriptorSet(converterDescriptorSet, 2, 0,
		// VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &skyboxSamplerInfo));

		vkUpdateDescriptorSets(VulkanRenderer::GetRenderer()->mDevice, static_cast<uint32_t>(descriptorWrites.size()),
							   descriptorWrites.data(), 0, nullptr);

		VkPushConstantRange pushConstantRange = {};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_ALL;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(ConverterPushConstants);
		VkPipelineLayoutCreateInfo converterPipelineLayoutInfo{};
		converterPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		converterPipelineLayoutInfo.setLayoutCount = 1;
		converterPipelineLayoutInfo.pSetLayouts = &converterDescriptorSetLayout;
		converterPipelineLayoutInfo.pushConstantRangeCount = 1;
		converterPipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		VulkanPlazaPipeline converterPipeline = VulkanPlazaPipeline();

		// Render pass
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependency.srcStageMask =
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstStageMask =
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 1> attachments = {colorAttachment};
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(VulkanRenderer::GetRenderer()->mDevice, &renderPassInfo, nullptr,
							   &converterPipeline.mShaders->mRenderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_FALSE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_NOT_EQUAL;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;
		converterPipeline.mShaders->mVertexShaderPath = VulkanShadersCompiler::Compile(
			Application::Get()->enginePath + "/Shaders/Vulkan/skybox/equirectangularToCubemap.vert");
		converterPipeline.mShaders->mFragmentShaderPath = VulkanShadersCompiler::Compile(
			Application::Get()->enginePath + "/Shaders/Vulkan/skybox/equirectangularToCubemap.frag");
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
			plvk::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
		VkPipelineRasterizationStateCreateInfo rasterizationState =
			plvk::pipelineRasterizationStateCreateInfo(VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL, 1.0f, VK_FALSE, 0.0f,
													   0.0f, 0.0f, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
		VkPipelineColorBlendAttachmentState blendAttachmentState = plvk::pipelineColorBlendAttachmentState(VK_TRUE);
		std::vector<VkPipelineColorBlendAttachmentState> blendAttachments{blendAttachmentState};
		VkPipelineColorBlendStateCreateInfo colorBlendState =
			plvk::pipelineColorBlendStateCreateInfo(1, blendAttachments.data());
		VkPipelineDepthStencilStateCreateInfo depthStencilState =
			plvk::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);
		VkPipelineViewportStateCreateInfo viewportState = plvk::pipelineViewportStateCreateInfo(1, 1);
		VkPipelineMultisampleStateCreateInfo multisampleState =
			plvk::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
		std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
		VkPipelineDynamicStateCreateInfo dynamicState = plvk::pipelineDynamicStateCreateInfo(dynamicStateEnables);
		converterPipeline.mShaders->InitializeFull(
			VulkanRenderer::GetRenderer()->mDevice, converterPipelineLayoutInfo, true, faceSize, faceSize, {},
			vertexInputInfo, inputAssemblyState, viewportState, rasterizationState, multisampleState, colorBlendState,
			dynamicState, converterPipeline.mShaders->mRenderPass, depthStencilState);

		std::vector<VkFramebuffer> frameBuffers = std::vector<VkFramebuffer>();
		ConverterPushConstants converterPushConstants{};
		converterPushConstants.first = true;

		// VulkanRenderer::GetRenderer()->AddTrackerToImage(mSkyboxTexture->mImageView, "Skybox",
		// mSkyboxTexture->mSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		// std::vector<glm::mat4> matrices = {
		//	// POSITIVE_X
		//	glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
		//glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		//	// NEGATIVE_X
		//	glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
		//glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		//	// POSITIVE_Y
		//	glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		//	// NEGATIVE_Y
		//	glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		//	// POSITIVE_Z
		//	glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		//	// NEGATIVE_Z
		//	glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
		// };

		VkCommandBuffer commandBuffer = VulkanRenderer::GetRenderer()->BeginSingleTimeCommands();
		glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		proj[1][1] *= -1;
		for (uint32_t i = 0; i < 6; i++) {
			VkImageViewCreateInfo layerImageViewCreateInfo = {};
			layerImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			layerImageViewCreateInfo.image = mSkyboxTexture->mImage;
			layerImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			layerImageViewCreateInfo.format = mSkyboxFormat;
			layerImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			layerImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
			layerImageViewCreateInfo.subresourceRange.levelCount = 1;
			layerImageViewCreateInfo.subresourceRange.baseArrayLayer = i; // Target specific layer
			layerImageViewCreateInfo.subresourceRange.layerCount = 1;

			VkImageView layerImageView;
			vkCreateImageView(VulkanRenderer::GetRenderer()->mDevice, &layerImageViewCreateInfo, nullptr,
							  &layerImageView);

			std::vector<VkImageView> frameBufferAttachments{layerImageView};
			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = converterPipeline.mShaders->mRenderPass;
			framebufferInfo.attachmentCount = frameBufferAttachments.size();
			framebufferInfo.pAttachments = frameBufferAttachments.data();
			framebufferInfo.width = faceSize;  // Application::Get()->appSizes->sceneSize.x;
			framebufferInfo.height = faceSize; // Application::Get()->appSizes->sceneSize.y;
			framebufferInfo.layers = 1;

			VkFramebuffer framebuffer; //= frameBuffers[i];
			if (vkCreateFramebuffer(VulkanRenderer::GetRenderer()->mDevice, &framebufferInfo, nullptr, &framebuffer) !=
				VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}

			// Begin render pass
			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = converterPipeline.mShaders->mRenderPass;
			renderPassInfo.framebuffer = framebuffer;
			renderPassInfo.renderArea.offset = {0, 0};
			renderPassInfo.renderArea.extent = {faceSize, faceSize};

			VkClearValue clearColor = {0.3f, 0.3f, 0.3f, 1.0f};
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			// Bind pipeline, descriptor sets, etc.
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, converterPipeline.mShaders->mPipeline);

			VkRect2D scissor = {};
			scissor.offset = {0, 0};			   // Set appropriate offset
			scissor.extent = {faceSize, faceSize}; // Set appropriate extent

			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = faceSize;
			viewport.height = -(float)(faceSize);
			viewport.y = faceSize;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

			// vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, );
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
									converterPipeline.mShaders->mPipelineLayout, 0, 1, &converterDescriptorSet, 0,
									nullptr);
			converterPushConstants.mvp =
				glm::perspective((float)(glm::pi<double>() / 2.0), 1.0f, 0.1f, 512.0f) * matrices[i];
			vkCmdPushConstants(commandBuffer, converterPipeline.mShaders->mPipelineLayout, VK_SHADER_STAGE_ALL, 0,
							   sizeof(ConverterPushConstants), &converterPushConstants);
			// Draw call to render to the cubemap face
			vkCmdDraw(commandBuffer, 36, 1, 0, 0);

			vkCmdEndRenderPass(commandBuffer);

			// Transition cubemap face to transfer destination layout
			// (Omitted for brevity)
		}

		VulkanRenderer::GetRenderer()->EndSingleTimeCommands(commandBuffer);

		VulkanRenderer::GetRenderer()->TransitionImageLayout(
			mSkyboxTexture->mImage, this->mSkyboxFormat, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT,
			mSkyboxTexture->GetTextureInfo().mLayersCount, mSkyboxTexture->mMipCount);
		mSkyboxTexture->GenerateMipmaps(mSkyboxTexture->mImage, mSkyboxTexture->mWidth, mSkyboxTexture->mHeight,
										mSkyboxTexture->mMipCount, mSkyboxTexture->GetFormat(),
										mSkyboxTexture->GetTextureInfo().mLayersCount);

		// VulkanRenderer::GetRenderer()->CopyBufferToImage(mStagingBuffer, this->mSkyboxImage, faceSize, faceSize, 0,
		// 6);

		// VulkanRenderer::GetRenderer()->TransitionImageLayout(this->mSkyboxImage, this->mSkyboxFormat,
		// VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 6,
		// mipCount); VkCommandBuffer commandBuffer2 = VulkanRenderer::GetRenderer()->BeginSingleTimeCommands();
		//
		////GenerateSkyboxMipmaps(commandBuffer2, this->mSkyboxImage, faceSize, faceSize, mipCount);
		//
		// VulkanRenderer::GetRenderer()->EndSingleTimeCommands(commandBuffer2);
		// VulkanRenderer::GetRenderer()->TransitionImageLayout(this->mSkyboxImage, this->mSkyboxFormat,
		// VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 6,
		// mipCount);

		InitializeIrradiance();
		GeneratePreFilteredEnvironment();

		// vkDestroyBuffer(VulkanRenderer::GetRenderer()->mDevice, temporaryStagingBuffer, nullptr);
		// vkFreeMemory(VulkanRenderer::GetRenderer()->mDevice, temporaryStagingBufferMemory, nullptr);
		// vkDestroyImage(VulkanRenderer::GetRenderer()->mDevice, mTemporaryEquirectangularTexture.mImage, nullptr);
		// vkDestroyImageView(VulkanRenderer::GetRenderer()->mDevice, temporaryImageView, nullptr);
		// vkDestroySampler(VulkanRenderer::GetRenderer()->mDevice, irrandianceGeneratorSampler, nullptr);

		mTemporaryEquirectangularTexture.Destroy();

		// for (unsigned int i = 0; i < 6; ++i) {

		//}
	}

	void VulkanSkybox::GenerateSkyboxMipmaps(VkCommandBuffer commandBuffer, VkImage image, int32_t width,
											 int32_t height, uint32_t mipLevels) {
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = width;
		int32_t mipHeight = height;

		for (uint32_t i = 1; i < mipLevels; i++) {
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
								 nullptr, 0, nullptr, 1, &barrier);

			VkImageBlit blit{};
			blit.srcOffsets[0] = {0, 0, 0};
			blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = {0, 0, 0};
			blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image,
						   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
								 0, 0, nullptr, 0, nullptr, 1, &barrier);

			if (mipWidth > 1)
				mipWidth /= 2;
			if (mipHeight > 1)
				mipHeight /= 2;
		}
	}

	void VulkanSkybox::GeneratePreFilteredEnvironment() {
		glm::vec2 size = glm::vec2(512, 512);

		mPreFilteredTexture = new VulkanTexture();
		mPreFilteredTexture->CreateTextureImage(
			VulkanRenderer::GetRenderer()->mDevice, mSkyboxFormat, size.x, size.y, false,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
				VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED, 6,
			VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT, false, VK_SHARING_MODE_EXCLUSIVE, true);
		mPreFilteredTexture->CreateImageView(mSkyboxFormat, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_CUBE, 6);
		mPreFilteredTexture->CreateTextureSampler(VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_MIPMAP_MODE_LINEAR,
												  VK_FILTER_LINEAR, VK_FILTER_LINEAR,
												  VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);
		const uint32_t numMips = mPreFilteredTexture->mMipCount;

		VkDescriptorSetLayout descriptorsetLayout{};
		VkDescriptorSetLayoutBinding skyboxLayoutBinding = plvk::descriptorSetLayoutBinding(
			0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nullptr, VK_SHADER_STAGE_FRAGMENT_BIT);

		std::array<VkDescriptorSetLayoutBinding, 1> bindings = {skyboxLayoutBinding};
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();
		layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;

		if (vkCreateDescriptorSetLayout(VulkanRenderer::GetRenderer()->mDevice, &layoutInfo, nullptr,
										&descriptorsetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}

		// Descriptor set
		VkDescriptorSetAllocateInfo converterAllocInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
		converterAllocInfo.descriptorPool = VulkanRenderer::GetRenderer()->mDescriptorPool;
		converterAllocInfo.descriptorSetCount = 1;
		converterAllocInfo.pSetLayouts = &descriptorsetLayout;

		VkDescriptorSet descriptorset{};
		if (vkAllocateDescriptorSets(VulkanRenderer::GetRenderer()->mDevice, &converterAllocInfo, &descriptorset) !=
			VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		std::vector<VkWriteDescriptorSet> descriptorWrites = std::vector<VkWriteDescriptorSet>();
		VkDescriptorImageInfo samplerInfo =
			plvk::descriptorImageInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mSkyboxTexture->mImageView,
									  VulkanRenderer::GetRenderer()->mTextureSampler);
		descriptorWrites.push_back(
			plvk::writeDescriptorSet(descriptorset, 0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &samplerInfo));

		vkUpdateDescriptorSets(VulkanRenderer::GetRenderer()->mDevice, static_cast<uint32_t>(descriptorWrites.size()),
							   descriptorWrites.data(), 0, nullptr);

		VkPushConstantRange pushConstantRange = {};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_ALL;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(ConverterPushConstants);
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorsetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		VulkanPlazaPipeline pipeline = VulkanPlazaPipeline();

		// Render pass
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependency.srcStageMask =
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstStageMask =
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 1> attachments = {colorAttachment};
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(VulkanRenderer::GetRenderer()->mDevice, &renderPassInfo, nullptr,
							   &pipeline.mShaders->mRenderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_FALSE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_NOT_EQUAL;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;
		pipeline.mShaders->mVertexShaderPath = VulkanShadersCompiler::Compile(
			Application::Get()->enginePath + "/Shaders/Vulkan/skybox/equirectangularToCubemap.vert");
		pipeline.mShaders->mFragmentShaderPath = VulkanShadersCompiler::Compile(
			Application::Get()->enginePath + "/Shaders/Vulkan/skybox/prefilterEnvGenerator.frag");
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
			plvk::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
		VkPipelineRasterizationStateCreateInfo rasterizationState =
			plvk::pipelineRasterizationStateCreateInfo(VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL, 1.0f, VK_FALSE, 0.0f,
													   0.0f, 0.0f, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
		VkPipelineColorBlendAttachmentState blendAttachmentState = plvk::pipelineColorBlendAttachmentState(VK_TRUE);
		std::vector<VkPipelineColorBlendAttachmentState> blendAttachments{blendAttachmentState};
		VkPipelineColorBlendStateCreateInfo colorBlendState =
			plvk::pipelineColorBlendStateCreateInfo(1, blendAttachments.data());
		VkPipelineDepthStencilStateCreateInfo depthStencilState =
			plvk::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);
		VkPipelineViewportStateCreateInfo viewportState = plvk::pipelineViewportStateCreateInfo(1, 1);
		VkPipelineMultisampleStateCreateInfo multisampleState =
			plvk::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
		std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
		VkPipelineDynamicStateCreateInfo dynamicState = plvk::pipelineDynamicStateCreateInfo(dynamicStateEnables);
		pipeline.mShaders->InitializeFull(VulkanRenderer::GetRenderer()->mDevice, pipelineLayoutInfo, true, size.x,
										  size.y, {}, vertexInputInfo, inputAssemblyState, viewportState,
										  rasterizationState, multisampleState, colorBlendState, dynamicState,
										  pipeline.mShaders->mRenderPass, depthStencilState);

		VkCommandBuffer commandBuffer = VulkanRenderer::GetRenderer()->BeginSingleTimeCommands();

		ConverterPushConstants pushBlock{};

		std::vector<glm::mat4> mats = {
			// POSITIVE_X
			glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
						glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// NEGATIVE_X
			glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
						glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// POSITIVE_Y
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// NEGATIVE_Y
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// POSITIVE_Z
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// NEGATIVE_Z
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
		};

		for (uint32_t m = 0; m < numMips; m++) {
			pushBlock.roughness = (float)m / (float)(numMips - 1);
			for (uint32_t f = 0; f < 6; f++) {
				glm::vec2 currentSize = glm::vec2(size.x * std::pow(0.5f, m), size.y * std::pow(0.5f, m));
				VkImageViewCreateInfo layerImageViewCreateInfo = {};
				layerImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				layerImageViewCreateInfo.image = mPreFilteredTexture->mImage;
				layerImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				layerImageViewCreateInfo.format = mSkyboxFormat;
				layerImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				layerImageViewCreateInfo.subresourceRange.baseMipLevel = m;
				layerImageViewCreateInfo.subresourceRange.levelCount = 1;
				layerImageViewCreateInfo.subresourceRange.baseArrayLayer = f; // Target specific layer
				layerImageViewCreateInfo.subresourceRange.layerCount = 1;

				VkImageView layerImageView;
				vkCreateImageView(VulkanRenderer::GetRenderer()->mDevice, &layerImageViewCreateInfo, nullptr,
								  &layerImageView);

				std::vector<VkImageView> frameBufferAttachments{layerImageView};
				VkFramebufferCreateInfo framebufferInfo{};
				framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferInfo.renderPass = pipeline.mShaders->mRenderPass;
				framebufferInfo.attachmentCount = frameBufferAttachments.size();
				framebufferInfo.pAttachments = frameBufferAttachments.data();
				framebufferInfo.width = currentSize.x;	// Application::Get()->appSizes->sceneSize.x;
				framebufferInfo.height = currentSize.y; // Application::Get()->appSizes->sceneSize.y;
				framebufferInfo.layers = 1;

				VkFramebuffer framebuffer; //= frameBuffers[i];
				if (vkCreateFramebuffer(VulkanRenderer::GetRenderer()->mDevice, &framebufferInfo, nullptr,
										&framebuffer) != VK_SUCCESS) {
					throw std::runtime_error("failed to create framebuffer!");
				}

				// Begin render pass
				VkRenderPassBeginInfo renderPassInfo = {};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassInfo.renderPass = pipeline.mShaders->mRenderPass;
				renderPassInfo.framebuffer = framebuffer;
				renderPassInfo.renderArea.offset = {0, 0};
				renderPassInfo.renderArea.extent = {static_cast<uint32_t>(currentSize.x),
													static_cast<uint32_t>(currentSize.y)};

				VkClearValue clearColor = {0.3f, 0.3f, 0.3f, 1.0f};
				renderPassInfo.clearValueCount = 1;
				renderPassInfo.pClearValues = &clearColor;

				VkRect2D scissor = {};
				scissor.offset = {0, 0};
				scissor.extent = {static_cast<uint32_t>(currentSize.x), static_cast<uint32_t>(currentSize.y)};

				vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

				VkViewport viewport{};
				viewport.x = 0.0f;
				viewport.y = 0.0f;
				viewport.width = currentSize.x;
				viewport.height = (float)(currentSize.y);
				viewport.y = 0; // currentSize.y;
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;

				vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

				// Render scene from cube face's point of view
				vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mShaders->mPipeline);

				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
										pipeline.mShaders->mPipelineLayout, 0, 1, &descriptorset, 0, nullptr);
				pushBlock.mvp = glm::perspective((float)(glm::pi<double>() / 2.0), 1.0f, 0.1f, 512.0f) * mats[f];
				vkCmdPushConstants(commandBuffer, pipeline.mShaders->mPipelineLayout, VK_SHADER_STAGE_ALL, 0,
								   sizeof(ConverterPushConstants), &pushBlock);
				vkCmdDraw(commandBuffer, 36, 1, 0, 0);

				vkCmdEndRenderPass(commandBuffer);
			}
		}
		VulkanRenderer::GetRenderer()->EndSingleTimeCommands(commandBuffer);

		//		VulkanRenderer::GetRenderer()->AddTrackerToImage(mPreFilteredTexture->mImageView, "Pre Filtered
		//Texture", VulkanRenderer::GetRenderer()->mTextureSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void VulkanSkybox::InitializeDescriptorPool() {
		std::array<VkDescriptorPoolSize, 1> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[0].descriptorCount = 6;

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = 6;

		if (vkCreateDescriptorPool(VulkanRenderer::GetRenderer()->mDevice, &poolInfo, nullptr,
								   &this->mDescriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}
	void VulkanSkybox::InitializeDescriptorSets() {
		// Layout
		VkDescriptorSetLayoutBinding skyboxLayoutBinding = plvk::descriptorSetLayoutBinding(
			5, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nullptr, VK_SHADER_STAGE_FRAGMENT_BIT);

		std::array<VkDescriptorSetLayoutBinding, 1> bindings = {skyboxLayoutBinding};
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(VulkanRenderer::GetRenderer()->mDevice, &layoutInfo, nullptr,
										&this->mDescriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}

		// Descriptor set
		VkDescriptorSetAllocateInfo allocInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
		allocInfo.descriptorPool = this->mDescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &mDescriptorSetLayout;

		this->mDescriptorSets.resize(Application::Get()->mRenderer->mMaxFramesInFlight);
		for (unsigned int i = 0; i < Application::Get()->mRenderer->mMaxFramesInFlight; ++i) {
			if (vkAllocateDescriptorSets(VulkanRenderer::GetRenderer()->mDevice, &allocInfo,
										 &this->mDescriptorSets[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate descriptor sets!");
			}
			std::vector<VkWriteDescriptorSet> descriptorWrites = std::vector<VkWriteDescriptorSet>();

			VkDescriptorImageInfo skyboxInfo =
				plvk::descriptorImageInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mPreFilteredTexture->mImageView,
										  mPreFilteredTexture->mSampler);
			// VkDescriptorImageInfo skyboxInfo = plvk::descriptorImageInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			// mIrradianceTexture->mImageView, mIrradianceTexture->mSampler);
			descriptorWrites.push_back(plvk::writeDescriptorSet(
				mDescriptorSets[i], 5, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &skyboxInfo));

			vkUpdateDescriptorSets(VulkanRenderer::GetRenderer()->mDevice,
								   static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}

	void VulkanSkybox::InitializeRenderPass() {
		/* Render pass */
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = VulkanRenderer::GetRenderer()->mLighting->mDeferredEndTextureFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependency.srcStageMask =
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstStageMask =
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(VulkanRenderer::GetRenderer()->mDevice, &renderPassInfo, nullptr,
							   &this->mSkyboxPostEffect->mRenderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}

	void VulkanSkybox::InitializeBRDF() {
		mBRDFLUTTexture = new VulkanTexture();
		mBRDFLUTTexture->CreateTextureImage(VulkanRenderer::GetRenderer()->mDevice, VK_FORMAT_R16G16_SFLOAT, mBrdfSize,
											mBrdfSize, false,
											VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
												VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
											VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED, 1, 0,
											true, VK_SHARING_MODE_EXCLUSIVE, false);
		mBRDFLUTTexture->CreateImageView(VK_FORMAT_R16G16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, 1);
		mBRDFLUTTexture->CreateTextureSampler(VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_MIPMAP_MODE_LINEAR,
											  VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);

		VulkanPlazaPipeline pipeline = VulkanPlazaPipeline();

		// Render pass
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = mBRDFLUTTexture->GetFormat();
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependency.srcStageMask =
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstStageMask =
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 1> attachments = {colorAttachment};
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(VulkanRenderer::GetRenderer()->mDevice, &renderPassInfo, nullptr,
							   &pipeline.mShaders->mRenderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pushConstantRangeCount = 0;

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_FALSE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_NOT_EQUAL;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;
		pipeline.mShaders->mVertexShaderPath = VulkanShadersCompiler::Compile(
			Application::Get()->enginePath + "/Shaders/Vulkan/skybox/brdfGenerator.vert");
		pipeline.mShaders->mFragmentShaderPath = VulkanShadersCompiler::Compile(
			Application::Get()->enginePath + "/Shaders/Vulkan/skybox/brdfGenerator.frag");
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
			plvk::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
		VkPipelineRasterizationStateCreateInfo rasterizationState =
			plvk::pipelineRasterizationStateCreateInfo(VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL, 1.0f, VK_FALSE, 0.0f,
													   0.0f, 0.0f, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
		VkPipelineColorBlendAttachmentState blendAttachmentState = plvk::pipelineColorBlendAttachmentState(VK_TRUE);
		std::vector<VkPipelineColorBlendAttachmentState> blendAttachments{blendAttachmentState};
		VkPipelineColorBlendStateCreateInfo colorBlendState =
			plvk::pipelineColorBlendStateCreateInfo(1, blendAttachments.data());
		VkPipelineDepthStencilStateCreateInfo depthStencilState =
			plvk::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);
		VkPipelineViewportStateCreateInfo viewportState = plvk::pipelineViewportStateCreateInfo(1, 1);
		VkPipelineMultisampleStateCreateInfo multisampleState =
			plvk::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
		std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
		VkPipelineDynamicStateCreateInfo dynamicState = plvk::pipelineDynamicStateCreateInfo(dynamicStateEnables);
		pipeline.mShaders->InitializeFull(VulkanRenderer::GetRenderer()->mDevice, pipelineLayoutInfo, true, mBrdfSize,
										  mBrdfSize, {}, vertexInputInfo, inputAssemblyState, viewportState,
										  rasterizationState, multisampleState, colorBlendState, dynamicState,
										  pipeline.mShaders->mRenderPass, depthStencilState);

		VkCommandBuffer commandBuffer = VulkanRenderer::GetRenderer()->BeginSingleTimeCommands();

		VkImageViewCreateInfo layerImageViewCreateInfo = {};
		layerImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		layerImageViewCreateInfo.image = mBRDFLUTTexture->mImage;
		layerImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		layerImageViewCreateInfo.format = mBRDFLUTTexture->GetFormat();
		layerImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		layerImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		layerImageViewCreateInfo.subresourceRange.levelCount = 1;
		layerImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		layerImageViewCreateInfo.subresourceRange.layerCount = 1;

		VkImageView layerImageView;
		vkCreateImageView(VulkanRenderer::GetRenderer()->mDevice, &layerImageViewCreateInfo, nullptr, &layerImageView);

		std::vector<VkImageView> frameBufferAttachments{layerImageView};
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = pipeline.mShaders->mRenderPass;
		framebufferInfo.attachmentCount = frameBufferAttachments.size();
		framebufferInfo.pAttachments = frameBufferAttachments.data();
		framebufferInfo.width = mBrdfSize;
		framebufferInfo.height = mBrdfSize;
		framebufferInfo.layers = 1;

		VkFramebuffer framebuffer; //= frameBuffers[i];
		if (vkCreateFramebuffer(VulkanRenderer::GetRenderer()->mDevice, &framebufferInfo, nullptr, &framebuffer) !=
			VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}

		// Begin render pass
		VkRenderPassBeginInfo beginRenderPassInfo = {};
		beginRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		beginRenderPassInfo.renderPass = pipeline.mShaders->mRenderPass;
		beginRenderPassInfo.framebuffer = framebuffer;
		beginRenderPassInfo.renderArea.offset = {0, 0};
		beginRenderPassInfo.renderArea.extent = {static_cast<uint32_t>(mBrdfSize), static_cast<uint32_t>(mBrdfSize)};

		VkClearValue clearColor = {0.3f, 0.3f, 0.3f, 1.0f};
		beginRenderPassInfo.clearValueCount = 1;
		beginRenderPassInfo.pClearValues = &clearColor;

		VkRect2D scissor = {};
		scissor.offset = {0, 0};
		scissor.extent = {static_cast<uint32_t>(mBrdfSize), static_cast<uint32_t>(mBrdfSize)};

		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = mBrdfSize;
		viewport.height = -static_cast<int32_t>(mBrdfSize);
		viewport.y = mBrdfSize;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		vkCmdBeginRenderPass(commandBuffer, &beginRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mShaders->mPipeline);
		vkCmdDraw(commandBuffer, 3, 1, 0, 0);

		vkCmdEndRenderPass(commandBuffer);

		VulkanRenderer::GetRenderer()->EndSingleTimeCommands(commandBuffer);
	}

	void VulkanSkybox::Init() {
		this->mSkyboxPostEffect = new VulkanPlazaPipeline();

		// this->mSkyboxPostEffect->mRenderPass = VulkanRenderer::GetRenderer()->mDeferredRenderPass;

		std::string shadersPath;
#ifdef EDITOR_MODE
		shadersPath = Application::Get()->enginePath + "/Editor/DefaultAssets/Skybox/oldskybox/";
#else
		shadersPath = Application::Get()->projectPath + "/";
#endif
		this->mSkyboxPaths[0] = shadersPath + "right.jpg";
		this->mSkyboxPaths[1] = shadersPath + "left.jpg";
		this->mSkyboxPaths[2] = shadersPath + "top.jpg";
		this->mSkyboxPaths[3] = shadersPath + "bottom.jpg";
		this->mSkyboxPaths[4] = shadersPath + "front.jpg";
		this->mSkyboxPaths[5] = shadersPath + "back.jpg";

		this->mSkyboxPaths[0] =
			Application::Get()->enginePath + "/Editor/DefaultAssets/Skybox/" + "autumn_field_4k.hdr";

		this->mResolution = glm::vec2(4096); // Application::Get()->appSizes->sceneSize;
		this->InitializeImageSampler();
		this->InitializeImageView();
		this->InitializeDescriptorPool();
		this->InitializeDescriptorSets();

		InitializeBRDF();
		// this->InitializeRenderPass();

		// this->mFramebuffers.resize(Application::Get()->mRenderer->mMaxFramesInFlight);
		// for (unsigned int i = 0; i < Application::Get()->mRenderer->mMaxFramesInFlight; ++i) {
		//      VkFramebufferCreateInfo framebufferInfo{};
		//      framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		//      framebufferInfo.renderPass = this->mSkyboxPostEffect->mRenderPass;
		//      framebufferInfo.attachmentCount = 1;
		//      std::array<VkImageView, 1> imageViews{ this->mSkyboxImageViews[i] };
		//
		//      framebufferInfo.pAttachments = imageViews.data();
		//      framebufferInfo.width = this->mResolution.x;
		//      framebufferInfo.height = this->mResolution.y;
		//      framebufferInfo.layers = 1;
		//      if (vkCreateFramebuffer(VulkanRenderer::GetRenderer()->mDevice, &framebufferInfo, nullptr,
		//      &this->mFramebuffers[i]) != VK_SUCCESS) {
		//           throw std::runtime_error("Failed to create framebuffer!");
		//      }
		// }

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_ALL;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(VulkanSkybox::PushConstants);

		this->mPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		this->mPipelineLayoutInfo.setLayoutCount = 1;
		this->mPipelineLayoutInfo.pSetLayouts = &this->mDescriptorSetLayout;
		this->mPipelineLayoutInfo.pushConstantRangeCount = 1;
		this->mPipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		VkDescriptorSetLayoutBinding skyboxLayoutBinding = plvk::descriptorSetLayoutBinding(
			0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nullptr, VK_SHADER_STAGE_FRAGMENT_BIT);

		std::vector<VkDescriptorSetLayoutBinding> bindings = {skyboxLayoutBinding};
		VkDescriptorSetLayoutCreateInfo layoutInfo = plvk::descriptorSetLayoutCreateInfo(
			bindings, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT);
		mSkyboxPostEffect->mRenderPass = VulkanRenderer::GetRenderer()->mGeometryPassRenderer.mRenderPass;
		if (vkCreateDescriptorSetLayout(VulkanRenderer::GetRenderer()->mDevice, &layoutInfo, nullptr,
										&mSkyboxPostEffect->mShaders->mDescriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}

		// this->mSkyboxPostEffect->mShaders = new VulkanShaders(vertexPath, fragmentPath, "");
		mScreenSize = Application::Get()->appSizes->sceneSize;
		std::vector<VkImageView> attachments{VulkanRenderer::GetRenderer()->mLighting->mDeferredEndTexture.mImageView,
											 VulkanRenderer::GetRenderer()->mDepthImageView};
		mSkyboxPostEffect->InitializeFramebuffer(attachments.data(), attachments.size(), this->mScreenSize, 1);
		// mSkyboxPostEffect->Init(
		//	VulkanShadersCompiler::Compile(Application::Get()->enginePath + "/Shaders/Vulkan/skybox/skybox.vert"),
		//	VulkanShadersCompiler::Compile(Application::Get()->enginePath + "/Shaders/Vulkan/skybox/skybox.frag"),
		//	"",
		//	VulkanRenderer::GetRenderer()->mDevice,
		//	mScreenSize,
		//	mSkyboxPostEffect->mShaders->mDescriptorSetLayout,
		//	this->mPipelineLayoutInfo);

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_FALSE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_NOT_EQUAL;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;
		this->mSkyboxPostEffect->mShaders->mVertexShaderPath =
			VulkanShadersCompiler::Compile(Application::Get()->enginePath + "/Shaders/Vulkan/skybox/skybox.vert");
		this->mSkyboxPostEffect->mShaders->mFragmentShaderPath =
			VulkanShadersCompiler::Compile(Application::Get()->enginePath + "/Shaders/Vulkan/skybox/skybox.frag");
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
			plvk::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
		VkPipelineRasterizationStateCreateInfo rasterizationState =
			plvk::pipelineRasterizationStateCreateInfo(VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL, 1.0f, VK_FALSE, 0.0f,
													   0.0f, 0.0f, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
		VkPipelineColorBlendAttachmentState blendAttachmentState = plvk::pipelineColorBlendAttachmentState(VK_TRUE);
		std::vector<VkPipelineColorBlendAttachmentState> blendAttachments{blendAttachmentState, blendAttachmentState,
																		  blendAttachmentState, blendAttachmentState};
		VkPipelineColorBlendStateCreateInfo colorBlendState =
			plvk::pipelineColorBlendStateCreateInfo(4, blendAttachments.data());
		VkPipelineDepthStencilStateCreateInfo depthStencilState =
			plvk::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_FALSE, VK_COMPARE_OP_LESS);
		VkPipelineViewportStateCreateInfo viewportState = plvk::pipelineViewportStateCreateInfo(1, 1);
		VkPipelineMultisampleStateCreateInfo multisampleState =
			plvk::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
		std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
		VkPipelineDynamicStateCreateInfo dynamicState = plvk::pipelineDynamicStateCreateInfo(dynamicStateEnables);
		this->mSkyboxPostEffect->mShaders->InitializeFull(
			VulkanRenderer::GetRenderer()->mDevice, this->mPipelineLayoutInfo, true,
			Application::Get()->appSizes->sceneSize.x, Application::Get()->appSizes->sceneSize.y, {}, {},
			inputAssemblyState, viewportState, rasterizationState, multisampleState, colorBlendState, dynamicState,
			mSkyboxPostEffect->mRenderPass, depthStencilState);

		/* Update deferred irradiance binding */
		for (unsigned int i = 0; i < Application::Get()->mRenderer->mMaxFramesInFlight; ++i) {
			std::vector<VkWriteDescriptorSet> descriptorWrites = std::vector<VkWriteDescriptorSet>();
			VkDescriptorImageInfo brdfInfo =
				plvk::descriptorImageInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
										  VulkanRenderer::GetRenderer()->mSkybox->mBRDFLUTTexture->mImageView,
										  VulkanRenderer::GetRenderer()->mSkybox->mBRDFLUTTexture->mSampler);
			descriptorWrites.push_back(plvk::writeDescriptorSet(
				VulkanRenderer::GetRenderer()->mGeometryPassRenderer.mShaders->mDescriptorSets[i], 6, 0,
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &brdfInfo));
			VkDescriptorImageInfo preFilterInfo =
				plvk::descriptorImageInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
										  VulkanRenderer::GetRenderer()->mSkybox->mPreFilteredTexture->mImageView,
										  VulkanRenderer::GetRenderer()->mSkybox->mPreFilteredTexture->mSampler);
			descriptorWrites.push_back(plvk::writeDescriptorSet(
				VulkanRenderer::GetRenderer()->mGeometryPassRenderer.mShaders->mDescriptorSets[i], 7, 0,
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &preFilterInfo));
			VkDescriptorImageInfo irradianceInfo =
				plvk::descriptorImageInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
										  VulkanRenderer::GetRenderer()->mSkybox->mIrradianceTexture->mImageView,
										  VulkanRenderer::GetRenderer()->mSkybox->mIrradianceTexture->mSampler);
			descriptorWrites.push_back(plvk::writeDescriptorSet(
				VulkanRenderer::GetRenderer()->mGeometryPassRenderer.mShaders->mDescriptorSets[i], 8, 0,
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &irradianceInfo));
			vkUpdateDescriptorSets(VulkanRenderer::GetRenderer()->mDevice,
								   static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}

	void VulkanSkybox::DrawSkybox() {
		if (!mSkyboxMesh)
			mSkyboxMesh = (VulkanMesh*)Editor::DefaultModels::Cube();
		this->mCommandBuffer = *VulkanRenderer::GetRenderer()->mActiveCommandBuffer;
		PLAZA_PROFILE_SECTION("Draw Skybox");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		// renderPassInfo.renderPass = this->mSkyboxPostEffect->mRenderPass;
		// renderPassInfo.framebuffer =
		// this->mFramebuffers[Application::Get()->mRenderer->mCurrentFrame];//mSwapChainFramebuffers[0];//mSwapChainFramebuffers[imageIndex];
		renderPassInfo.renderPass = mSkyboxPostEffect->mRenderPass;
		renderPassInfo.framebuffer = mSkyboxPostEffect->mFramebuffer;

		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = VulkanRenderer::GetRenderer()->mSwapChainExtent;

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = {{0.0f, 0.0f, 0.0f, 0.0f}};
		clearValues[1].depthStencil = {0.0f, 0};
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		viewport.width = static_cast<float>(VulkanRenderer::GetRenderer()->mSwapChainExtent.width);
		viewport.height = static_cast<float>(VulkanRenderer::GetRenderer()->mSwapChainExtent.height);
		// viewport.y = this->mResolution.y;

		renderPassInfo.renderArea.extent.width = Application::Get()->appSizes->sceneSize.x;
		renderPassInfo.renderArea.extent.height = Application::Get()->appSizes->sceneSize.y;

		viewport.width = Application::Get()->appSizes->sceneSize.x;
		viewport.height = -Application::Get()->appSizes->sceneSize.y;
		viewport.y = Application::Get()->appSizes->sceneSize.y;

		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = VulkanRenderer::GetRenderer()->mSwapChainExtent;
		scissor.extent.width = Application::Get()->appSizes->sceneSize.x;
		scissor.extent.height = Application::Get()->appSizes->sceneSize.y;

		// vkCmdBeginRenderPass(this->mCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		// vkCmdSetViewport(this->mCommandBuffer, 0, 1, &viewport);
		// vkCmdSetScissor(this->mCommandBuffer, 0, 1, &scissor);

		vkCmdBindPipeline(this->mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
						  this->mSkyboxPostEffect->mShaders->mPipeline);

		// this->UpdateAndPushConstants(mCommandBuffer);
		vkCmdBindDescriptorSets(this->mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
								this->mSkyboxPostEffect->mShaders->mPipelineLayout, 0, 1,
								&this->mDescriptorSets[Application::Get()->mRenderer->mCurrentFrame], 0, nullptr);

		VkDeviceSize offsets[] = {0, 0};

		pushData.skyboxIntensity = VulkanRenderer::GetRenderer()->mSkyboxIntensity;
		pushData.gamma = VulkanRenderer::GetRenderer()->gamma;
		pushData.exposure = VulkanRenderer::GetRenderer()->exposure;
		pushData.projection = Application::Get()->activeCamera->GetProjectionMatrix();
		pushData.view = Application::Get()->activeCamera->GetViewMatrix();
		vkCmdPushConstants(this->mCommandBuffer, this->mSkyboxPostEffect->mShaders->mPipelineLayout,
						   VK_SHADER_STAGE_ALL, 0, sizeof(VulkanSkybox::PushConstants), &pushData);

		// VkDeviceSize offsets2[1] = { 0 };
		// vkCmdBindIndexBuffer(mCommandBuffer, VulkanRenderer::GetRenderer()->mMainIndexBuffer, 0,
		// VK_INDEX_TYPE_UINT32); vkCmdBindVertexBuffers(mCommandBuffer, 0, 1,
		// &VulkanRenderer::GetRenderer()->mMainVertexBuffer, offsets2); vkCmdBindVertexBuffers(mCommandBuffer, 1, 1,
		// &VulkanRenderer::GetRenderer()->mMainInstanceMatrixBuffers[VulkanRenderer::GetRenderer()->mCurrentFrame],
		// offsets2);

		vkCmdDrawIndexed(mCommandBuffer, static_cast<uint32_t>(mSkyboxMesh->indices.size()), 1,
						 mSkyboxMesh->indicesOffset, mSkyboxMesh->verticesOffset, mSkyboxMesh->instanceOffset);
		// vkCmdEndRenderPass(mCommandBuffer);
	}

	void VulkanSkybox::UpdateAndPushConstants(VkCommandBuffer commandBuffer) {}

	void VulkanSkybox::Terminate() {}

} // namespace Plaza
