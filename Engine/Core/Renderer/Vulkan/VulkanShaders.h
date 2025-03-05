#pragma once
namespace Plaza {
	class VulkanShaders {
	  public:
		static VkShaderModule CreateShaderModule(const std::vector<char>& code, VkDevice device);
		static std::vector<char> ReadFile(const std::string& filename);
		std::string mVertexShaderPath;
		std::string mFragmentShaderPath;
		std::string mGeometryShaderPath;

		VkShaderModule vertShaderModule{};
		VkShaderModule fragShaderModule{};
		VkShaderModule geomShaderModule{};

		VulkanShaders(){};
		VulkanShaders(std::string vertexPath, std::string fragmentPath, std::string geometryPath)
			: mVertexShaderPath(vertexPath), mFragmentShaderPath(fragmentPath), mGeometryShaderPath(geometryPath){};

		VkPipeline mPipeline = VK_NULL_HANDLE;
		VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> mDescriptorSets = std::vector<VkDescriptorSet>();
		VkDescriptorSetLayout mDescriptorSetLayout = VK_NULL_HANDLE;

		void Init(VkDevice device, VkRenderPass renderPass, int width, int height,
				  VkDescriptorSetLayout descriptorSetLayout, VkPipelineLayoutCreateInfo pipelineLayoutInfo,
				  std::vector<VkPushConstantRange> pushConstantRanges = std::vector<VkPushConstantRange>(),
				  bool useVertexInputInfo = true);
		void InitializeDefaultValues(
			VkDevice device, VkRenderPass renderPass, int width, int height, VkDescriptorSetLayout descriptorSetLayout,
			VkPipelineLayoutCreateInfo pipelineLayoutInfo,
			std::vector<VkPushConstantRange> pushConstantRanges = std::vector<VkPushConstantRange>(),
			bool useVertexInputInfo = true);
		void InitializeFull(VkDevice device, VkPipelineLayoutCreateInfo pipelineLayoutInfo, bool useVertexInputInfo,
							int width, int height, std::vector<VkPipelineShaderStageCreateInfo> shaderStages,
							VkPipelineVertexInputStateCreateInfo vertexInputInfo,
							VkPipelineInputAssemblyStateCreateInfo inputAssembly,
							VkPipelineViewportStateCreateInfo viewportState,
							VkPipelineRasterizationStateCreateInfo rasterizer,
							VkPipelineMultisampleStateCreateInfo multisampling,
							VkPipelineColorBlendStateCreateInfo colorBlending,
							VkPipelineDynamicStateCreateInfo dynamicState, VkRenderPass renderPass,
							VkPipelineDepthStencilStateCreateInfo depthStencil,
							std::vector<VkVertexInputBindingDescription> vertexInputBindings = {},
							std::vector<VkVertexInputAttributeDescription> vertexInputAttributes = {});
		void Terminate();

		std::vector<VkPipelineShaderStageCreateInfo> mShaderStages = std::vector<VkPipelineShaderStageCreateInfo>();
		VkPipelineVertexInputStateCreateInfo mVertexInputInfo{};
		VkPipelineInputAssemblyStateCreateInfo mInputAssembly{};
		VkPipelineViewportStateCreateInfo mViewportState{};
		VkPipelineRasterizationStateCreateInfo mRasterizer{};
		VkPipelineMultisampleStateCreateInfo mMultisampling{};
		VkPipelineColorBlendStateCreateInfo mColorBlending{};
		VkPipelineDynamicStateCreateInfo mDynamicState{};
		VkRenderPass mRenderPass = VK_NULL_HANDLE;
		VkPipelineDepthStencilStateCreateInfo mDepthStencil{};

	  private:
	};
} // namespace Plaza
