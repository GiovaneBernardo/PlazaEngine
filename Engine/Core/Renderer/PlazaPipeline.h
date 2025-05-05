#pragma once
#include "PushConstants.h"
#include "Engine/Core/Engine.h"
#include "Engine/Core/Renderer/RendererTypes.h"
#include "Buffer.h"

namespace Plaza {
	class PLAZA_API PlazaPipeline {
	  public:
		std::vector<PlPushConstants> mPushConstants = std::vector<PlPushConstants>();
		uint64_t mIndirectBufferOffset = 0;
		uint64_t mIndirectBufferDrawCount = 1;
		bool mCompiled = false;

		template <typename T> void UpdatePushConstants(unsigned int index, const T& data) {
			if (mPushConstants[index].mData == nullptr)
				mPushConstants[index].mData = new T();
			memcpy(mPushConstants[index].mData, &data, sizeof(T));
		}

		PlazaPipeline() = default;
		PlazaPipeline(PlPipelineCreateInfo createInfo) : mCreateInfo(createInfo){};
		virtual void Init(std::string vertexPath, std::string fragmentPath, std::string geometryPath, VkDevice device,
						  glm::vec2 size, VkDescriptorSetLayout descriptorSetLayout,
						  VkPipelineLayoutCreateInfo pipelineLayoutInfo) {};
		virtual void InitializeShaders(std::string vertexPath, std::string fragmentPath, std::string geometryPath,
									   VkDevice device, glm::vec2 size, VkDescriptorSetLayout descriptorSetLayout,
									   VkPipelineLayoutCreateInfo pipelineLayoutInfo) {};
		virtual void Update() {};
		virtual void DrawFullScreenRectangle() {};
		virtual void Terminate() {};

		std::shared_ptr<PlBuffer> mIndirectBuffer = nullptr;
		std::vector<std::shared_ptr<PlBuffer>> mVertexBuffers = std::vector<std::shared_ptr<PlBuffer>>();

		void SetCreateInfo(PlPipelineCreateInfo createInfo) { mCreateInfo = createInfo; };
		PlPipelineCreateInfo mCreateInfo{};

		template <class Archive> void serialize(Archive& archive) {
			archive(PL_SER(mPushConstants), PL_SER(mIndirectBufferOffset), PL_SER(mIndirectBufferDrawCount),
					PL_SER(mCreateInfo));
		}
	};
} // namespace Plaza

/*
		pipeline->mShaders->mVertexShaderPath = VulkanShadersCompiler::Compile(Application::Get()->enginePath +
   "/Shaders/Vulkan/deferred/geometryPass.vert"); pipeline->mShaders->mFragmentShaderPath =
   VulkanShadersCompiler::Compile(Application::Get()->enginePath + "/Shaders/Vulkan/deferred/geometryPass.frag"); auto
   bindingsArray = VertexGetBindingDescription(); std::vector<VkVertexInputBindingDescription>
   bindings(std::begin(bindingsArray), std::end(bindingsArray)); auto attributesArray =
   VertexGetAttributeDescriptions(); std::vector<VkVertexInputAttributeDescription>
   attributes(std::begin(attributesArray), std::end(attributesArray)); VkPipelineVertexInputStateCreateInfo
   vertexInputInfo = plvk::pipelineVertexInputStateCreateInfo(bindings, attributes);
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
   plvk::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);
		VkPipelineRasterizationStateCreateInfo rasterizationState = plvk::pipelineRasterizationStateCreateInfo(VK_FALSE,
   VK_FALSE, VK_POLYGON_MODE_FILL, 1.0f, VK_FALSE, 0.0f, 0.0f, 0.0f, VK_CULL_MODE_BACK_BIT,
   VK_FRONT_FACE_COUNTER_CLOCKWISE); VkPipelineColorBlendAttachmentState blendAttachmentState =
   plvk::pipelineColorBlendAttachmentState(VK_TRUE); std::vector<VkPipelineColorBlendAttachmentState> blendAttachments{
   blendAttachmentState,blendAttachmentState,blendAttachmentState,blendAttachmentState };
		VkPipelineColorBlendStateCreateInfo colorBlendState = plvk::pipelineColorBlendStateCreateInfo(4,
   blendAttachments.data()); VkPipelineDepthStencilStateCreateInfo depthStencilState =
   plvk::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
		VkPipelineViewportStateCreateInfo viewportState = plvk::pipelineViewportStateCreateInfo(1, 1);
		VkPipelineMultisampleStateCreateInfo multisampleState =
   plvk::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0); std::vector<VkDynamicState> dynamicStateEnables =
   { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR }; VkPipelineDynamicStateCreateInfo dynamicState =
   plvk::pipelineDynamicStateCreateInfo(dynamicStateEnables); pipeline->mShaders->InitializeFull(
			VulkanRenderer::GetRenderer()->mDevice,
			pipelineLayoutCreateInfo,
			true,
			Application::Get()->appSizes->sceneSize.x,
			Application::Get()->appSizes->sceneSize.y,
			{},
			vertexInputInfo,
			inputAssemblyState,
			viewportState,
			rasterizationState,
			multisampleState,
			colorBlendState,
			dynamicState,
			renderPass->mRenderPass,
			depthStencilState
		);
*/
