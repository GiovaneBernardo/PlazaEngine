#include <Engine/Core/PreCompiledHeaders.h>
#include "VulkanPicking.h"
#include "VulkanPlazaPipeline.h"
#include "Editor/DefaultAssets/Models/DefaultModels.h"
#include "VulkanShaders.h"
#include "Renderer.h"
#include "Engine/Core/Scene.h"

namespace Plaza {
	void VulkanPicking::Init() {
		this->mResolution = Application::Get()->appSizes->sceneSize;

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_ALL;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(VulkanPicking::PushConstants);

		this->InitializeDescriptorSets();

		this->mPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		this->mPipelineLayoutInfo.setLayoutCount = 1;
		this->mPipelineLayoutInfo.pSetLayouts = &this->mDescriptorSetLayout;
		this->mPipelineLayoutInfo.pushConstantRangeCount = 1;
		this->mPipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		this->InitializePicking();
		this->InitializeOutline();

		//VulkanRenderer::GetRenderer()->AddTrackerToImage(this->mPickingTextureImage, "Picking Texture");
		//VulkanRenderer::GetRenderer()->AddTrackerToImage(this->mDepthImageView, "Picking Depth Texture", nullptr, VK_IMAGE_LAYOUT_GENERAL);
	}

	void VulkanPicking::InitializeImageView(VkImageView& imageView, VkImage& image, VkFormat format) {
		VkImageCreateInfo imageCreateInfo = {};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.extent.width = Application::Get()->appSizes->sceneSize.x;
		imageCreateInfo.extent.height = Application::Get()->appSizes->sceneSize.y;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.format = format;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;

		if (vkCreateImage(VulkanRenderer::GetRenderer()->mDevice, &imageCreateInfo, nullptr, &image) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements memoryRequirements;
		vkGetImageMemoryRequirements(VulkanRenderer::GetRenderer()->mDevice, image, &memoryRequirements);

		VkMemoryAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = memoryRequirements.size;
		allocateInfo.memoryTypeIndex = VulkanRenderer::GetRenderer()->FindMemoryType(
			memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkDeviceMemory imageMemory;
		if (vkAllocateMemory(VulkanRenderer::GetRenderer()->mDevice, &allocateInfo, nullptr, &imageMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}
		vkBindImageMemory(VulkanRenderer::GetRenderer()->mDevice, image, imageMemory, 0);

		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = image;

		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = format;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(VulkanRenderer::GetRenderer()->mDevice, &createInfo, nullptr, &imageView) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image views!");
		}
	}

	void VulkanPicking::InitializeFramebuffer() {
		std::array<VkImageView, 2> attachments = {
			mPickingTextureImageView,
			mDepthImageView
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = this->mRenderPickingTexturePostEffects->mRenderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = Application::Get()->appSizes->sceneSize.x;
		framebufferInfo.height = Application::Get()->appSizes->sceneSize.y;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(VulkanRenderer::GetRenderer()->mDevice, &framebufferInfo, nullptr, &mFramebuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}

	}

	void VulkanPicking::InitializeDescriptorSets() {
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 0;//static_cast<uint32_t>(bindings.size());
		//layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(VulkanRenderer::GetRenderer()->mDevice, &layoutInfo, nullptr, &this->mDescriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	void VulkanPicking::InitializeRenderPass() {
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = VK_FORMAT_R32G32B32A32_UINT;
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
		depthAttachment.format = VulkanRenderer::GetRenderer()->FindDepthFormat();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
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
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(VulkanRenderer::GetRenderer()->mDevice, &renderPassInfo, nullptr, &this->mRenderPickingTexturePostEffects->mRenderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}

	void VulkanPicking::InitializePicking() {
		const std::string pickingVertexPath = VulkanShadersCompiler::Compile(Application::Get()->enginePath + "\\Shaders\\Vulkan\\picking\\picking.vert");
		const std::string pickingFragmentPath = VulkanShadersCompiler::Compile(Application::Get()->enginePath + "\\Shaders\\Vulkan\\picking\\picking.frag");

		this->mRenderPickingTexturePostEffects = new VulkanPlazaPipeline();

		this->InitializeRenderPass();

		this->InitializeImageView(this->mPickingTextureImageView, this->mPickingTextureImage, VK_FORMAT_R32G32B32A32_UINT);

		VkDeviceMemory imageMemory;
		VkFormat depthFormat = VulkanRenderer::GetRenderer()->FindDepthFormat();
		VulkanRenderer::GetRenderer()->CreateImage(this->mResolution.x, this->mResolution.y, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->mDepthTextureImage, imageMemory);
		this->mDepthImageView = VulkanRenderer::GetRenderer()->CreateImageView(this->mDepthTextureImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
		this->InitializeFramebuffer();
		std::cout << "3 \n";

		this->mRenderPickingTexturePostEffects->mShaders = new VulkanShaders(pickingVertexPath, pickingFragmentPath, "");

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		std::cout << "0 \n";
		this->mRenderPickingTexturePostEffects->mShaders->InitializeFull(
			VulkanRenderer::GetRenderer()->mDevice,
			this->mPipelineLayoutInfo,
			true,
			this->mResolution.x,
			this->mResolution.y,
			{}, {}, {}, {}, rasterizer, {}, colorBlending, {},
			this->mRenderPickingTexturePostEffects->mRenderPass,
			depthStencil);
		std::cout << "end \n";
	}

	void VulkanPicking::InitializeOutline() {
		//const std::string outlineVertexPath = Application::Get()->enginePath + "\\Shaders\\Vulkan\\picking\\outline.vert";
		//const std::string outlineFragmentPath = Application::Get()->enginePath + "\\Shaders\\Vulkan\\picking\\outline.frag";
		//
		//this->mRenderPickingTexturePostEffects = new VulkanPostEffects();
		//
		//this->mOutlinePostEffects->mRenderPass = VulkanRenderer::GetRenderer()->mRenderPass;
		//this->mOutlinePostEffects->mShaders = new VulkanShaders(outlineVertexPath, outlineFragmentPath, "");
		//
		//this->mOutlinePostEffects->mShaders->InitializeFull(
		//	VulkanRenderer::GetRenderer()->mDevice,
		//	this->mPipelineLayoutInfo,
		//	true,
		//	Application::Get()->appSizes->sceneSize.x,
		//	Application::Get()->appSizes->sceneSize.y,
		//	{}, {}, {}, {}, {}, {}, {}, {},
		//	this->mRenderPickingTexturePostEffects->mRenderPass,
		//	{});
	}

	void VulkanPicking::DrawMeshToPickingTexture(const MeshRenderer& meshRenderer, VkCommandBuffer& commandBuffer) {
		VulkanMesh* mesh = (VulkanMesh*)meshRenderer.GetMesh();
		if (!mesh || mesh->vertices.size() < 3) {
			return;
		}
		
		VkDeviceSize offsets[] = { 0, 0 };
		VkCommandBuffer activeCommandBuffer = commandBuffer;
		
		VulkanPicking::PushConstants pushData{};
		pushData.projection = Application::Get()->activeCamera->GetProjectionMatrix();
		pushData.view = Application::Get()->activeCamera->GetViewMatrix();
		pushData.model = Scene::GetActiveScene()->GetComponent<TransformComponent>(meshRenderer.mUuid)->mWorldMatrix;
		pushData.uuid1 = (uint32_t)(meshRenderer.mUuid >> 32);
		pushData.uuid2 = (uint32_t)(meshRenderer.mUuid & 0xFFFFFFFF);
		
		vkCmdPushConstants(commandBuffer, this->mRenderPickingTexturePostEffects->mShaders->mPipelineLayout, VK_SHADER_STAGE_ALL, 0, sizeof(VulkanPicking::PushConstants), &pushData);

		vkCmdDrawIndexed(activeCommandBuffer, static_cast<uint32_t>(mesh->indices.size()), 1, mesh->indicesOffset, mesh->verticesOffset, mesh->instanceOffset);
	}

	void VulkanPicking::DrawSelectedObjectsUuid(Scene* scene) {
		PLAZA_PROFILE_SECTION("DrawSelectedObjectsUuid");

		//VkCommandBuffer& commandBuffer = *VulkanRenderer::GetRenderer()->mActiveCommandBuffer;
		VkCommandBuffer commandBuffer = VulkanRenderer::GetRenderer()->BeginSingleTimeCommands();

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();
		renderPassInfo.renderPass = this->mRenderPickingTexturePostEffects->mRenderPass;
		renderPassInfo.framebuffer = this->mFramebuffer;//VulkanRenderer::GetRenderer()->mFinalSceneFramebuffer;
		renderPassInfo.renderArea.extent.width = this->mResolution.x;
		renderPassInfo.renderArea.extent.height = this->mResolution.y;

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = this->mResolution.x;
		viewport.height = -this->mResolution.y;
		viewport.y = this->mResolution.y;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->mRenderPickingTexturePostEffects->mShaders->mPipeline);

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent.width = this->mResolution.x;
		scissor.extent.height = this->mResolution.y;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &VulkanRenderer::GetRenderer()->mMainVertexBuffer->GetBuffer(), offsets);
		vkCmdBindVertexBuffers(commandBuffer, 1, 1, &VulkanRenderer::GetRenderer()->mMainInstanceMatrixBuffers[VulkanRenderer::GetRenderer()->mCurrentFrame], offsets);
		//vkCmdBindVertexBuffers(commandBuffer, 2, 1, &VulkanRenderer::GetRenderer()->mMainInstanceMaterialBuffers[VulkanRenderer::GetRenderer()->mCurrentFrame], offsets);
		vkCmdBindIndexBuffer(commandBuffer, VulkanRenderer::GetRenderer()->mMainIndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

		//vkCmdDrawIndexedIndirect(commandBuffer, VulkanRenderer::GetRenderer()->mIndirectBuffer, 0, VulkanRenderer::GetRenderer()->mIndirectDrawCount, sizeof(VkDrawIndexedIndirectCommand));

		for (const uint64_t& uuid : SceneView<MeshRenderer>(scene)) {
			this->DrawMeshToPickingTexture(*scene->GetComponent<MeshRenderer>(uuid), commandBuffer);
		}

		vkCmdEndRenderPass(commandBuffer);
		VulkanRenderer::GetRenderer()->EndSingleTimeCommands(commandBuffer);
	}

	void VulkanPicking::DrawOutline() {

	}

	std::pair<uint32_t, uint32_t> reverseHash(float normalizedValue) {
		// Denormalize the value to get the original hash value
		uint32_t hash_value = static_cast<uint32_t>(normalizedValue * 0xFFFFFF);

		// Reverse the hash function: Undo the prime number multiplication and XOR
		uint32_t combined = hash_value / 2654435761u;
		uint32_t x_low = combined & 0xFFFFFFFF;
		uint32_t x_high = combined ^ x_low;

		return std::make_pair(x_low, x_high);
	}

	uint64_t VulkanPicking::ReadPickingTexture(glm::vec2 pos) {
		if (pos.x > this->mResolution.x || pos.x < 0 || pos.y > this->mResolution.y || pos.y < 0)
			return 0;
		// Create a staging buffer
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		VulkanRenderer::GetRenderer()->CreateBuffer(this->mResolution.x * this->mResolution.y * (sizeof(uint32_t) * 4), VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		// Copy image to staging buffer
		VkCommandBuffer commandBuffer = VulkanRenderer::GetRenderer()->BeginSingleTimeCommands();
		VkImageSubresourceLayers subResource = {};
		subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subResource.mipLevel = 0;
		subResource.baseArrayLayer = 0;
		subResource.layerCount = 1;

		VkBufferImageCopy region = {};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource = subResource;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { (unsigned int)this->mResolution.x, (unsigned int)this->mResolution.y, 1 };

		vkCmdCopyImageToBuffer(commandBuffer, this->mPickingTextureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingBuffer, 1, &region);
		VulkanRenderer::GetRenderer()->EndSingleTimeCommands(commandBuffer);

		// Map staging buffer memory
		void* data;
		vkMapMemory(VulkanRenderer::GetRenderer()->mDevice, stagingBufferMemory, 0, this->mResolution.x * this->mResolution.y * (sizeof(uint32_t) * 4), 0, &data);

		// Read pixel data
		// Assuming RGBA8 format
		uint32_t* pixelData = static_cast<uint32_t*>(data);
		int desiredPixelX = pos.x;
		int desiredPixelY = this->mResolution.y - pos.y;
		int byteOffset = (desiredPixelY * this->mResolution.x + desiredPixelX) * 4; // RGBA8 format, 4 bytes per pixel
		uint32_t red = pixelData[byteOffset];
		uint32_t green = pixelData[byteOffset + 1];
		uint32_t blue = pixelData[byteOffset + 2];
		uint32_t alpha = pixelData[byteOffset + 3];

		// Unmap staging buffer memory
		vkUnmapMemory(VulkanRenderer::GetRenderer()->mDevice, stagingBufferMemory);

		// Clean up
		vkDestroyBuffer(VulkanRenderer::GetRenderer()->mDevice, stagingBuffer, nullptr);
		vkFreeMemory(VulkanRenderer::GetRenderer()->mDevice, stagingBufferMemory, nullptr);

		std::pair<uint32_t, uint32_t> pair = reverseHash(static_cast<unsigned int>(red));
		uint32_t uuid1 = red;
		uint32_t uuid2 = green;

		return ((uint64_t)uuid1 << 32) | uuid2;
	}

	uint64_t VulkanPicking::DrawAndRead(Scene* scene, glm::vec2 pos) {
		DrawSelectedObjectsUuid(scene);
		return this->ReadPickingTexture(pos);
	}

	void VulkanPicking::UpdateAndPushConstants(VkCommandBuffer commandBuffer) {

	}

	void VulkanPicking::Terminate() {

	}

}