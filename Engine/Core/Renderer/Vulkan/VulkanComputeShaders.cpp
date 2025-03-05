#include <Engine/Core/PreCompiledHeaders.h>
#include "VulkanComputeShaders.h"
#include "VulkanShaders.h"
#include "Renderer.h"

namespace Plaza {
	void VulkanComputeShaders::CreateComputeDescriptorSetLayout() {
		std::array<VkDescriptorSetLayoutBinding, 3> layoutBindings{};
		layoutBindings[0].binding = 0;
		layoutBindings[0].descriptorCount = 1;
		layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBindings[0].pImmutableSamplers = nullptr;
		layoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		layoutBindings[1].binding = 1;
		layoutBindings[1].descriptorCount = 1;
		layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		layoutBindings[1].pImmutableSamplers = nullptr;
		layoutBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		layoutBindings[2].binding = 2;
		layoutBindings[2].descriptorCount = 1;
		layoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		layoutBindings[2].pImmutableSamplers = nullptr;
		layoutBindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 3;
		layoutInfo.pBindings = layoutBindings.data();

		if (vkCreateDescriptorSetLayout(VulkanRenderer::GetRenderer()->mDevice, &layoutInfo, nullptr,
										&mComputeDescriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create compute descriptor set layout!");
		}
	}
	void VulkanComputeShaders::Init(std::string shadersPath, std::vector<VkPushConstantRange> pushConstantsRange) {
		if (mComputeDescriptorSetLayout == VK_NULL_HANDLE)
			CreateComputeDescriptorSetLayout();

		bool pathEndsWithSpv = shadersPath.ends_with(".spv");
		VkShaderModule computeShaderModule = VulkanShaders::CreateShaderModule(
			VulkanShaders::ReadFile(pathEndsWithSpv ? shadersPath : VulkanShadersCompiler::Compile(shadersPath)),
			VulkanRenderer::GetRenderer()->mDevice);

		VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
		computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		computeShaderStageInfo.module = computeShaderModule;
		computeShaderStageInfo.pName = "main";

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &mComputeDescriptorSetLayout;
		if (pushConstantsRange.size() > 0) {
			pipelineLayoutInfo.pPushConstantRanges = pushConstantsRange.data();
			pipelineLayoutInfo.pushConstantRangeCount = pushConstantsRange.size();
		}

		if (vkCreatePipelineLayout(VulkanRenderer::GetRenderer()->mDevice, &pipelineLayoutInfo, nullptr,
								   &mComputePipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create compute pipeline layout!");
		}

		VkComputePipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.layout = mComputePipelineLayout;
		pipelineInfo.stage = computeShaderStageInfo;

		if (vkCreateComputePipelines(VulkanRenderer::GetRenderer()->mDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
									 &mComputePipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create compute pipeline!");
		}

		vkDestroyShaderModule(VulkanRenderer::GetRenderer()->mDevice, computeShaderModule, nullptr);
	}

	void VulkanComputeShaders::RunCompute() {
		// UniformBufferObject ubo{};
		////ubo.deltaTime = Time::deltaTime * 1000.0f;
		//
		// memcpy(mUniformBuffersMapped[Application::Get()->mRenderer->mCurrentFrame], &ubo, sizeof(ubo));
		//
		// vkCmdBindPipeline(*VulkanRenderer::GetRenderer()->mActiveCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
		// mComputePipeline);
		//
		// vkCmdBindDescriptorSets(*VulkanRenderer::GetRenderer()->mActiveCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
		// mComputePipelineLayout, 0, 1, &mComputeDescriptorSets[VulkanRenderer::GetRenderer()->mCurrentFrame], 0,
		// nullptr);
		//
		// vkCmdDispatch(*VulkanRenderer::GetRenderer()->mActiveCommandBuffer, PARTICLE_COUNT / 256, 1, 1);
	}

	void VulkanComputeShaders::Dispatch(int x, int y, int z, void* pushConstantData, unsigned int pushConstantSize,
										VkDescriptorSet descriptorSet) {
		vkCmdBindPipeline(*VulkanRenderer::GetRenderer()->mActiveCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
						  mComputePipeline);

		uint32_t offsets[1] = {0};
		vkCmdBindDescriptorSets(*VulkanRenderer::GetRenderer()->mActiveCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
								mComputePipelineLayout, 0, 1,
								descriptorSet == VK_NULL_HANDLE
									? &mComputeDescriptorSets[VulkanRenderer::GetRenderer()->mCurrentFrame]
									: &descriptorSet,
								0, nullptr);

		if (pushConstantData)
			vkCmdPushConstants(*VulkanRenderer::GetRenderer()->mActiveCommandBuffer, this->mComputePipelineLayout,
							   VK_SHADER_STAGE_COMPUTE_BIT, 0, pushConstantSize, pushConstantData);

		vkCmdDispatch(*VulkanRenderer::GetRenderer()->mActiveCommandBuffer, x, y, z);
	}

	void VulkanComputeShaders::Draw() {
		// VkDeviceSize offsets[] = { 0 };
		// vkCmdBindVertexBuffers(*VulkanRenderer::GetRenderer()->mActiveCommandBuffer, 0, 1,
		// &mShaderStorageBuffers[VulkanRenderer::GetRenderer()->mCurrentFrame], offsets);
		//
		// vkCmdDraw(*VulkanRenderer::GetRenderer()->mActiveCommandBuffer, PARTICLE_COUNT, 1, 0, 0);
	}

	void VulkanComputeShaders::Terminate() {}

} // namespace Plaza
