#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include "ThirdParty/vulkan/vulkan/vulkan.h"
#include <vector>
#include "ThirdParty/glm/glm.hpp"

namespace Plaza {
	namespace plvk {
		static VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(uint32_t binding, uint32_t descriptorCount,
																	   VkDescriptorType descriptorType,
																	   VkSampler* pImmutableSamplers,
																	   VkShaderStageFlags shaderStageFlags) {
			VkDescriptorSetLayoutBinding bindingInfo{};
			bindingInfo.binding = binding;
			bindingInfo.descriptorCount = descriptorCount;
			bindingInfo.descriptorType = descriptorType;
			bindingInfo.pImmutableSamplers = pImmutableSamplers;
			bindingInfo.stageFlags = shaderStageFlags;
			return bindingInfo;
		}

		static VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(
			std::vector<VkDescriptorSetLayoutBinding>& bindings,
			VkDescriptorSetLayoutCreateFlags flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT,
			void* pNext = nullptr) {
			VkDescriptorSetLayoutCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
			createInfo.pBindings = bindings.data();
			createInfo.flags = flags;
			createInfo.pNext = pNext;
			return createInfo;
		}

		static VkWriteDescriptorSet writeDescriptorSet(
			VkDescriptorSet dstSet, uint32_t dstBinding = 0, uint32_t dstArrayElement = 0,
			VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, uint32_t descriptorCount = 1,
			VkDescriptorImageInfo* imageInfo = nullptr, VkDescriptorBufferInfo* bufferInfo = nullptr) {
			VkWriteDescriptorSet writeInfo{};
			writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeInfo.dstSet = dstSet;
			writeInfo.dstBinding = dstBinding;
			writeInfo.dstArrayElement = dstArrayElement;
			writeInfo.descriptorType = descriptorType;
			writeInfo.descriptorCount = descriptorCount;
			if (imageInfo)
				writeInfo.pImageInfo = imageInfo;
			if (bufferInfo)
				writeInfo.pBufferInfo = bufferInfo;
			return writeInfo;
		}

		static VkDescriptorImageInfo descriptorImageInfo(VkImageLayout layout, VkImageView imageView,
														 VkSampler sampler) {
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = layout;
			imageInfo.imageView = imageView;
			imageInfo.sampler = sampler;
			return imageInfo;
		}

		static VkDescriptorBufferInfo descriptorBufferInfo(VkBuffer& buffer, uint32_t offset, uint32_t range) {
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = buffer;
			bufferInfo.offset = offset;
			bufferInfo.range = range;
			return bufferInfo;
		}

		static VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(uint32_t setLayoutCount,
																   VkDescriptorSetLayout* pSetLayouts,
																   uint32_t pushConstantRangeCount,
																   VkPushConstantRange* pPushConstantRanges) {
			VkPipelineLayoutCreateInfo layoutInfo{};
			layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			layoutInfo.setLayoutCount = setLayoutCount;
			layoutInfo.pSetLayouts = pSetLayouts;
			layoutInfo.pushConstantRangeCount = pushConstantRangeCount;
			layoutInfo.pPushConstantRanges = pPushConstantRanges;
			layoutInfo.pNext = nullptr;
			layoutInfo.flags = 0;
			return layoutInfo;
		}

		static VkPushConstantRange pushConstantRange(VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size) {
			VkPushConstantRange pushConstantRange{};
			pushConstantRange.stageFlags = stageFlags;
			pushConstantRange.offset = offset;
			pushConstantRange.size = size;
			return pushConstantRange;
		}

		static VkAttachmentDescription attachmentDescription(VkFormat format, VkSampleCountFlagBits samples,
															 VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp,
															 VkAttachmentLoadOp stencilLoadOp,
															 VkAttachmentStoreOp stencilStoreOp,
															 VkImageLayout initialLayout, VkImageLayout finalLayout,
															 VkAttachmentDescriptionFlags flags = 0) {
			VkAttachmentDescription attachmentDescription{};
			attachmentDescription.format = format;
			attachmentDescription.samples = samples;
			attachmentDescription.loadOp = loadOp;
			attachmentDescription.storeOp = storeOp;
			attachmentDescription.stencilLoadOp = stencilLoadOp;
			attachmentDescription.stencilStoreOp = stencilStoreOp;
			attachmentDescription.initialLayout = initialLayout;
			attachmentDescription.finalLayout = finalLayout;
			attachmentDescription.flags = flags;
			return attachmentDescription;
		}

		static VkSubpassDependency subpassDependency(uint32_t srcSubpass, uint32_t dstSubpass,
													 VkPipelineStageFlags srcStageMask,
													 VkPipelineStageFlags dstStageMask, VkAccessFlags srcAccessMask,
													 VkAccessFlags dstAccessMask, VkDependencyFlags dependencyFlags) {
			VkSubpassDependency dependency{};
			dependency.srcSubpass = srcSubpass;
			dependency.dstSubpass = dstSubpass;
			dependency.srcStageMask = srcStageMask;
			dependency.dstStageMask = dstStageMask;
			dependency.srcAccessMask = srcAccessMask;
			dependency.dstAccessMask = dstAccessMask;
			dependency.dependencyFlags = dependencyFlags;
			return dependency;
		}

		static VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo(VkShaderStageFlagBits stage,
																			 VkShaderModule module,
																			 const char* pName = "main") {
			VkPipelineShaderStageCreateInfo shaderStageInfo{};
			shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStageInfo.stage = stage;
			shaderStageInfo.module = module;
			shaderStageInfo.pName = pName;
			return shaderStageInfo;
		}

		static VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo(
			std::vector<VkVertexInputBindingDescription>& bindings,
			std::vector<VkVertexInputAttributeDescription>& attributes) {
			VkPipelineVertexInputStateCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			createInfo.pVertexBindingDescriptions = bindings.data();
			createInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindings.size());
			createInfo.pVertexAttributeDescriptions = attributes.data();
			createInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
			return createInfo;
		}

		static VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(
			VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			VkBool32 primitiveRestartEnable = VK_FALSE) {
			VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
			inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssembly.topology = topology;
			inputAssembly.primitiveRestartEnable = primitiveRestartEnable;
			return inputAssembly;
		}

		static VkViewport viewport(glm::vec2 position, glm::vec2 size, float minDepth = 1.0f, float maxDepth = 1.0f) {
			VkViewport viewport{};
			viewport.x = position.x;
			viewport.y = position.y;
			viewport.width = (float)size.x;
			viewport.height = (float)size.y;
			viewport.minDepth = minDepth;
			viewport.maxDepth = maxDepth;
			return viewport;
		}

		static VkRect2D rect2D(glm::vec2 offset, glm::vec2 size) {
			VkRect2D scissor{};
			scissor.offset = {(int32_t)offset.x, (int32_t)offset.y};
			scissor.extent = {(uint32_t)size.x, (uint32_t)size.y};
			return scissor;
		}

		static VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(uint32_t viewportCount = 1,
																				 uint32_t scissorCount = 1) {
			VkPipelineViewportStateCreateInfo viewportState{};
			viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportState.viewportCount = viewportCount;
			viewportState.scissorCount = scissorCount;
			return viewportState;
		}

		static VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(
			VkBool32 depthClampEnable = VK_FALSE, VkBool32 rasterizerDiscardEnable = VK_FALSE,
			VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL, float lineWidth = 1.0f,
			VkBool32 depthBiasEnable = VK_FALSE, float depthBiasConstantFactor = 0.0f, float depthBiasClamp = 0.0f,
			float depthBiasSlopeFactor = 0.0f, VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT,
			VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE) {
			VkPipelineRasterizationStateCreateInfo rasterizer{};
			rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizer.depthClampEnable = depthClampEnable;
			rasterizer.rasterizerDiscardEnable = rasterizerDiscardEnable;
			rasterizer.polygonMode = polygonMode;
			rasterizer.lineWidth = lineWidth;
			rasterizer.depthBiasEnable = depthBiasEnable;
			rasterizer.depthBiasConstantFactor = depthBiasConstantFactor;
			rasterizer.depthBiasClamp = depthBiasClamp;
			rasterizer.depthBiasSlopeFactor = depthBiasSlopeFactor;
			rasterizer.cullMode = cullMode;
			rasterizer.frontFace = frontFace;
			return rasterizer;
		}

		static VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo(
			VkSampleCountFlagBits rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
			VkBool32 sampleShadingEnable = VK_FALSE) {
			VkPipelineMultisampleStateCreateInfo multisampling{};
			multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampling.sampleShadingEnable = sampleShadingEnable;
			multisampling.rasterizationSamples = rasterizationSamples;
			return multisampling;
		}

		static VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo(
			VkBool32 depthTestEnable = VK_TRUE, VkBool32 depthWriteEnable = VK_TRUE,
			VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS, VkBool32 depthBoundsTestEnable = VK_FALSE,
			VkBool32 stencilTestEnable = VK_FALSE) {
			VkPipelineDepthStencilStateCreateInfo depthStencil{};
			depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencil.depthTestEnable = depthTestEnable;
			depthStencil.depthWriteEnable = depthWriteEnable;
			depthStencil.depthCompareOp = depthCompareOp;
			depthStencil.depthBoundsTestEnable = depthBoundsTestEnable;
			depthStencil.stencilTestEnable = stencilTestEnable;
			return depthStencil;
		}

		static VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState(
			VkBool32 blendEnable = VK_TRUE, VkBlendFactor srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
			VkBlendFactor dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
			VkBlendOp colorBlendOp = VK_BLEND_OP_ADD, VkBlendFactor srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
			VkBlendFactor dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO, VkBlendOp alphaBlendOp = VK_BLEND_OP_ADD,
			VkColorComponentFlags colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
												   VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT) {
			VkPipelineColorBlendAttachmentState colorBlendAttachment{};
			colorBlendAttachment.blendEnable = blendEnable;
			colorBlendAttachment.srcColorBlendFactor = srcColorBlendFactor;
			colorBlendAttachment.dstColorBlendFactor = dstColorBlendFactor;
			colorBlendAttachment.colorBlendOp = colorBlendOp;
			colorBlendAttachment.srcAlphaBlendFactor = srcAlphaBlendFactor;
			colorBlendAttachment.dstAlphaBlendFactor = dstAlphaBlendFactor;
			colorBlendAttachment.alphaBlendOp = alphaBlendOp;
			colorBlendAttachment.colorWriteMask = colorWriteMask;
			return colorBlendAttachment;
		}

		static VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(
			uint32_t attachmentCount = 1, const VkPipelineColorBlendAttachmentState* pAttachments = nullptr,
			VkBool32 logicOpEnable = VK_FALSE, VkLogicOp logicOp = VK_LOGIC_OP_COPY,
			std::vector<float> blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}) {
			VkPipelineColorBlendStateCreateInfo colorBlending{};
			colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlending.logicOpEnable = logicOpEnable;
			colorBlending.logicOp = logicOp;
			colorBlending.attachmentCount = attachmentCount;
			colorBlending.pAttachments = pAttachments;
			colorBlending.blendConstants[0] = blendConstants.size() > 0 ? blendConstants[0] : 1.0f;
			colorBlending.blendConstants[1] = blendConstants.size() > 1 ? blendConstants[1] : 1.0f;
			colorBlending.blendConstants[2] = blendConstants.size() > 2 ? blendConstants[2] : 1.0f;
			colorBlending.blendConstants[3] = blendConstants.size() > 3 ? blendConstants[3] : 1.0f;
			return colorBlending;
		}

		static VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(
			std::vector<VkDynamicState>& dynamicStates) {
			VkPipelineDynamicStateCreateInfo dynamicState{};
			dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
			dynamicState.pDynamicStates = dynamicStates.data();
			return dynamicState;
		}

		static VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo(
			uint32_t stageCount, VkPipelineShaderStageCreateInfo* pStages,
			VkPipelineVertexInputStateCreateInfo* pVertexInputState,
			VkPipelineInputAssemblyStateCreateInfo* pInputAssemblyState,
			VkPipelineViewportStateCreateInfo* pViewportState,
			VkPipelineRasterizationStateCreateInfo* pRasterizationState,
			VkPipelineMultisampleStateCreateInfo* pMultisampleState,
			VkPipelineDepthStencilStateCreateInfo* pDepthStencilState,
			VkPipelineColorBlendStateCreateInfo* pColorBlendState, VkPipelineDynamicStateCreateInfo* pDynamicState,
			VkPipelineLayout layout, VkRenderPass renderPass, uint32_t subpass,
			VkPipeline basePipelineHandle = VK_NULL_HANDLE, int32_t basePipelineIndex = 0) {
			VkGraphicsPipelineCreateInfo pipelineInfo{};
			pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineInfo.stageCount = stageCount;
			pipelineInfo.pStages = pStages;
			pipelineInfo.pVertexInputState = pVertexInputState;
			pipelineInfo.pInputAssemblyState = pInputAssemblyState;
			pipelineInfo.pViewportState = pViewportState;
			pipelineInfo.pRasterizationState = pRasterizationState;
			pipelineInfo.pMultisampleState = pMultisampleState;
			pipelineInfo.pDepthStencilState = pDepthStencilState;
			pipelineInfo.pColorBlendState = pColorBlendState;
			pipelineInfo.pDynamicState = pDynamicState;
			pipelineInfo.layout = layout;
			pipelineInfo.renderPass = renderPass;
			pipelineInfo.subpass = subpass;
			pipelineInfo.basePipelineHandle = basePipelineHandle;
			return pipelineInfo;
		}

		static VkRenderPassBeginInfo renderPassBeginInfo(VkRenderPass renderPass, VkFramebuffer frameBuffer,
														 uint32_t renderAreaX, uint32_t renderAreaY, uint32_t offsetX,
														 uint32_t offsetY, uint32_t clearValueCount,
														 VkClearValue* clearValue) {
			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderPass;
			renderPassInfo.framebuffer = frameBuffer;
			renderPassInfo.renderArea.extent.width = renderAreaX;
			renderPassInfo.renderArea.extent.height = renderAreaY;
			renderPassInfo.renderArea.offset.x = offsetX;
			renderPassInfo.renderArea.offset.y = offsetY;
			renderPassInfo.clearValueCount = clearValueCount;
			renderPassInfo.pClearValues = clearValue;
			return renderPassInfo;
		}

		static VkViewport viewport(float x, float y, float width, float height, float minDepth = 0.0f,
								   float maxDepth = 1.0f) {
			VkViewport view{};
			view.x = x;
			view.y = y;
			view.width = width;
			view.height = height;
			view.minDepth = minDepth;
			view.maxDepth = maxDepth;
			return view;
		}

		static VkExtent2D extent2D(uint32_t width, uint32_t height) {
			VkExtent2D extent{};
			extent.width = width;
			extent.height = height;
			return extent;
		}

		static VkRect2D rect2D(int32_t offsetX, int32_t offsetY, uint32_t width, uint32_t height) {
			VkRect2D rect{};
			rect.offset = {offsetX, offsetY};
			rect.extent.width = width;
			rect.extent.height = height;
			return rect;
		}
	} // namespace plvk
} // namespace Plaza
