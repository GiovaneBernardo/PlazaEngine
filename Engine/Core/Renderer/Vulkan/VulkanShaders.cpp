#include "Engine/Core/PreCompiledHeaders.h"
#include "VulkanShaders.h"

#include "Renderer.h"
#include "Engine/Core/Renderer/RendererTypes.h"
#include "Engine/Core/Renderer/Mesh.h"

namespace Plaza {
	VkShaderModule VulkanShaders::CreateShaderModule(const std::vector<char>& code, VkDevice device) {
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}

		return shaderModule;
	}

	std::vector<char> VulkanShaders::ReadFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	static std::array<VkVertexInputAttributeDescription, 5> VertexGetAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions{};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 1;
		attributeDescriptions[1].location = 5;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[1].offset = 0;

		attributeDescriptions[2].binding = 1;
		attributeDescriptions[2].location = 6;
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[2].offset = sizeof(float) * 4;

		attributeDescriptions[3].binding = 1;
		attributeDescriptions[3].location = 7;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[3].offset = sizeof(float) * 8;

		attributeDescriptions[4].binding = 1;
		attributeDescriptions[4].location = 8;
		attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[4].offset = sizeof(float) * 12;
		return attributeDescriptions;
	}

	void VulkanShaders::InitializeDefaultValues(VkDevice device, VkRenderPass renderPass, int width, int height,
												VkDescriptorSetLayout descriptorSetLayout,
												VkPipelineLayoutCreateInfo pipelineLayoutInfo,
												std::vector<VkPushConstantRange> pushConstantRanges,
												bool useVertexInputInfo) {
		auto vertShaderCode = ReadFile(mVertexShaderPath);
		auto fragShaderCode = ReadFile(mFragmentShaderPath);
		// auto geomShaderCode = readFile(mGeometryShaderPath);

		vertShaderModule = CreateShaderModule(vertShaderCode, device);
		fragShaderModule = CreateShaderModule(fragShaderCode, device);
		// VkShaderModule geomShaderModule = createShaderModule(geomShaderCode, device);

		if (!mVertexShaderPath.empty()) {
			VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
			vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertShaderStageInfo.module = vertShaderModule;
			vertShaderStageInfo.pName = "main";
			mShaderStages.push_back(vertShaderStageInfo);
		}

		if (!mFragmentShaderPath.empty()) {
			VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
			fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragShaderStageInfo.module = fragShaderModule;
			fragShaderStageInfo.pName = "main";
			mShaderStages.push_back(fragShaderStageInfo);
		}

		// f (!mGeometryShaderPath.empty())
		//
		//	VkPipelineShaderStageCreateInfo geomShaderStageInfo{};
		//	geomShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		//	geomShaderStageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
		//	geomShaderStageInfo.module = geomShaderModule;
		//	geomShaderStageInfo.pName = "main";
		//	shaderStages.push_back(geomShaderStageInfo);
		//

		mVertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		mVertexInputInfo.vertexAttributeDescriptionCount = 0;
		mVertexInputInfo.pVertexAttributeDescriptions = nullptr;
		mVertexInputInfo.vertexBindingDescriptionCount = 0;
		mVertexInputInfo.pVertexBindingDescriptions = nullptr;

		mInputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		mInputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		mInputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)width;
		viewport.height = (float)height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent.width = (float)width;
		scissor.extent.height = (float)height;

		mViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		mViewportState.viewportCount = 1;
		mViewportState.scissorCount = 1;

		mRasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		mRasterizer.depthClampEnable = VK_FALSE;

		mRasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		mRasterizer.lineWidth = 1.0f;
		mRasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		mRasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		if (!useVertexInputInfo) {
			mRasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		}

		mMultisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		mMultisampling.sampleShadingEnable = VK_FALSE;
		mMultisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		mColorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		mColorBlending.logicOpEnable = VK_FALSE;
		mColorBlending.attachmentCount = 1;
		mColorBlending.pAttachments = &colorBlendAttachment;

		std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

		mDynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		mDynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		mDynamicState.pDynamicStates = dynamicStates.data();

		if (useVertexInputInfo) {
			// Populate VkVertexInputBindingDescription (if needed)
			VkVertexInputBindingDescription bindingDescription = {};
			bindingDescription.binding = 0;				   // Assuming the binding is 0
			bindingDescription.stride = sizeof(glm::vec3); // Adjust the stride based on your vertex data structure
			bindingDescription.inputRate =
				VK_VERTEX_INPUT_RATE_VERTEX; // or VK_VERTEX_INPUT_RATE_INSTANCE if using instancing

			std::array<VkVertexInputBindingDescription, 2> bindingDescriptions = {};
			bindingDescriptions[0].binding = 0;
			bindingDescriptions[0].stride = sizeof(Vertex);
			bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			VkVertexInputBindingDescription instanceBindingDescription = {};
			instanceBindingDescription.binding = 1;
			instanceBindingDescription.stride = sizeof(glm::vec4) * 4;
			instanceBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

			std::array<VkVertexInputAttributeDescription, 4> instanceAttributeDescriptions = {};
			instanceAttributeDescriptions[0].binding = 1;
			instanceAttributeDescriptions[0].location = 5;
			instanceAttributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			instanceAttributeDescriptions[0].offset = 0;

			instanceAttributeDescriptions[1].binding = 1;
			instanceAttributeDescriptions[1].location = 6;
			instanceAttributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			instanceAttributeDescriptions[1].offset = sizeof(float) * 4;

			instanceAttributeDescriptions[2].binding = 1;
			instanceAttributeDescriptions[2].location = 7;
			instanceAttributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			instanceAttributeDescriptions[2].offset = sizeof(float) * 8;

			instanceAttributeDescriptions[3].binding = 1;
			instanceAttributeDescriptions[3].location = 8;
			instanceAttributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			instanceAttributeDescriptions[3].offset = sizeof(float) * 12;
			bindingDescriptions[1] = instanceBindingDescription;

			auto attributeDescriptions = VertexGetAttributeDescriptions();

			mVertexInputInfo.vertexBindingDescriptionCount = bindingDescriptions.size();
			mVertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
			mVertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
			mVertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
		}
		else {
			mVertexInputInfo.vertexBindingDescriptionCount = 0;
			mVertexInputInfo.vertexAttributeDescriptionCount = 0;
		}

		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

		mColorBlending.attachmentCount = useVertexInputInfo ? 1 : 1;
		mDepthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		mDepthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		mDepthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		mDepthStencil.depthTestEnable = VK_TRUE;
		mDepthStencil.depthWriteEnable = VK_TRUE;
		// mDepthStencil.depthWriteEnable = VK_FALSE;
		mDepthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		mDepthStencil.depthBoundsTestEnable = VK_FALSE;
		mDepthStencil.stencilTestEnable = VK_FALSE;
	}

	void VulkanShaders::Init(VkDevice device, VkRenderPass renderPass, int width, int height,
							 VkDescriptorSetLayout descriptorSetLayout, VkPipelineLayoutCreateInfo pipelineLayoutInfo,
							 std::vector<VkPushConstantRange> pushConstantRanges, bool useVertexInputInfo) {
		// InitializeDefaultValues(device, renderPass, width, height, descriptorSetLayout, pipelineLayoutInfo,
		// pushConstantRanges, useVertexInputInfo);

		InitializeFull(device, pipelineLayoutInfo, useVertexInputInfo, width, height, mShaderStages, mVertexInputInfo,
					   mInputAssembly, mViewportState, mRasterizer, mMultisampling, mColorBlending, mDynamicState,
					   renderPass, mDepthStencil);
	}

	void VulkanShaders::InitializeFull(
		VkDevice device, VkPipelineLayoutCreateInfo pipelineLayoutInfo, bool useVertexInputInfo, int width, int height,
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages, VkPipelineVertexInputStateCreateInfo vertexInputInfo,
		VkPipelineInputAssemblyStateCreateInfo inputAssembly, VkPipelineViewportStateCreateInfo viewportState,
		VkPipelineRasterizationStateCreateInfo rasterizer, VkPipelineMultisampleStateCreateInfo multisampling,
		VkPipelineColorBlendStateCreateInfo colorBlending, VkPipelineDynamicStateCreateInfo dynamicState,
		VkRenderPass renderPass, VkPipelineDepthStencilStateCreateInfo depthStencil,
		std::vector<VkVertexInputBindingDescription> vertexInputBindings,
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributes) {
		// auto geomShaderCode = readFile(mGeometryShaderPath);

		// VkShaderModule geomShaderModule = createShaderModule(geomShaderCode, device);

		if (!mVertexShaderPath.empty()) {
			auto vertShaderCode = ReadFile(mVertexShaderPath);
			vertShaderModule = CreateShaderModule(vertShaderCode, device);
			VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
			vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertShaderStageInfo.module = vertShaderModule;
			vertShaderStageInfo.pName = "main";
			mShaderStages.push_back(vertShaderStageInfo);
		}

		if (!mFragmentShaderPath.empty()) {
			auto fragShaderCode = ReadFile(mFragmentShaderPath);
			fragShaderModule = CreateShaderModule(fragShaderCode, device);
			VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
			fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragShaderStageInfo.module = fragShaderModule;
			fragShaderStageInfo.pName = "main";
			mShaderStages.push_back(fragShaderStageInfo);
		}

		// f (!mGeometryShaderPath.empty())
		//
		//	VkPipelineShaderStageCreateInfo geomShaderStageInfo{};
		//	geomShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		//	geomShaderStageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
		//	geomShaderStageInfo.module = geomShaderModule;
		//	geomShaderStageInfo.pName = "main";
		//	shaderStages.push_back(geomShaderStageInfo);
		//

		mVertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		mVertexInputInfo.vertexAttributeDescriptionCount = 0;
		mVertexInputInfo.pVertexAttributeDescriptions = nullptr;
		mVertexInputInfo.vertexBindingDescriptionCount = 0;
		mVertexInputInfo.pVertexBindingDescriptions = nullptr;

		mInputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		mInputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		mInputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)width;
		viewport.height = (float)height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent.width = (float)width;
		scissor.extent.height = (float)height;

		mViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		mViewportState.viewportCount = 1;
		mViewportState.scissorCount = 1;

		mRasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		mRasterizer.depthClampEnable = VK_FALSE;

		mRasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		mRasterizer.lineWidth = 1.0f;
		mRasterizer.cullMode = VK_CULL_MODE_NONE; // VK_CULL_MODE_FRONT_BIT;
		mRasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		mMultisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

		mMultisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		mColorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		mColorBlending.logicOpEnable = VK_FALSE;
		mColorBlending.attachmentCount = 1;
		mColorBlending.pAttachments = &colorBlendAttachment;

		std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

		mDynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		mDynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		mDynamicState.pDynamicStates = dynamicStates.data();

		if (useVertexInputInfo && vertexInputBindings.size() == 0) {
			// Populate VkVertexInputBindingDescription (if needed)
			VkVertexInputBindingDescription bindingDescription = {};
			bindingDescription.binding = 0;				   // Assuming the binding is 0
			bindingDescription.stride = sizeof(glm::vec3); // Adjust the stride based on your vertex data structure
			bindingDescription.inputRate =
				VK_VERTEX_INPUT_RATE_VERTEX; // or VK_VERTEX_INPUT_RATE_INSTANCE if using instancing

			std::array<VkVertexInputBindingDescription, 2> bindingDescriptions = {};
			bindingDescriptions[0].binding = 0;
			bindingDescriptions[0].stride = sizeof(Vertex);
			bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			VkVertexInputBindingDescription instanceBindingDescription = {};
			instanceBindingDescription.binding = 1;
			instanceBindingDescription.stride = sizeof(glm::vec4) * 4;
			instanceBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

			std::array<VkVertexInputAttributeDescription, 4> instanceAttributeDescriptions = {};
			instanceAttributeDescriptions[0].binding = 1;
			instanceAttributeDescriptions[0].location = 4;
			instanceAttributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			instanceAttributeDescriptions[0].offset = 0;

			instanceAttributeDescriptions[1].binding = 1;
			instanceAttributeDescriptions[1].location = 5;
			instanceAttributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			instanceAttributeDescriptions[1].offset = sizeof(float) * 4;

			instanceAttributeDescriptions[2].binding = 1;
			instanceAttributeDescriptions[2].location = 6;
			instanceAttributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			instanceAttributeDescriptions[2].offset = sizeof(float) * 8;

			instanceAttributeDescriptions[3].binding = 1;
			instanceAttributeDescriptions[3].location = 7;
			instanceAttributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			instanceAttributeDescriptions[3].offset = sizeof(float) * 12;
			bindingDescriptions[1] = instanceBindingDescription;

			auto attributeDescriptions = VertexGetAttributeDescriptions();

			std::cout << "Start \n";
			for (unsigned int i = 0; i < instanceAttributeDescriptions.size(); ++i) {
				std::cout << "instanceAttributeDescriptions \n";
				std::cout << instanceAttributeDescriptions[i].binding << "\n";
				std::cout << instanceAttributeDescriptions[i].location << "\n";
				std::cout << instanceAttributeDescriptions[i].format << "\n";
				std::cout << instanceAttributeDescriptions[i].offset << "\n";
			}
			for (unsigned int i = 0; i < bindingDescriptions.size(); ++i) {
				std::cout << "bindingDescriptions \n";
				std::cout << bindingDescriptions[i].binding << "\n";
				std::cout << bindingDescriptions[i].stride << "\n";
				std::cout << bindingDescriptions[i].inputRate << "\n";
			}

			std::cout << "End \n";

			mVertexInputInfo.vertexBindingDescriptionCount = bindingDescriptions.size();
			mVertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
			mVertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
			mVertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
		}
		else if (!useVertexInputInfo && vertexInputBindings.size() == 0) {
			mVertexInputInfo.vertexBindingDescriptionCount = 0;
			mVertexInputInfo.vertexAttributeDescriptionCount = 0;
		}
		else if (useVertexInputInfo) {
			mVertexInputInfo.vertexBindingDescriptionCount = vertexInputBindings.size();
			mVertexInputInfo.pVertexBindingDescriptions = vertexInputBindings.data();
			mVertexInputInfo.vertexAttributeDescriptionCount = vertexInputAttributes.size();
			mVertexInputInfo.pVertexAttributeDescriptions = vertexInputAttributes.data();
		}

		mColorBlending.attachmentCount = useVertexInputInfo ? 1 : 1;
		mDepthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		mDepthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		mDepthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		mDepthStencil.depthTestEnable = VK_TRUE;
		mDepthStencil.depthWriteEnable = VK_TRUE;
		mDepthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		mDepthStencil.depthBoundsTestEnable = VK_FALSE;
		mDepthStencil.stencilTestEnable = VK_FALSE;

		shaderStages = shaderStages.size() == 0 ? mShaderStages : shaderStages;
		// VK_STRUCTURE_TYPE_APPLICATION_INFO means the struct hasnt been initialized
		vertexInputInfo =
			vertexInputInfo.sType == VK_STRUCTURE_TYPE_APPLICATION_INFO ? mVertexInputInfo : vertexInputInfo;
		inputAssembly = inputAssembly.sType == VK_STRUCTURE_TYPE_APPLICATION_INFO ? mInputAssembly : inputAssembly;
		viewportState = viewportState.sType == VK_STRUCTURE_TYPE_APPLICATION_INFO ? mViewportState : viewportState;
		rasterizer = rasterizer.sType == VK_STRUCTURE_TYPE_APPLICATION_INFO ? mRasterizer : rasterizer;
		multisampling = multisampling.sType == VK_STRUCTURE_TYPE_APPLICATION_INFO ? mMultisampling : multisampling;
		colorBlending = colorBlending.sType == VK_STRUCTURE_TYPE_APPLICATION_INFO ? mColorBlending : colorBlending;
		dynamicState = dynamicState.sType == VK_STRUCTURE_TYPE_APPLICATION_INFO ? mDynamicState : dynamicState;
		depthStencil = depthStencil.sType == VK_STRUCTURE_TYPE_APPLICATION_INFO ? mDepthStencil : depthStencil;

		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = shaderStages.size();
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;

		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = this->mPipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;
		pipelineInfo.pDepthStencilState = &depthStencil;

		if (vkCreateGraphicsPipelines(device, VulkanRenderer::GetRenderer()->GetPipelineCache(), 1, &pipelineInfo, nullptr, &this->mPipeline) !=
			VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}

		std::cout << "End Pipe \n";

		vkDestroyShaderModule(device, fragShaderModule, nullptr);
		vkDestroyShaderModule(device, vertShaderModule, nullptr);
	}
} // namespace Plaza
