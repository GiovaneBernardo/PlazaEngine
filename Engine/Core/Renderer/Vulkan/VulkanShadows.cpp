#include "Engine/Core/PreCompiledHeaders.h"
#include "VulkanShadows.h"

namespace Plaza {
	void VulkanShadows::CreateDescriptorPool(VkDevice device) {
		std::array<VkDescriptorPoolSize, 1> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = 1;

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = 32;
		poolInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

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

		std::array<VkDescriptorSetLayoutBinding, 1> bindings = { uboLayoutBinding };
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();
		layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;

		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &this->mDescriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	void VulkanShadows::CreateDescriptorSet(VkDevice device) {
		VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		allocInfo.descriptorPool = this->mDescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &mDescriptorSetLayout;

		if (vkAllocateDescriptorSets(device, &allocInfo, &this->mDescriptorSet) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = mUniformBuffer;
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(ShadowsUniformBuffer);

		std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = this->mDescriptorSet;
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

		for (uint32_t i = 0; i < mCascades.size(); i++) {
			vkAllocateDescriptorSets(device, &allocInfo, &this->mCascades[i].mDescriptorSet);
			VkDescriptorImageInfo cascadeImageInfo{};
			cascadeImageInfo.sampler = this->mShadowsSampler;
			cascadeImageInfo.imageView = this->mShadowDepthImageViews[0];
			cascadeImageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

			std::array<VkWriteDescriptorSet, 1> writeDescriptorSets{};
			writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSets[0].dstSet = this->mCascades[i].mDescriptorSet;
			writeDescriptorSets[0].dstBinding = 0;
			writeDescriptorSets[0].dstArrayElement = 0;
			writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeDescriptorSets[0].descriptorCount = 1;
			writeDescriptorSets[0].pBufferInfo = &bufferInfo;
			vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
		}
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
		//imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateImage(renderer.mDevice, &imageInfo, nullptr, &this->mShadowDepthImage) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create depth image!");
		}

		// Allocate memory and bind image
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(renderer.mDevice, this->mShadowDepthImage, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = renderer.FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkDeviceMemory shadowsDepthMapMemory;
		if (vkAllocateMemory(renderer.mDevice, &allocInfo, nullptr, &shadowsDepthMapMemory) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate depth image memory!");
		}

		vkBindImageMemory(renderer.mDevice, this->mShadowDepthImage, shadowsDepthMapMemory, 0);

		// Transition image layout 
		//renderer.TransitionImageLayout(this->mShadowDepthImage, mDepthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT, 9);
		//renderer.TransitionImageLayout(this->mShadowDepthImage, mDepthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_DEPTH_BIT, 9);

		mShadowDepthImageViews.resize(1);
		for (unsigned int i = 0; i < 1; ++i) {
			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = this->mShadowDepthImage;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			viewInfo.format = mDepthFormat;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 9;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			if (vkCreateImageView(VulkanRenderer::GetRenderer()->mDevice, &viewInfo, nullptr, &this->mShadowDepthImageViews[i]) != VK_SUCCESS) {
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
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = i;
			viewInfo.subresourceRange.layerCount = 1;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			if (vkCreateImageView(renderer.mDevice, &viewInfo, nullptr, &this->mCascades[i].mImageView) != VK_SUCCESS) {
				throw std::runtime_error("failed to create texture image view!");
			}

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = this->mRenderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = &this->mCascades[i].mImageView;
			framebufferInfo.width = this->mShadowResolution;
			framebufferInfo.height = this->mShadowResolution;
			framebufferInfo.layers = 1;
			if(vkCreateFramebuffer(renderer.mDevice, &framebufferInfo, nullptr, &this->mCascades[i].mFramebuffer) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create framebuffer!");
			}
		}

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = this->mRenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &this->mShadowDepthImageViews[0];
		framebufferInfo.width = this->mShadowResolution;
		framebufferInfo.height = this->mShadowResolution;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(renderer.mDevice, &framebufferInfo, nullptr, &this->mFramebuffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create framebuffer!");
		}
	}


	void VulkanShadows::InitializeRenderPass(VulkanRenderer& renderer) {
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.maxAnisotropy = 1.0;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		vkCreateSampler(renderer.mDevice, &samplerInfo, nullptr, &this->mShadowsSampler);

		VkFormat depthFormat = mDepthFormat;

		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = VK_FORMAT_R8G8B8A8_SRGB;
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
		depthAttachment.format = depthFormat;
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 0;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 0;
		//subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		std::array<VkSubpassDependency, 2> dependencies;

		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		std::array<VkAttachmentDescription, 1> attachments = { depthAttachment };
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = dependencies.size();
		renderPassInfo.pDependencies = dependencies.data();

		if (vkCreateRenderPass(renderer.mDevice, &renderPassInfo, nullptr, &this->mRenderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}

	void VulkanShadows::Init() {
		int bufferSize = sizeof(ShadowsUniformBuffer);
		VulkanRenderer::GetRenderer()->CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mUniformBuffer, mUniformBufferMemory);
		vkMapMemory(VulkanRenderer::GetRenderer()->mDevice, mUniformBufferMemory, 0, bufferSize, 0, &mUniformBufferMapped);

		this->InitializeRenderPass(*VulkanRenderer::GetRenderer());
		this->InitializeBuffers(*VulkanRenderer::GetRenderer());

		this->CreateDescriptorPool(VulkanRenderer::GetRenderer()->mDevice);
		this->CreateDescriptorSetLayout(VulkanRenderer::GetRenderer()->mDevice);
		this->CreateDescriptorSet(VulkanRenderer::GetRenderer()->mDevice);

		mShadowsShader = new VulkanShaders(VulkanShadersCompiler::Compile(Application->enginePath + "\\Shaders\\shadows\\cascadedShadowDepthShaders.vert"), VulkanShadersCompiler::Compile(Application->enginePath + "\\Shaders\\shadows\\cascadedShadowDepthShaders.frag"), VulkanShadersCompiler::Compile(Application->enginePath + "\\Shaders\\shadows\\cascadedShadowDepthShaders.geom"));
		
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_ALL;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(VulkanShadows::PushConstants);
		
		mShadowsShader->Init(VulkanRenderer::GetRenderer()->mDevice, this->mRenderPass, Application->appSizes->sceneSize.x, Application->appSizes->sceneSize.y, this->mDescriptorSetLayout, std::vector<VkPushConstantRange> { pushConstantRange });

		float mult = 1.0f;
		shadowCascadeLevels = vector{ Application->activeCamera->farPlane / (9000.0f * mult), Application->activeCamera->farPlane / (3000.0f * mult), Application->activeCamera->farPlane / (1000.0f * mult), Application->activeCamera->farPlane / (500.0f * mult), Application->activeCamera->farPlane / (100.0f * mult), Application->activeCamera->farPlane / (35.0f * mult),Application->activeCamera->farPlane / (10.0f * mult), Application->activeCamera->farPlane / (2.0f * mult), Application->activeCamera->farPlane / (1.0f * mult) };
	}

	void VulkanShadows::RenderToShadowMap() {
		UpdateUniformBuffer();
	}

	void VulkanShadows::Terminate() {

	}

	using std::min;
	glm::mat4 GetLightSpaceMatrix(const float nearPlane, const float farPlane)
	{
		const auto proj = glm::perspective(
			glm::radians(Application->activeCamera->Zoom), (float)Application->appSizes->sceneSize.x / (float)Application->appSizes->sceneSize.y, nearPlane,
			farPlane);
		const auto corners = Application->activeCamera->getFrustumCornersWorldSpace(proj, Application->activeCamera->GetViewMatrix());

		glm::vec3 center = glm::vec3(0, 0, 0);
		for (const auto& v : corners)
		{
			center += glm::vec3(v);
		}
		center /= corners.size();
		const float LARGE_CONSTANT = std::abs(std::numeric_limits<float>::min());
		glm::vec3 lightDir = glm::normalize(glm::vec3(20.0f, 50, 20.0f));
		const auto lightView = glm::lookAt(center + glm::radians(lightDir), center, glm::vec3(0.0f, 1.0f, 0.0f));
		float minX = std::numeric_limits<float>::max();
		float maxX = std::numeric_limits<float>::lowest();
		float minY = std::numeric_limits<float>::max();
		float maxY = std::numeric_limits<float>::lowest();
		float minZ = std::numeric_limits<float>::max();
		float maxZ = std::numeric_limits<float>::lowest();
		for (const auto& v : corners)
		{
			const auto trf = lightView * v;
			minX = std::min(minX, trf.x);
			maxX = std::max(maxX, trf.x);
			minY = std::min(minY, trf.y);
			maxY = std::max(maxY, trf.y);
			minZ = std::min(minZ, trf.z);
			maxZ = std::max(maxZ, trf.z);
		}

		// Tune this parameter according to the scene

		constexpr float zMult = 22.0f;
		if (minZ < 0)
		{
			minZ *= zMult;
		}
		else
		{
			minZ /= zMult;
		}
		if (maxZ < 0)
		{
			maxZ /= zMult;
		}
		else
		{
			maxZ *= zMult;
		}
		const glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);

		return lightProjection * lightView;
	}
	void GetLightSpaceMatrices(std::vector<float>shadowCascadeLevels, VulkanShadows::ShadowsUniformBuffer& ubo)
	{
		for (size_t i = 0; i < shadowCascadeLevels.size() + 1; ++i) {
			if (i == 0) {
				ubo.lightSpaceMatrices[i] = GetLightSpaceMatrix(Application->editorCamera->nearPlane - 1.0f, shadowCascadeLevels[i]);
			}
			else if (i < shadowCascadeLevels.size()) {
				ubo.lightSpaceMatrices[i] = GetLightSpaceMatrix(shadowCascadeLevels[i - 1] - 1.0f, shadowCascadeLevels[i]);
			}
		}
	}

	void VulkanShadows::UpdateUniformBuffer() {
		ShadowsUniformBuffer ubo{};
		GetLightSpaceMatrices(this->shadowCascadeLevels, ubo);

		memcpy(mUniformBufferMapped, &ubo, sizeof(ubo));
	}

	void VulkanShadows::UpdateAndPushConstants(VkCommandBuffer commandBuffer, unsigned int cascadeIndex) {
		this->pushConstants.cascadeIndex = cascadeIndex;
		vkCmdPushConstants(commandBuffer, this->mShadowsShader->mPipelineLayout, VK_SHADER_STAGE_ALL, 0, sizeof(VulkanShadows::PushConstants), &this->pushConstants);
	}
}