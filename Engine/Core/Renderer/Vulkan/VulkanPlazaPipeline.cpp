#include "Engine/Core/PreCompiledHeaders.h"
#include "VulkanPlazaPipeline.h"
#include "Renderer.h"
#include "Engine/Core/Engine.h"

namespace Plaza {
	void VulkanPlazaPipeline::InitializeShaders(std::string vertexPath, std::string fragmentPath,
												std::string geometryPath, VkDevice device, glm::vec2 size,
												VkDescriptorSetLayout descriptorSetLayout,
												VkPipelineLayoutCreateInfo pipelineLayoutInfo) {
		this->mShaders->mVertexShaderPath = vertexPath;
		this->mShaders->mFragmentShaderPath = fragmentPath;
		this->mShaders->mGeometryShaderPath = geometryPath;
		this->mShaders->Init(device, this->mRenderPass, size.x, size.y, descriptorSetLayout, pipelineLayoutInfo,
							 std::vector<VkPushConstantRange>(), true);
	}

	void VulkanPlazaPipeline::Init(std::string vertexPath, std::string fragmentPath, std::string geometryPath,
								   VkDevice device, glm::vec2 size, VkDescriptorSetLayout descriptorSetLayout,
								   VkPipelineLayoutCreateInfo pipelineLayoutInfo) {
		this->InitializeShaders(vertexPath, fragmentPath, geometryPath, device, size, descriptorSetLayout,
								pipelineLayoutInfo);
	}

	void VulkanPlazaPipeline::InitializeRenderPass(VkAttachmentDescription* attachmentDescs, uint32_t attachmentsCount,
												   VkSubpassDescription* subpasses, uint32_t subpassesCount,
												   VkSubpassDependency* dependencies, uint32_t dependenciesCount) {
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.pAttachments = attachmentDescs;
		renderPassInfo.attachmentCount = attachmentsCount;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = subpasses;
		renderPassInfo.dependencyCount = dependenciesCount;
		renderPassInfo.pDependencies = dependencies;

		if (vkCreateRenderPass(VulkanRenderer::GetRenderer()->mDevice, &renderPassInfo, nullptr, &this->mRenderPass) !=
			VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}

	void VulkanPlazaPipeline::InitializeFramebuffer(VkImageView* pAttachmentsData, uint32_t attachmentsCount,
													glm::vec2 size, uint32_t layers) {
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = this->mRenderPass;
		framebufferInfo.attachmentCount = attachmentsCount;
		framebufferInfo.pAttachments = pAttachmentsData;
		framebufferInfo.width = Application::Get()->appSizes->sceneSize.x;
		framebufferInfo.height = Application::Get()->appSizes->sceneSize.y;
		framebufferInfo.layers = layers;

		if (vkCreateFramebuffer(VulkanRenderer::GetRenderer()->mDevice, &framebufferInfo, nullptr,
								&this->mFramebuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}

	void VulkanPlazaPipeline::Update() {}

	void VulkanPlazaPipeline::DrawFullScreenRectangle() {
		vkCmdBindDescriptorSets(*VulkanRenderer::GetRenderer()->mActiveCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
								this->mShaders->mPipelineLayout, 0, 1,
								&this->mShaders->mDescriptorSets[Application::Get()->mRenderer->mCurrentFrame], 0,
								nullptr);
		vkCmdDraw(*VulkanRenderer::GetRenderer()->mActiveCommandBuffer, 3, 1, 0, 0);
	}

	void VulkanPlazaPipeline::UpdateCommandBuffer(VkCommandBuffer commandBuffer) {
		this->mCommandBuffer = commandBuffer;
	}

	void VulkanPlazaPipeline::Terminate() {
		vkDestroyPipelineLayout(VulkanRenderer::GetRenderer()->mDevice, mShaders->mPipelineLayout, nullptr);
		vkDestroyPipeline(VulkanRenderer::GetRenderer()->mDevice, mShaders->mPipeline, nullptr);
		vkDestroyPipelineLayout(VulkanRenderer::GetRenderer()->mDevice, mComputeShaders->mComputePipelineLayout,
								nullptr);
		vkDestroyPipeline(VulkanRenderer::GetRenderer()->mDevice, mComputeShaders->mComputePipeline, nullptr);
	}
} // namespace Plaza

PL_SER_REGISTER_TYPE(Plaza::PlazaPipeline);
PL_SER_REGISTER_TYPE(Plaza::VulkanPlazaPipeline);
PL_SER_REGISTER_POLYMORPHIC_RELATION(Plaza::PlazaPipeline, Plaza::VulkanPlazaPipeline);
