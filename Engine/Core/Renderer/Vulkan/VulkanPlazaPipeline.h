#pragma once
#include "Engine/Core/Renderer/PlazaPipeline.h"
#include "VulkanShaders.h"
#include "VulkanComputeShaders.h"
#include "Engine/Core/Engine.h"

namespace Plaza {
	class PLAZA_API VulkanPlazaPipeline : public PlazaPipeline {
	  public:
		VulkanPlazaPipeline(){};
		VulkanPlazaPipeline(PlPipelineCreateInfo createInfo) : PlazaPipeline(createInfo){};

		VkRenderPass mRenderPass = VK_NULL_HANDLE;
		VkFramebuffer mFramebuffer = VK_NULL_HANDLE;
		VulkanShaders* mShaders = new VulkanShaders();
		VulkanComputeShaders* mComputeShaders = new VulkanComputeShaders();

		void Init(std::string vertexPath, std::string fragmentPath, std::string geometryPath, VkDevice device,
				  glm::vec2 size, VkDescriptorSetLayout descriptorSetLayout,
				  VkPipelineLayoutCreateInfo pipelineLayoutInfo) override;
		void InitializeShaders(std::string vertexPath, std::string fragmentPath, std::string geometryPath,
							   VkDevice device, glm::vec2 size, VkDescriptorSetLayout descriptorSetLayout,
							   VkPipelineLayoutCreateInfo pipelineLayoutInfo) override;
		void InitializeRenderPass(VkAttachmentDescription* attachmentDescs, uint32_t attachmentsCount,
								  VkSubpassDescription* subpasses, uint32_t subpassesCount,
								  VkSubpassDependency* dependencies, uint32_t dependenciesCount);
		void InitializeFramebuffer(VkImageView* pAttachmentsData, uint32_t attachmentsCount, glm::vec2 size,
								   uint32_t layers = 1);
		void Update() override;
		void DrawFullScreenRectangle() override;
		void Terminate() override;

		void UpdateCommandBuffer(VkCommandBuffer commandBuffer);

		template <class Archive> void serialize(Archive& archive) { archive(cereal::base_class<PlazaPipeline>(this)); }

	  private:
		VkCommandBuffer mCommandBuffer = VK_NULL_HANDLE;
	};
} // namespace Plaza
