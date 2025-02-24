#include <Engine/Core/PreCompiledHeaders.h>
#include "VulkanLighting.h"
#include "VulkanPlazaPipeline.h"
#include "Editor/DefaultAssets/Models/DefaultModels.h"
#include "VulkanShaders.h"
#include "VulkanPlazaInitializator.h"
#include "Renderer.h"
#include "Engine/Core/Scene.h"
#include "VulkanTexture.h"

namespace Plaza {
	void VulkanLighting::InitializeBuffers() {
		this->mLightsBuffer.resize(Application::Get()->mRenderer->mMaxFramesInFlight);
		this->mLightsBufferMemory.resize(Application::Get()->mRenderer->mMaxFramesInFlight);
		this->mTilesBuffer.resize(Application::Get()->mRenderer->mMaxFramesInFlight);
		this->mTilesBufferMemory.resize(Application::Get()->mRenderer->mMaxFramesInFlight);
		this->mDepthValuesBuffer.resize(Application::Get()->mRenderer->mMaxFramesInFlight);
		this->mDepthValuesBufferMemory.resize(Application::Get()->mRenderer->mMaxFramesInFlight);

		for (int i = 0; i < Application::Get()->mRenderer->mMaxFramesInFlight; ++i) {
			VulkanRenderer::GetRenderer()->CreateBuffer(
				sizeof(LightStruct) * 1024 * 1024,
				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
				this->mLightsBuffer[i],
				this->mLightsBufferMemory[i]);

			VulkanRenderer::GetRenderer()->CreateBuffer(sizeof(Tile) * 1024 * 1024,
				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
				this->mTilesBuffer[i],
				this->mTilesBufferMemory[i]);


			VulkanRenderer::GetRenderer()->CreateBuffer(sizeof(glm::vec2) * 1024,
				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
				this->mDepthValuesBuffer[i],
				this->mDepthValuesBufferMemory[i]);
		}
	}

	void VulkanLighting::InitializeDescriptorSets() {
		/* Descriptor Sets Layout */
		std::array<VkDescriptorSetLayoutBinding, 4> layoutBindings{};
		layoutBindings[0].binding = 0;
		layoutBindings[0].descriptorCount = 1;
		layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		layoutBindings[0].pImmutableSamplers = nullptr;
		layoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		layoutBindings[0].stageFlags |= VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;

		layoutBindings[1].binding = 1;
		layoutBindings[1].descriptorCount = 1;
		layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		layoutBindings[1].pImmutableSamplers = nullptr;
		layoutBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		layoutBindings[1].stageFlags |= VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;

		layoutBindings[2].binding = 2;
		layoutBindings[2].descriptorCount = 1;
		layoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		layoutBindings[2].pImmutableSamplers = nullptr;
		layoutBindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		layoutBindings[2].stageFlags |= VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;

		layoutBindings[3].binding = 3;
		layoutBindings[3].descriptorCount = 1;
		layoutBindings[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layoutBindings[3].pImmutableSamplers = nullptr;
		layoutBindings[3].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		layoutBindings[3].stageFlags |= VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = layoutBindings.size();
		layoutInfo.pBindings = layoutBindings.data();

		if (vkCreateDescriptorSetLayout(VulkanRenderer::GetRenderer()->mDevice, &layoutInfo, nullptr, &this->mLightSorterComputeShaders.mComputeDescriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create compute descriptor set layout!");
		}

		/* Descriptor sets */
		std::vector<VkDescriptorSetLayout> layouts(Application::Get()->mRenderer->mMaxFramesInFlight, this->mLightSorterComputeShaders.mComputeDescriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = VulkanRenderer::GetRenderer()->mDescriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(Application::Get()->mRenderer->mMaxFramesInFlight);
		allocInfo.pSetLayouts = layouts.data();

		this->mLightSorterComputeShaders.mComputeDescriptorSets.resize(Application::Get()->mRenderer->mMaxFramesInFlight);
		if (vkAllocateDescriptorSets(VulkanRenderer::GetRenderer()->mDevice, &allocInfo, this->mLightSorterComputeShaders.mComputeDescriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (int i = 0; i < Application::Get()->mRenderer->mMaxFramesInFlight; ++i) {
			VkDescriptorBufferInfo lightsBufferInfo{};
			lightsBufferInfo.buffer = this->mLightsBuffer[i];
			lightsBufferInfo.range = VK_WHOLE_SIZE;
			this->mLightSorterComputeShaders.mDescriptorWrites.resize(4);
			this->mLightSorterComputeShaders.mDescriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			this->mLightSorterComputeShaders.mDescriptorWrites[0].dstSet = this->mLightSorterComputeShaders.mComputeDescriptorSets[i];
			this->mLightSorterComputeShaders.mDescriptorWrites[0].dstBinding = 0;
			this->mLightSorterComputeShaders.mDescriptorWrites[0].dstArrayElement = 0;
			this->mLightSorterComputeShaders.mDescriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			this->mLightSorterComputeShaders.mDescriptorWrites[0].descriptorCount = 1;
			this->mLightSorterComputeShaders.mDescriptorWrites[0].pBufferInfo = &lightsBufferInfo;

			VkDescriptorBufferInfo tilesBufferInfo{};
			tilesBufferInfo.buffer = this->mTilesBuffer[i];
			tilesBufferInfo.range = VK_WHOLE_SIZE;
			this->mLightSorterComputeShaders.mDescriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			this->mLightSorterComputeShaders.mDescriptorWrites[1].dstSet = this->mLightSorterComputeShaders.mComputeDescriptorSets[i];
			this->mLightSorterComputeShaders.mDescriptorWrites[1].dstBinding = 1;
			this->mLightSorterComputeShaders.mDescriptorWrites[1].dstArrayElement = 0;
			this->mLightSorterComputeShaders.mDescriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			this->mLightSorterComputeShaders.mDescriptorWrites[1].descriptorCount = 1;
			this->mLightSorterComputeShaders.mDescriptorWrites[1].pBufferInfo = &tilesBufferInfo;

			VkDescriptorBufferInfo depthValuesBufferInfo{};
			depthValuesBufferInfo.buffer = this->mDepthValuesBuffer[i];
			depthValuesBufferInfo.range = VK_WHOLE_SIZE;
			this->mLightSorterComputeShaders.mDescriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			this->mLightSorterComputeShaders.mDescriptorWrites[2].dstSet = this->mLightSorterComputeShaders.mComputeDescriptorSets[i];
			this->mLightSorterComputeShaders.mDescriptorWrites[2].dstBinding = 2;
			this->mLightSorterComputeShaders.mDescriptorWrites[2].dstArrayElement = 0;
			this->mLightSorterComputeShaders.mDescriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			this->mLightSorterComputeShaders.mDescriptorWrites[2].descriptorCount = 1;
			this->mLightSorterComputeShaders.mDescriptorWrites[2].pBufferInfo = &depthValuesBufferInfo;

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			imageInfo.imageView = VulkanRenderer::GetRenderer()->mDepthImageView;
			imageInfo.sampler = VulkanRenderer::GetRenderer()->mImGuiTextureSampler;
			this->mLightSorterComputeShaders.mDescriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			this->mLightSorterComputeShaders.mDescriptorWrites[3].dstSet = this->mLightSorterComputeShaders.mComputeDescriptorSets[i];
			this->mLightSorterComputeShaders.mDescriptorWrites[3].dstBinding = 3;
			this->mLightSorterComputeShaders.mDescriptorWrites[3].dstArrayElement = 0;
			this->mLightSorterComputeShaders.mDescriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			this->mLightSorterComputeShaders.mDescriptorWrites[3].descriptorCount = 1;
			this->mLightSorterComputeShaders.mDescriptorWrites[3].pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(VulkanRenderer::GetRenderer()->mDevice, this->mLightSorterComputeShaders.mDescriptorWrites.size(), this->mLightSorterComputeShaders.mDescriptorWrites.data(), 0, nullptr);
		}
	}

	void VulkanLighting::Init() {
		this->mScreenSize = Application::Get()->appSizes->sceneSize;

		std::cerr << "Initializing Tiled Deferred Shading \n";
		/* Initialize Light Sorter Compute Shaders */
		this->InitializeBuffers();
		this->InitializeDescriptorSets();

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.size = sizeof(LightSorterPushConstants);
		pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		this->mLightSorterComputeShaders.Init(Application::Get()->enginePath + "/Shaders/Vulkan/lighting/lightSorter.comp", { pushConstantRange });

		/* Initialize Deferred Pass */
		VkFormat form = VK_FORMAT_R32G32B32A32_SFLOAT;
		mDeferredEndTexture.CreateTextureImage(VulkanRenderer::GetRenderer()->mDevice, form, this->mScreenSize.x, this->mScreenSize.y, false, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED, 1, 0, false);
		VulkanRenderer::GetRenderer()->TransitionImageLayout(VulkanRenderer::GetRenderer()->mLighting->mDeferredEndTexture.mImage, VulkanRenderer::GetRenderer()->mFinalDeferredFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		mDeferredEndTexture.CreateTextureSampler();
		mDeferredEndTexture.CreateImageView(form, VK_IMAGE_ASPECT_COLOR_BIT);
		//mDeferredEndTexture.InitDescriptorSetLayout();

		VulkanRenderer::GetRenderer()->AddTrackerToImage(mDeferredEndTexture.mImage, "Deferred End Texture", VulkanRenderer::GetRenderer()->mImGuiTextureSampler, mDeferredEndTexture.GetTextureInfo(), mDeferredEndTexture.GetLayout());

		VkDescriptorSetLayoutBinding positionLayoutBinding = plvk::descriptorSetLayoutBinding(0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nullptr, VK_SHADER_STAGE_FRAGMENT_BIT);
		VkDescriptorSetLayoutBinding normalLayoutBinding = plvk::descriptorSetLayoutBinding(1, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nullptr, VK_SHADER_STAGE_FRAGMENT_BIT);
		VkDescriptorSetLayoutBinding diffuseLayoutBinding = plvk::descriptorSetLayoutBinding(2, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nullptr, VK_SHADER_STAGE_FRAGMENT_BIT);
		VkDescriptorSetLayoutBinding othersLayoutBinding = plvk::descriptorSetLayoutBinding(3, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nullptr, VK_SHADER_STAGE_FRAGMENT_BIT);

		VkDescriptorSetLayoutBinding lightsLayoutBinding = plvk::descriptorSetLayoutBinding(4, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, VK_SHADER_STAGE_FRAGMENT_BIT);
		VkDescriptorSetLayoutBinding tilesLayoutBinding = plvk::descriptorSetLayoutBinding(5, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, VK_SHADER_STAGE_FRAGMENT_BIT);

		std::vector<VkDescriptorSetLayoutBinding> bindings = { positionLayoutBinding, normalLayoutBinding, diffuseLayoutBinding, othersLayoutBinding, lightsLayoutBinding, tilesLayoutBinding };
		VkDescriptorSetLayoutCreateInfo layoutInfo = plvk::descriptorSetLayoutCreateInfo(bindings, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT);

		if (vkCreateDescriptorSetLayout(VulkanRenderer::GetRenderer()->mDevice, &layoutInfo, nullptr, &this->mDeferredEndPassRenderer.mShaders->mDescriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}

		VkPipelineLayoutCreateInfo mSwapchainPipelineLayoutInfo{};
		mSwapchainPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		mSwapchainPipelineLayoutInfo.setLayoutCount = 1;
		mSwapchainPipelineLayoutInfo.pSetLayouts = &this->mDeferredEndPassRenderer.mShaders->mDescriptorSetLayout;

		VkPushConstantRange deferredPassPushConstantRange = {};
		deferredPassPushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		deferredPassPushConstantRange.offset = 0;
		deferredPassPushConstantRange.size = sizeof(DeferredPassPushConstants);
		mSwapchainPipelineLayoutInfo.pushConstantRangeCount = 1;
		mSwapchainPipelineLayoutInfo.pPushConstantRanges = &deferredPassPushConstantRange;

		/* Render pass */
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = this->mDeferredEndTextureFormat;
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

		if (vkCreateRenderPass(VulkanRenderer::GetRenderer()->mDevice, &renderPassInfo, nullptr, &this->mDeferredEndPassRenderer.mRenderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}

		std::vector<VkImageView> imageViews = { this->mDeferredEndTexture.mImageView };
		this->mDeferredEndPassRenderer.InitializeFramebuffer(imageViews.data(), imageViews.size(), this->mScreenSize, 1);

		mDeferredEndPassRenderer.Init(
			VulkanShadersCompiler::Compile(Application::Get()->enginePath + "/Shaders/Vulkan/lighting/deferredPass.vert"),
			VulkanShadersCompiler::Compile(Application::Get()->enginePath + "/Shaders/Vulkan/lighting/deferredPass.frag"),
			"",
			VulkanRenderer::GetRenderer()->mDevice,
			this->mScreenSize,
			this->mDeferredEndPassRenderer.mShaders->mDescriptorSetLayout,
			mSwapchainPipelineLayoutInfo
		);

		// Descriptor Set
		VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		allocInfo.descriptorPool = VulkanRenderer::GetRenderer()->mDescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &this->mDeferredEndPassRenderer.mShaders->mDescriptorSetLayout;

		this->mDeferredEndPassRenderer.mShaders->mDescriptorSets.resize(Application::Get()->mRenderer->mMaxFramesInFlight);
		for (unsigned int i = 0; i < Application::Get()->mRenderer->mMaxFramesInFlight; ++i) {
			if (vkAllocateDescriptorSets(VulkanRenderer::GetRenderer()->mDevice, &allocInfo, &this->mDeferredEndPassRenderer.mShaders->mDescriptorSets[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate descriptor sets!");
			}

			VkDescriptorImageInfo geometryPositionInfo = plvk::descriptorImageInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VulkanRenderer::GetRenderer()->mDeferredPositionTexture.mImageView, VulkanRenderer::GetRenderer()->mImGuiTextureSampler);
			VkDescriptorImageInfo geometryNormalInfo = plvk::descriptorImageInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VulkanRenderer::GetRenderer()->mDeferredNormalTexture.mImageView, VulkanRenderer::GetRenderer()->mImGuiTextureSampler);
			VkDescriptorImageInfo geometryDiffuseInfo = plvk::descriptorImageInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VulkanRenderer::GetRenderer()->mDeferredDiffuseTexture.mImageView, VulkanRenderer::GetRenderer()->mImGuiTextureSampler);
			VkDescriptorImageInfo geometryOthersInfo = plvk::descriptorImageInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VulkanRenderer::GetRenderer()->mDeferredOthersTexture.mImageView, VulkanRenderer::GetRenderer()->mImGuiTextureSampler);

			VkDescriptorBufferInfo lightsBufferInfo = plvk::descriptorBufferInfo(this->mLightsBuffer[i], 0, 1024 * 1024 * sizeof(LightStruct));
			VkDescriptorBufferInfo tilesBufferInfo = plvk::descriptorBufferInfo(this->mTilesBuffer[i], 0, 1024 * 1024 * sizeof(Tile));
			//std::vector<VkWriteDescriptorSet> descriptorWrites{ plvk::writeDescriptorSet(this->mDeferredEndPassRenderer.mShaders->mDescriptorSets[i], 0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &geometryPositionInfo) };

			//VkDescriptorBufferInfo bufferInfo = plvk::descriptorBufferInfo(this->mUniformBuffers[i], 0, sizeof(UniformBufferObject));
			//VkDescriptorImageInfo imageInfo = plvk::descriptorImageInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, this->mShadows->mShadowDepthImageViews[i], this->mShadows->mShadowsSampler);

			std::vector<VkWriteDescriptorSet> descriptorWrites = std::vector<VkWriteDescriptorSet>();
			descriptorWrites.push_back(plvk::writeDescriptorSet(this->mDeferredEndPassRenderer.mShaders->mDescriptorSets[i], 0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &geometryPositionInfo));
			descriptorWrites.push_back(plvk::writeDescriptorSet(this->mDeferredEndPassRenderer.mShaders->mDescriptorSets[i], 1, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &geometryNormalInfo));
			descriptorWrites.push_back(plvk::writeDescriptorSet(this->mDeferredEndPassRenderer.mShaders->mDescriptorSets[i], 2, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &geometryDiffuseInfo));
			descriptorWrites.push_back(plvk::writeDescriptorSet(this->mDeferredEndPassRenderer.mShaders->mDescriptorSets[i], 3, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &geometryOthersInfo));
			descriptorWrites.push_back(plvk::writeDescriptorSet(this->mDeferredEndPassRenderer.mShaders->mDescriptorSets[i], 4, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, nullptr, &lightsBufferInfo));
			descriptorWrites.push_back(plvk::writeDescriptorSet(this->mDeferredEndPassRenderer.mShaders->mDescriptorSets[i], 5, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, nullptr, &tilesBufferInfo));

			vkUpdateDescriptorSets(VulkanRenderer::GetRenderer()->mDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}

	void VulkanLighting::GetLights(Scene* scene) {
		this->mLights.clear();
		for (const uint64_t& uuid : SceneView<Light>(scene)) {
			Light& component = *scene->GetComponent<Light>(uuid);
			const glm::vec3& position = scene->GetComponent<TransformComponent>(uuid)->GetWorldPosition();
			this->mLights.push_back(LightStruct{ component.color, component.radius, position, component.intensity, component.cutoff });
		}

		void* data;
		size_t bufferSize = sizeof(LightStruct) * this->mLights.size();
		vkMapMemory(VulkanRenderer::GetRenderer()->mDevice, this->mLightsBufferMemory[Application::Get()->mRenderer->mCurrentFrame], 0, VK_WHOLE_SIZE, 0, &data);
		memcpy(data, this->mLights.data(), (size_t)bufferSize);
		vkUnmapMemory(VulkanRenderer::GetRenderer()->mDevice, this->mLightsBufferMemory[Application::Get()->mRenderer->mCurrentFrame]);
	}

	void VulkanLighting::UpdateTiles() {
		this->mScreenSize = Application::Get()->appSizes->sceneSize;

		glm::vec2 clusterSize = glm::vec2(32.0f);
		glm::vec2 clusterCount = glm::ceil(this->mScreenSize / clusterSize);

		LightSorterPushConstants lightSorterPushConstants{};
		lightSorterPushConstants.clusterSize = glm::vec2(32);
		lightSorterPushConstants.first = true;
		lightSorterPushConstants.lightCount = this->mLights.size();
		lightSorterPushConstants.projection = Application::Get()->activeCamera->GetProjectionMatrix();
		lightSorterPushConstants.view = Application::Get()->activeCamera->GetViewMatrix();
		lightSorterPushConstants.screenSize = this->mScreenSize;
		vkCmdPushConstants(*VulkanRenderer::GetRenderer()->mActiveCommandBuffer, this->mLightSorterComputeShaders.mComputePipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, { 0 }, sizeof(LightSorterPushConstants), &lightSorterPushConstants);
		this->mLightSorterComputeShaders.Dispatch(clusterCount.x, clusterCount.y, 1);


	}

	void VulkanLighting::DrawDeferredPass() {
		mDeferredEndPassRenderer.UpdateCommandBuffer(*VulkanRenderer::GetRenderer()->mActiveCommandBuffer);

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		//renderPassInfo.renderPass = this->mSkyboxPostEffect->mRenderPass;
		//renderPassInfo.framebuffer = this->mFramebuffers[Application::Get()->mRenderer->mCurrentFrame];//mSwapChainFramebuffers[0];//mSwapChainFramebuffers[imageIndex];
		renderPassInfo.renderPass = this->mDeferredEndPassRenderer.mRenderPass;
		renderPassInfo.framebuffer = this->mDeferredEndPassRenderer.mFramebuffer;

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = VulkanRenderer::GetRenderer()->mSwapChainExtent;

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { {0.0f, 0.0f, 0.0f, 0.0f} };
		clearValues[1].depthStencil = { 0.0f, 0 };

		renderPassInfo.clearValueCount = 2;
		renderPassInfo.pClearValues = clearValues.data();

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		viewport.width = static_cast<float>(VulkanRenderer::GetRenderer()->mSwapChainExtent.width);
		viewport.height = static_cast<float>(VulkanRenderer::GetRenderer()->mSwapChainExtent.height);
		//viewport.y = this->mResolution.y;

		renderPassInfo.renderArea.extent.width = Application::Get()->appSizes->sceneSize.x;
		renderPassInfo.renderArea.extent.height = Application::Get()->appSizes->sceneSize.y;

		viewport.width = Application::Get()->appSizes->sceneSize.x;
		viewport.height = -Application::Get()->appSizes->sceneSize.y;
		viewport.y = Application::Get()->appSizes->sceneSize.y;
		vkCmdSetViewport(*VulkanRenderer::GetRenderer()->mActiveCommandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = VulkanRenderer::GetRenderer()->mSwapChainExtent;
		scissor.extent.width = Application::Get()->appSizes->sceneSize.x;
		scissor.extent.height = Application::Get()->appSizes->sceneSize.y;
		vkCmdSetScissor(*VulkanRenderer::GetRenderer()->mActiveCommandBuffer, 0, 1, &scissor);

		vkCmdBeginRenderPass(*VulkanRenderer::GetRenderer()->mActiveCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		//vkCmdBeginRenderPass(this->mCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(*VulkanRenderer::GetRenderer()->mActiveCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->mDeferredEndPassRenderer.mShaders->mPipeline);
		//vkCmdBindDescriptorSets(*VulkanRenderer::GetRenderer()->mActiveCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, &this->mDeferredPassRenderer.mShaders->mDescriptorSetLayout, 0, 1, &this->mDeferredPassRenderer.mShaders->mDescriptorSet, 0, 0);
		DeferredPassPushConstants deferredPassPushConstants{};
		deferredPassPushConstants.lightCount = this->mLights.size();
		deferredPassPushConstants.projection = Application::Get()->activeCamera->GetProjectionMatrix();
		deferredPassPushConstants.view = Application::Get()->activeCamera->GetViewMatrix();
		deferredPassPushConstants.viewPos = Application::Get()->activeCamera->Position;
		deferredPassPushConstants.ambientLightColor = this->ambientLightColor * this->ambientLightIntensity;
		vkCmdPushConstants(*VulkanRenderer::GetRenderer()->mActiveCommandBuffer, this->mDeferredEndPassRenderer.mShaders->mPipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(DeferredPassPushConstants), &deferredPassPushConstants);
		mDeferredEndPassRenderer.DrawFullScreenRectangle();

		vkCmdEndRenderPass(*VulkanRenderer::GetRenderer()->mActiveCommandBuffer);
	}

	void VulkanLighting::Terminate() {

	}
}