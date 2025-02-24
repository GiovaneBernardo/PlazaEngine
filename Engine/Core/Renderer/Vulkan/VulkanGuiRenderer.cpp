#pragma once
#include "VulkanGuiRenderer.h"
#include "Renderer.h"
#include "Engine/Core/Scene.h"
#include "VulkanShaders.h"

namespace Plaza {
	void VulkanGuiRenderer::PreparePipeline()
	{
		std::string vertexPath = VulkanShadersCompiler::Compile(Application::Get()->enginePath + "/Shaders/Vulkan/gui/text.vert");
		std::string fragmentPath = VulkanShadersCompiler::Compile(Application::Get()->enginePath + "/Shaders/Vulkan/gui/text.frag");

		this->mTextPipeline = new VulkanPlazaPipeline();
		this->mTextPipeline->mShaders = new VulkanShaders(vertexPath, fragmentPath, "");
		this->mTextPipeline->mShaders->mRenderPass = mRenderer->mRenderPass;
		//this->mTextPipeline->mShaders->InitializeFull(*mDevice, )

		this->mPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		this->mPipelineLayoutInfo.setLayoutCount = 1;
		this->mPipelineLayoutInfo.pSetLayouts = &this->mDescriptorSetLayout;

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_FALSE;
		depthStencil.depthWriteEnable = VK_FALSE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencil.back.compareOp = VK_COMPARE_OP_ALWAYS;

		VkPipelineColorBlendAttachmentState blendAttachmentState{};
		blendAttachmentState.blendEnable = VK_TRUE;
		blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
		blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &blendAttachmentState;

		std::array<VkVertexInputBindingDescription, 2> vertexInputBindings{};
		vertexInputBindings[0].binding = 0;
		vertexInputBindings[0].stride = sizeof(glm::vec4);
		vertexInputBindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		vertexInputBindings[1].binding = 1;
		vertexInputBindings[1].stride = sizeof(glm::vec4);
		vertexInputBindings[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		std::array<VkVertexInputAttributeDescription, 2> vertexInputAttributes{};
		vertexInputAttributes[0].location = 0;
		vertexInputAttributes[0].binding = 0;
		vertexInputAttributes[0].format = VK_FORMAT_R32G32_SFLOAT;
		vertexInputAttributes[0].offset = 0;

		vertexInputAttributes[1].location = 1;
		vertexInputAttributes[1].binding = 1;
		vertexInputAttributes[1].format = VK_FORMAT_R32G32_SFLOAT;
		vertexInputAttributes[1].offset = sizeof(glm::vec2);

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_NONE;//BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
		inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		inputAssemblyState.primitiveRestartEnable = VK_FALSE;

		this->mTextPipeline->mShaders->InitializeFull(*mDevice, this->mPipelineLayoutInfo, true, Application::Get()->appSizes->sceneSize.x, Application::Get()->appSizes->sceneSize.y, {}, {}, inputAssemblyState, {}, rasterizer, {}, colorBlending, {}, this->mRenderer->mRenderPass, depthStencil, std::vector<VkVertexInputBindingDescription>(vertexInputBindings.begin(), vertexInputBindings.end()), std::vector<VkVertexInputAttributeDescription>(vertexInputAttributes.begin(), vertexInputAttributes.end()));

	}

	void VulkanGuiRenderer::Init() {
		this->mRenderer = VulkanRenderer::GetRenderer();
		this->mDevice = &this->mRenderer->mDevice;

		//this->InitializeRenderPass();

		const uint32_t fontWidth = STB_FONT_consolas_24_latin1_BITMAP_WIDTH;
		const uint32_t fontHeight = STB_FONT_consolas_24_latin1_BITMAP_HEIGHT;

		static unsigned char font24pixels[fontHeight][fontWidth];
		stb_font_consolas_24_latin1(stbFontData, font24pixels, fontHeight);

		//// Vertex buffer
		//VkDeviceSize bufferSize = TEXTOVERLAY_MAX_CHAR_COUNT * sizeof(glm::vec4);
		//
		//VkBufferCreateInfo bufferInfo{};
		//bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		//bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		//bufferInfo.size = bufferSize;
		//
		//vkCreateBuffer(*mDevice, &bufferInfo, nullptr, &mBuffer);
		//
		//VkMemoryRequirements memReqs;
		//VkMemoryAllocateInfo allocInfo{};
		//allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		//
		//vkGetBufferMemoryRequirements(*mDevice, mBuffer, &memReqs);
		//allocInfo.allocationSize = memReqs.size;
		//allocInfo.memoryTypeIndex = mRenderer->FindMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		//
		//vkAllocateMemory(*mDevice, &allocInfo, nullptr, &mMemory);
		//vkBindBufferMemory(*mDevice, mBuffer, mMemory, 0);
		//
		//// Font texture
		//VkImageCreateInfo imageInfo{};
		//imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		//imageInfo.imageType = VK_IMAGE_TYPE_2D;
		//imageInfo.format = VK_FORMAT_R8_UNORM;
		//imageInfo.extent.width = fontWidth;
		//imageInfo.extent.height = fontHeight;
		//imageInfo.extent.depth = 1;
		//imageInfo.mipLevels = 1;
		//imageInfo.arrayLayers = 1;
		//imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		//imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		//imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		//imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		//imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		//
		//vkCreateImage(*mDevice, &imageInfo, nullptr, &mImage);
		//
		//vkGetImageMemoryRequirements(*mDevice, mImage, &memReqs);
		//allocInfo.allocationSize = memReqs.size;
		//allocInfo.memoryTypeIndex = mRenderer->FindMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		//
		//vkAllocateMemory(*mDevice, &allocInfo, nullptr, &mImageMemory);
		//vkBindImageMemory(*mDevice, mImage, mImageMemory, 0);
		//
		//// Staging
		//
		//struct {
		//	VkDeviceMemory memory;
		//	VkBuffer buffer;
		//} stagingBuffer;
		//
		//VkBufferCreateInfo bufferCreateInfo{};
		//bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		//bufferCreateInfo.size = allocInfo.allocationSize;
		//bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		//bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		//
		//vkCreateBuffer(*mDevice, &bufferCreateInfo, nullptr, &stagingBuffer.buffer);
		//
		//// Get memory requirements for the staging buffer (alignment, memory type bits)
		//vkGetBufferMemoryRequirements(*mDevice, stagingBuffer.buffer, &memReqs);
		//
		//allocInfo.allocationSize = memReqs.size;
		//// Get memory type index for a host visible buffer
		//allocInfo.memoryTypeIndex = mRenderer->FindMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		//
		//vkAllocateMemory(*mDevice, &allocInfo, nullptr, &stagingBuffer.memory);
		//vkBindBufferMemory(*mDevice, stagingBuffer.buffer, stagingBuffer.memory, 0);
		//
		//uint8_t* data;
		//vkMapMemory(*mDevice, stagingBuffer.memory, 0, allocInfo.allocationSize, 0, (void**)&data);
		//// Size of the font texture is WIDTH * HEIGHT * 1 byte (only one channel)
		//memcpy(data, &font24pixels[0][0], fontWidth * fontHeight);
		//vkUnmapMemory(*mDevice, stagingBuffer.memory);
		//
		//// Copy to image
		//
		//VkCommandBuffer copyCmd = mRenderer->CreateCommandBuffer();
		//VkCommandBufferBeginInfo cmdBufferBeginInfo{};
		//cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		//vkBeginCommandBuffer(copyCmd, &cmdBufferBeginInfo);
		//
		//// Prepare for transfer
		////  vks::tools::setImageLayout(
		////  	copyCmd,
		////  	image,
		////  	VK_IMAGE_ASPECT_COLOR_BIT,
		////  	VK_IMAGE_LAYOUT_UNDEFINED,
		////  	VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		//mRenderer->TransitionImageLayout(this->mImage, VK_FORMAT_R8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
		//
		//VkBufferImageCopy bufferCopyRegion = {};
		//bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		//bufferCopyRegion.imageSubresource.mipLevel = 0;
		//bufferCopyRegion.imageSubresource.layerCount = 1;
		//bufferCopyRegion.imageExtent.width = fontWidth;
		//bufferCopyRegion.imageExtent.height = fontHeight;
		//bufferCopyRegion.imageExtent.depth = 1;
		//
		//vkCmdCopyBufferToImage(
		//	copyCmd,
		//	stagingBuffer.buffer,
		//	mImage,
		//	VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		//	1,
		//	&bufferCopyRegion
		//);
		//
		//// Prepare for shader read
		////    vks::tools::setImageLayout(
		////    	copyCmd,
		////    	image,
		////    	VK_IMAGE_ASPECT_COLOR_BIT,
		////    	VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		////    	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		//
		//FlushCommandBuffer(copyCmd, this->mRenderer->mGraphicsQueue, mRenderer->mCommandPool, true);
		//
		//mRenderer->TransitionImageLayout(this->mImage, VK_FORMAT_R8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
		//
		//vkFreeMemory(*mDevice, stagingBuffer.memory, nullptr);
		//vkDestroyBuffer(*mDevice, stagingBuffer.buffer, nullptr);
		//
		//VkImageViewCreateInfo imageViewInfo{};
		//imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		//imageViewInfo.image = mImage;
		//imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		//imageViewInfo.format = imageInfo.format;
		//imageViewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B,	VK_COMPONENT_SWIZZLE_A };
		//imageViewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		//vkCreateImageView(*mDevice, &imageViewInfo, nullptr, &mImageView);
		//
		//// Sampler
		//VkSamplerCreateInfo samplerInfo{};
		//samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		//samplerInfo.maxAnisotropy = 1.0f;
		//samplerInfo.magFilter = VK_FILTER_LINEAR;
		//samplerInfo.minFilter = VK_FILTER_LINEAR;
		//samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		//samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		//samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		//samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		//samplerInfo.mipLodBias = 0.0f;
		//samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
		//samplerInfo.minLod = 0.0f;
		//samplerInfo.maxLod = 1.0f;
		//samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		//vkCreateSampler(*mDevice, &samplerInfo, nullptr, &mSampler);
		//
		//// Descriptor
		//// Font uses a separate descriptor pool
		//std::array<VkDescriptorPoolSize, 1> poolSizes;
		//poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		//poolSizes[0].descriptorCount = 1;
		//
		//VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		//descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		//descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		//descriptorPoolInfo.pPoolSizes = poolSizes.data();
		//descriptorPoolInfo.maxSets = 1;
		//
		//vkCreateDescriptorPool(*mDevice, &descriptorPoolInfo, nullptr, &mDescriptorPool);
		//
		//// Descriptor set layout
		//std::array<VkDescriptorSetLayoutBinding, 1> setLayoutBindings;
		//VkDescriptorSetLayoutBinding setLayoutBinding{};
		//setLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		//setLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		//setLayoutBinding.binding = 0;
		//setLayoutBinding.descriptorCount = 1;
		//setLayoutBindings[0] = setLayoutBinding;
		//
		//VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
		//descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		//descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();
		//descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
		//
		//vkCreateDescriptorSetLayout(*mDevice, &descriptorSetLayoutInfo, nullptr, &mDescriptorSetLayout);
		//
		//// Descriptor set
		//VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
		//descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		//descriptorSetAllocInfo.descriptorPool = mDescriptorPool;
		//descriptorSetAllocInfo.pSetLayouts = &mDescriptorSetLayout;
		//descriptorSetAllocInfo.descriptorSetCount = 1;
		//
		//vkAllocateDescriptorSets(*mDevice, &descriptorSetAllocInfo, &mDescriptorSet);
		//
		//// Descriptor for the font image
		//VkDescriptorImageInfo texDescriptor{};
		//texDescriptor.sampler = mSampler;
		//texDescriptor.imageView = mImageView;
		//texDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		//
		//std::array<VkWriteDescriptorSet, 1> writeDescriptorSets;
		//VkWriteDescriptorSet writeDescriptorSet{};
		//writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		//writeDescriptorSet.dstSet = mDescriptorSet;
		//writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		//writeDescriptorSet.dstBinding = 0;
		//writeDescriptorSet.pImageInfo = &texDescriptor;
		//writeDescriptorSet.descriptorCount = 1;
		//writeDescriptorSets[0] = writeDescriptorSet;//vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &texDescriptor);
		//vkUpdateDescriptorSets(*mDevice, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
		//
		//PreparePipeline();
		//
		//this->mRenderer->AddTrackerToImage(this->mImageView, "Text Rendering", nullptr, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void VulkanGuiRenderer::AddText(std::string text, float x, float y, float scale, TextAlign align, glm::vec4*& mapped, int& letters)
	{
		static bool stbInitialized = false;
		static const uint32_t fontWidth = STB_FONT_consolas_24_latin1_BITMAP_WIDTH;
		static const uint32_t fontHeight = STB_FONT_consolas_24_latin1_BITMAP_HEIGHT;

		static unsigned char font24pixels[fontHeight][fontWidth];
		static stb_fontchar stbFontData[STB_FONT_consolas_24_latin1_NUM_CHARS];

		if (!stbInitialized) {
			stb_font_consolas_24_latin1(stbFontData, font24pixels, fontHeight);
			stbInitialized = true;
		}


		const uint32_t firstChar = STB_FONT_consolas_24_latin1_FIRST_CHAR;

		assert(mapped != nullptr);

		uint32_t frameBufferWidth = (uint32_t)Application::Get()->appSizes->sceneSize.x;
		uint32_t frameBufferHeight = (uint32_t)Application::Get()->appSizes->sceneSize.y;

		const float charW = 1.5f * scale / *&frameBufferWidth;
		const float charH = 1.5f * scale / *&frameBufferHeight;

		float fbW = (float)*&frameBufferWidth;
		float fbH = (float)*&frameBufferHeight;
		x = (x / fbW * 2.0f) - 1.0f;
		y = (y / fbH * 2.0f) - 1.0f;

		// Calculate text width
		float textWidth = 0;
		for (auto letter : text)
		{
			stb_fontchar* charData = &stbFontData[(uint32_t)letter - firstChar];
			textWidth += charData->advance * charW;
		}

		switch (align)
		{
		case alignRight:
			x -= textWidth;
			break;
		case alignCenter:
			x -= textWidth / 2.0f;
			break;
		case alignLeft:
			break;
		}
		float xOffset = 0.0f;//-(Application->appSizes->sceneImageStart.x / Application->appSizes->appSize.x);
		float yOffset = 0.0f;//-(Application->appSizes->sceneImageStart.y / Application->appSizes->appSize.y);
		// Generate a uv mapped quad per char in the new text
		for (auto letter : text)
		{
			stb_fontchar* charData = &stbFontData[(uint32_t)letter - firstChar];

			mapped->x = xOffset + (x + (float)charData->x0 * charW);
			mapped->y = yOffset + (y + (float)charData->y0 * charH);
			mapped->z = charData->s0;
			mapped->w = charData->t0;
			mapped++;

			mapped->x = xOffset + (x + (float)charData->x1 * charW);
			mapped->y = yOffset + (y + (float)charData->y0 * charH);
			mapped->z = charData->s1;
			mapped->w = charData->t0;
			mapped++;

			mapped->x = xOffset + (x + (float)charData->x0 * charW);
			mapped->y = yOffset + (y + (float)charData->y1 * charH);
			mapped->z = charData->s0;
			mapped->w = charData->t1;
			mapped++;

			mapped->x = xOffset + (x + (float)charData->x1 * charW);
			mapped->y = yOffset + (y + (float)charData->y1 * charH);
			mapped->z = charData->s1;
			mapped->w = charData->t1;
			mapped++;

			x += charData->advance * charW;

			letters++;
		}
	}

	void VulkanGuiRenderer::RenderText(Scene* scene, Drawing::UI::TextRenderer* textRendererComponent) {
		VkCommandBuffer cmdBuffer = *mRenderer->mActiveCommandBuffer;
		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->mTextPipeline->mShaders->mPipeline);
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->mTextPipeline->mShaders->mPipelineLayout, 0, 1, &mDescriptorSet, 0, NULL);

		vkMapMemory(*mDevice, mMemory, 0, VK_WHOLE_SIZE, 0, (void**)&mappede);
		numLetters = 0;

		for (const uint64_t& uuid : SceneView<Plaza::Drawing::UI::TextRenderer>(scene)) {
			auto& component = *scene->GetComponent<Plaza::Drawing::UI::TextRenderer>(uuid);
			this->AddText(component.mText, component.mPosX, component.mPosY, component.mScale, TextAlign::alignLeft, mappede, numLetters);
		}



		vkUnmapMemory(*mDevice, mMemory);
		mappede = nullptr;

		VkDeviceSize offsets = 0;
		vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &mBuffer, &offsets);
		vkCmdBindVertexBuffers(cmdBuffer, 1, 1, &mBuffer, &offsets);

		// One draw command for every character. This is okay for a debug overlay, but not optimal
		// In a real-world application one would try to batch draw commands
		for (uint32_t j = 0; j < numLetters; j++) {
			vkCmdDraw(cmdBuffer, 4, 1, j * 4, 0);
		}
	}

	void VulkanGuiRenderer::Terminate() {

	}

#define DEFAULT_FENCE_TIMEOUT 100000000000
	void VulkanGuiRenderer::FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, bool free) {
		if (commandBuffer == VK_NULL_HANDLE)
		{
			return;
		}

		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		// Create fence to ensure that the command buffer has finished executing
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = 0;
		VkFence fence;
		vkCreateFence(*mDevice, &fenceInfo, nullptr, &fence);
		// Submit to the queue
		vkQueueSubmit(queue, 1, &submitInfo, fence);
		// Wait for the fence to signal that command buffer has finished executing
		vkWaitForFences(*mDevice, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT);
		vkDestroyFence(*mDevice, fence, nullptr);
		if (free)
		{
			vkFreeCommandBuffers(*mDevice, pool, 1, &commandBuffer);
		}
	}

	void VulkanGuiRenderer::InitializeRenderPass() {
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
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 1> attachments = { colorAttachment };
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(*mDevice, &renderPassInfo, nullptr, &this->mRenderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}
}