#include "Engine/Core/PreCompiledHeaders.h"
#include "VulkanRenderGraph.h"
#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/imgui_impl_vulkan.h"
#include "Editor/DefaultAssets/Models/DefaultModels.h"
#include "Engine/Core/Renderer/Vulkan/VulkanGuiRenderer.h"
#include "Engine/Components/Drawing/UI/Gui.h"
#include "Engine/Components/Drawing/UI/GuiButton.h"
#include "Renderer.h"
#include "Engine/Core/Scene.h"
#include "Engine/Core/Renderer/ShaderReflection.h"

#define FRAMES_IN_FLIGHT 2;

namespace Plaza {
	void VulkanRenderPass::CompilePipeline(std::shared_ptr<PlazaPipeline> plazaPipeline) {
		plazaPipeline->mCompiled = true;
		VulkanPlazaPipeline* pipeline = static_cast<VulkanPlazaPipeline*>(plazaPipeline.get());
		PlPipelineCreateInfo createInfo = pipeline->mCreateInfo;

		std::vector<VkPushConstantRange> pushConstants = std::vector<VkPushConstantRange>();
		if (pipeline->mPushConstants.size() == 0) {
			for (PlPushConstantRange range : createInfo.pushConstants) {
				pushConstants.push_back(
					plvk::pushConstantRange(PlRenderStageToVkShaderStage(range.stageFlags), range.offset, range.size));
				pipeline->mPushConstants.push_back(PlPushConstants(range.stageFlags, range.offset, range.size));
			}
		}

		bool isComputeShaders = createInfo.renderMethod == PL_RENDER_PASS_COMPUTE;
		if (isComputeShaders) {
			pipeline->mComputeShaders->mComputeDescriptorSetLayout = mDescriptorSetLayout;
			pipeline->mComputeShaders->mComputeDescriptorSets = mDescriptorSets;
			// pipeline->mComputeShaders->mComputeDescriptorSetLayout = mDescriptorSetLayout;
			pipeline->mComputeShaders->Init(mBaseShaderPath.string(), pushConstants);
			return;
		}

		pipeline->mRenderPass = this->mRenderPass;
		pipeline->mFramebuffer = this->mFrameBuffer;
		pipeline->mShaders->mDescriptorSetLayout = mDescriptorSetLayout;
		pipeline->mShaders->mDescriptorSets = mDescriptorSets;

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = plvk::pipelineLayoutCreateInfo(
			1, &pipeline->mShaders->mDescriptorSetLayout, pushConstants.size(), pushConstants.data());

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages{};
		for (size_t i = 0; i < mReflectedShaders.size(); ++i) {
			const ShaderReflection::Shader& shader = mReflectedShaders[i];

			VkPipelineShaderStageCreateInfo shaderStage = {};
			shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStage.stage = static_cast<VkShaderStageFlagBits>(
				PlRenderStageToVkShaderStage(ShaderReflection::ShaderTypeToPlRenderStageFlags(shader.mShaderType)));
			shaderStage.pName = shader.mEntryName.c_str();

			// std::string shadersPath = ShaderReflection::ReadSpirVBinary();
			// if (!shadersPath.ends_with(".spv"))
			//	shadersPath = VulkanShadersCompiler::Compile(shadersPath);

			std::vector<uint32_t> shaderCode = shader.mShadersData[0]; // VulkanShaders::ReadFile(shadersPath.c_str());
			shaderStage.module = VulkanShaders::CreateShaderModule(shaderCode, VulkanRenderer::GetRenderer()->mDevice);

			shaderStages.push_back(shaderStage);
		}

		// Convert PlPipelineCreateInfo to be used for creating the Vulkan pipeline
		std::vector<VkVertexInputBindingDescription> bindings = std::vector<VkVertexInputBindingDescription>();

		for (const PlVertexInputBindingDescription& description : createInfo.vertexBindingDescriptions) {
			VkVertexInputBindingDescription binding{};
			binding.binding = description.binding;
			binding.stride = description.stride;
			binding.inputRate = PlVertexInputRateToVkVertexInputRate(description.inputRate);
			bindings.push_back(binding);
		}

		std::vector<VkVertexInputAttributeDescription> attributes = std::vector<VkVertexInputAttributeDescription>();
		for (const PlVertexInputAttributeDescription& description : createInfo.vertexAttributeDescriptions) {
			VkVertexInputAttributeDescription attribute{};
			attribute.location = description.location;
			attribute.binding = description.binding;
			attribute.format = PlImageFormatToVkFormat(description.format);
			attribute.offset = description.offset;
			attributes.push_back(attribute);
		}
		VkPipelineVertexInputStateCreateInfo vertexInputInfo =
			plvk::pipelineVertexInputStateCreateInfo(bindings, attributes);
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = plvk::pipelineInputAssemblyStateCreateInfo(
			PlTopologyToVkTopology(createInfo.topology), createInfo.primitiveRestartEnable);
		VkPipelineRasterizationStateCreateInfo rasterizationState = plvk::pipelineRasterizationStateCreateInfo(
			createInfo.rasterization.depthClampEnable, createInfo.rasterization.rasterizerDiscardEnable,
			PlPolygonModeToVkPolygonMode(createInfo.rasterization.polygonMode), createInfo.rasterization.lineWidth,
			createInfo.rasterization.depthBiasEnable, createInfo.rasterization.depthBiasConstantFactor,
			createInfo.rasterization.depthBiasClamp, createInfo.rasterization.depthBiasSlopeFactor,
			PlCullModeToVkCullMode(createInfo.rasterization.cullMode),
			PlFrontFaceToVkFrontFace(createInfo.rasterization.frontFace));
		std::vector<VkPipelineColorBlendAttachmentState> blendAttachments{};
		for (const PlPipelineColorBlendAttachmentState& attachment : createInfo.colorBlendState.attachments) {
			blendAttachments.push_back(plvk::pipelineColorBlendAttachmentState(
				attachment.blendEnable, PlBlendFactorToVkBlendFactor(attachment.srcColorBlendFactor),
				PlBlendFactorToVkBlendFactor(attachment.dstColorBlendFactor),
				PlBlendOpToVkBlendOp(attachment.colorBlendOp),
				PlBlendFactorToVkBlendFactor(attachment.srcAlphaBlendFactor),
				PlBlendFactorToVkBlendFactor(attachment.dstAlphaBlendFactor),
				PlBlendOpToVkBlendOp(attachment.alphaBlendOp), attachment.colorWriteMask));
		}
		VkPipelineColorBlendStateCreateInfo colorBlendState = plvk::pipelineColorBlendStateCreateInfo(
			blendAttachments.size(), blendAttachments.data(), createInfo.colorBlendState.logicOpEnable,
			PlLogicOpToVkLogicOp(createInfo.colorBlendState.logicOp),
			std::vector<float>(std::begin(createInfo.colorBlendState.blendConstants),
							   std::end(createInfo.colorBlendState.blendConstants)));
		VkPipelineDepthStencilStateCreateInfo depthStencilState = plvk::pipelineDepthStencilStateCreateInfo(
			createInfo.depthStencilState.depthTestEnable, createInfo.depthStencilState.depthWriteEnable,
			PlCompareOpToVkCompareOp(createInfo.depthStencilState.depthCompareOp),
			createInfo.depthStencilState.depthBoundsTestEnable, createInfo.depthStencilState.stencilTestEnable,
			PlStencilOpStateToVkStencilOpState(createInfo.depthStencilState.front),
			PlStencilOpStateToVkStencilOpState(createInfo.depthStencilState.back));
		VkPipelineViewportStateCreateInfo viewportState = plvk::pipelineViewportStateCreateInfo(
			createInfo.viewPortState.viewportCount, createInfo.viewPortState.scissorsCount);
		VkPipelineMultisampleStateCreateInfo multisampleState = plvk::pipelineMultisampleStateCreateInfo(
			PlSampleCountToVkSampleCount(createInfo.multiSampleState.rasterizationSamples),
			createInfo.multiSampleState.sampleShadingEnable);
		std::vector<VkDynamicState> dynamicStates{};
		for (PlDynamicState state : createInfo.dynamicStates) {
			dynamicStates.push_back(PlDynamicStateToVkDynamicState(state));
		}
		VkPipelineDynamicStateCreateInfo dynamicState = plvk::pipelineDynamicStateCreateInfo(dynamicStates);

		// Create the pipeline with the converted types from Plaza to Vulkan
		pipeline->mShaders->InitializeFull(
			VulkanRenderer::GetRenderer()->mDevice, pipelineLayoutCreateInfo,
			createInfo.vertexAttributeDescriptions.size() > 0 || createInfo.vertexBindingDescriptions.size() > 0
				? true
				: false,
			mRenderSize.x, mRenderSize.y, shaderStages, vertexInputInfo, inputAssemblyState, viewportState,
			rasterizationState, multisampleState, colorBlendState, dynamicState, mRenderPass, depthStencilState);
		// mPipelines.push_back(plazaPipeline);
	}

	void VulkanRenderPass::TerminatePipeline(std::shared_ptr<PlazaPipeline> plazaPipeline) {
		plazaPipeline->Terminate();
	}

	void VulkanRenderPass::ResetPipelineCompiledBool() {
		for (auto& pipeline : mPipelines) {
			pipeline->mCompiled = false;
		}
		for (auto& child : mChildPasses) {
			child->ResetPipelineCompiledBool();
		}
	}

	void VulkanRenderPass::ReCompileShaders(PlazaRenderGraph* graph, bool resetCompiledBool) {
		VulkanRenderer::GetRenderer()->WaitRendererHere();
		if (resetCompiledBool)
			ResetPipelineCompiledBool();
		for (auto& pipeline : mPipelines) {
			if (this->mRenderMethod == PL_RENDER_PASS_HOLDER)
				continue;

			if (!pipeline->mCompiled) {
				PlPipelineCreateInfo createInfo = pipeline->mCreateInfo;
				this->TerminatePipeline(pipeline);

				// Clear reflection data and run reflection again
				mReflectedShaders.clear();
				mResourcesInfo.clear();
				mOutputBindings.clear();
				mOutputBindingNames.clear();
				mInputBindings.clear();
				mInputBindingNames.clear();
				pipeline->mPushConstants.clear();
				this->ReflectPass(graph);
				this->Compile(graph);
				// this->CompilePipeline(pipeline);
			}
			else {
				VulkanPlazaPipeline* vkPipeline = static_cast<VulkanPlazaPipeline*>(pipeline.get());
				mDescriptorSetLayout = vkPipeline->mShaders->mDescriptorSetLayout != VK_NULL_HANDLE
										   ? vkPipeline->mShaders->mDescriptorSetLayout
										   : vkPipeline->mComputeShaders->mComputeDescriptorSetLayout;
			}
		}
		for (auto& child : mChildPasses) {
			child->ReCompileShaders(graph, false);
		}
	}

	void VulkanRenderGraph::CreatePipeline(PlPipelineCreateInfo createInfo) {}

	std::shared_ptr<PlazaPipeline> VulkanRenderPass::AddPipeline(const PlPipelineCreateInfo& createInfo) {
		std::shared_ptr<VulkanPlazaPipeline> pipeline = std::make_shared<VulkanPlazaPipeline>();
		pipeline->mCreateInfo = createInfo;
		mPipelines.push_back(pipeline);

		bool failed = false;
		// TODO: Move this sanity check to somewhere else
		// Sanity check if the pipeline is valid
		// if (createInfo.colorBlendState.attachments.size() != mFramebufferAttachments.size()) {
		//	PL_CORE_ERROR(
		//		"Pipeline color blend state attachments count must be the same as the amount of render targets \n");
		//	failed = true;
		//}

		if (failed)
			assert(false && "Pipeline is not valid. Pipeline name: {}", createInfo.pipelineName.c_str());

		return pipeline;
	}

	bool compareRenderPasses(const std::shared_ptr<PlazaRenderPass>& a, const std::shared_ptr<PlazaRenderPass>& b) {
		return a->mExecutionIndex < b->mExecutionIndex;
	}

	void OrderPassDependencies(int16_t currentIndex, std::shared_ptr<PlazaRenderPass> pass,
							   std::set<std::string>& alreadyOrderedPasses) {
		alreadyOrderedPasses.emplace(pass->mName);
		pass->mExecutionIndex =
			currentIndex; // pass->mExecutionIndex > currentIndex ? pass->mExecutionIndex : currentIndex;
		for (const auto& dependent : pass->mDependents) {
			if (dependent.second != pass)
				OrderPassDependencies(currentIndex + 1, dependent.second, alreadyOrderedPasses);
			// dependency.second->mExecutionIndex = currentIndex;
		}
	}

	void VulkanRenderGraph::OrderPasses() {
		std::map<std::string, BindingModifiers> bindingsModifiers = std::map<std::string, BindingModifiers>();

		// mOrderedPasses.clear();
		for (const auto& [key, pass] : mPasses) {
			// mOrderedPasses.push_back(pass);
			for (const auto& resource : pass->mInputBindings) {
				if (bindingsModifiers.find(resource->mName) == bindingsModifiers.end())
					bindingsModifiers.emplace(resource->mName, BindingModifiers(resource));
				bindingsModifiers[resource->mName].readPasses.push_back(pass->mName);
			}
			for (const auto& resource : pass->mOutputBindings) {
				if (bindingsModifiers.find(resource->mName) == bindingsModifiers.end())
					bindingsModifiers.emplace(resource->mName, BindingModifiers(resource));
				bindingsModifiers[resource->mName].writePasses.push_back(pass->mName);
			}
		}

		for (int i = mOrderedPasses.size() - 1; i >= 0; i--) {
			for (const auto& resource : mOrderedPasses[i]->mInputBindings) {
				BindingModifiers binding = bindingsModifiers[resource->mName];
				for (const std::string& name : binding.writePasses)
					mOrderedPasses[i]->mDependencies.emplace(name, this->GetSharedRenderPass(name));
			}

			for (const auto& resource : mOrderedPasses[i]->mOutputBindings) {
				BindingModifiers binding = bindingsModifiers[resource->mName];
				for (const std::string& name : binding.readPasses)
					mOrderedPasses[i]->mDependents.emplace(name, this->GetSharedRenderPass(name));
			}
		}

		std::set<std::string> alreadyOrderedPasses = std::set<std::string>();

		for (int i = 0; i < mOrderedPasses.size(); ++i) {
			if (alreadyOrderedPasses.find(mOrderedPasses[i]->mName) != alreadyOrderedPasses.end() ||
				mOrderedPasses[i]->mDependents.size() == 0)
				continue;
			mOrderedPasses[i]->mExecutionIndex = 0;
			OrderPassDependencies(0, mOrderedPasses[mOrderedPasses[i]->mExecutionIndex], alreadyOrderedPasses);
		}

		std::sort(mOrderedPasses.begin(), mOrderedPasses.end(), compareRenderPasses);
		for (unsigned int i = 0; i < mOrderedPasses.size(); ++i) {
			mOrderedPasses[i]->mExecutionIndex = i;
		}
	}

	void VulkanRenderGraph::CompileBuffer(std::shared_ptr<PlBuffer> buffer, std::set<std::string>& compiledBindings) {
		// if (compiledBindings.find(buffer->mName) != compiledBindings.end())
		//	return;
		buffer->CreateBuffer(buffer->mMaxItems * buffer->mStride, PlBufferUsageToVkBufferUsage(buffer->mBufferUsage),
							 PlMemoryUsageToVmaMemoryUsage(buffer->mMemoryUsage), 0, buffer->mBufferCount);
		buffer->CreateMemory(0, buffer->mBufferCount);
	}

	void VulkanRenderGraph::CompileTexture(std::shared_ptr<PlazaTextureBinding> binding,
										   std::set<std::string>& compiledBindings) {
		if (binding->mMaxBindlessResources > 0)
			return;

		// this->mTexture->mCurrentImageLayout = this->GetTextureInfo().mInitialLayout;
		bool viewMustBeNonDefault = binding->mBaseMipLevel != 0 || binding->mBaseLayerLevel != 0;

		VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
		if (binding->GetTextureInfo().mImageUsage & PL_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT) {
			aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
			if (VulkanRenderer::IsFormatStencil(PlImageFormatToVkFormat(binding->GetTextureInfo().mFormat)))
				aspect |= VK_IMAGE_ASPECT_STENCIL_BIT;
		};

		if (viewMustBeNonDefault) {
			PL_CORE_CRITICAL("NOT YET REFACTORED FOR NEW RENDER GRAPHS!");
			// mNonDefaultView = VulkanRenderer::GetRenderer()->CreateImageView(
			//	this->GetTexture()->mImage, this->GetTexture()->GetFormat(), aspect,
			//	PlViewTypeToVkImageViewType(GetTextureInfo().mViewType), GetTextureInfo().mLayersCount, 1,
			//	mBaseMipLevel);
			return;
		}

		if (compiledBindings.find(binding->mName) != compiledBindings.end())
			return;

		VulkanTexture* vkTexture = static_cast<VulkanTexture*>(binding->mTexture.get());
		VkImageUsageFlags flags = 0;
		if (binding->GetTextureInfo().mViewType == PL_VIEW_TYPE_CUBE)
			flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		if (binding->GetTextureInfo().mPath == "") {
			vkTexture->CreateTextureImage(
				VulkanRenderer::GetRenderer()->mDevice, PlImageFormatToVkFormat(vkTexture->GetTextureInfo().mFormat),
				binding->mTexture->mResolution.x, vkTexture->mResolution.y, vkTexture->mMipCount == 0 ? true : false,
				PlImageUsageToVkImageUsage(vkTexture->GetTextureInfo().mImageUsage),
				PlTextureTypeToVkImageType(vkTexture->GetTextureInfo().mTextureType),
				PlImageTilingToVkImageTiling(vkTexture->GetTextureInfo().mImageTiling), VK_IMAGE_LAYOUT_UNDEFINED,
				vkTexture->GetTextureInfo().mLayersCount, flags, true, VK_SHARING_MODE_EXCLUSIVE);
			VulkanRenderer::GetRenderer()->TransitionTextureLayout(
				*vkTexture, PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VulkanRenderer::GetFormatAspectMask(PlImageFormatToVkFormat(vkTexture->GetTextureInfo().mFormat)),
				vkTexture->GetTextureInfo().mLayersCount, vkTexture->mMipCount);
		}
		else {
			vkTexture->CreateTextureImage(VulkanRenderer::GetRenderer()->mDevice, vkTexture->GetTextureInfo().mPath,
										  PlImageFormatToVkFormat(vkTexture->GetTextureInfo().mFormat), true,
										  vkTexture->GetTextureInfo().mIsHdr,
										  PlImageUsageToVkImageUsage(vkTexture->GetTextureInfo().mImageUsage));
		}

		if (vkTexture->GetTextureInfo().mInitialLayout != PL_IMAGE_LAYOUT_UNDEFINED) {
			VulkanRenderer::GetRenderer()->TransitionTextureLayout(
				*vkTexture, vkTexture->GetTextureInfo().mInitialLayout, aspect,
				vkTexture->GetTextureInfo().mLayersCount, vkTexture->mMipCount);
		}

		vkTexture->CreateTextureSampler(
			PlAddressModeToVkSamplerAddressMode(vkTexture->GetTextureInfo().mSamplerAddressMode));
		vkTexture->CreateImageView(PlImageFormatToVkFormat(vkTexture->GetTextureInfo().mFormat), aspect,
								   PlViewTypeToVkImageViewType(vkTexture->GetTextureInfo().mViewType),
								   vkTexture->GetTextureInfo().mLayersCount, 0);

		VulkanRenderer::GetRenderer()->AddTrackerToImage(vkTexture->mImage, vkTexture->mAssetName, vkTexture->mSampler,
														 vkTexture->GetTextureInfo(),
														 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void VulkanRenderGraph::CompileTextureSampler(std::shared_ptr<PlazaTextureSamplerBinding> binding,
												  std::set<std::string>& compiledBindings) {
		auto& samplerData = *binding->mSampler;
		VkSampler sampler = VulkanRenderer::GetRenderer()->CreateSampler(
			VulkanRenderer::GetRenderer()->mDevice, PlFilterToVkFilter(samplerData.mMagFilter),
			PlFilterToVkFilter(samplerData.mMinFilter), PlAddressModeToVkSamplerAddressMode(samplerData.mAddressModeU),
			PlAddressModeToVkSamplerAddressMode(samplerData.mAddressModeV),
			PlAddressModeToVkSamplerAddressMode(samplerData.mAddressModeW),
			samplerData.mUseAnisotropy ? VK_TRUE : VK_FALSE, samplerData.mMaxAnisotropy,
			PlBorderColorToVkBorderColor(samplerData.mBorderColor),
			samplerData.mUseUnnormalizedCoordinates ? VK_TRUE : VK_FALSE, samplerData.mUseCompare ? VK_TRUE : VK_FALSE,
			PlCompareOpToVkCompareOp(samplerData.mCompareOp),
			PlSamplerMipmapModeToVkSamplerMipmapMode(samplerData.mMipmapMode), samplerData.mMipLodBias,
			samplerData.mMinLod, samplerData.mMaxLod);
		static_cast<VulkanTextureSampler*>(binding->mSampler.get())->mSampler = sampler;
	}

	void VulkanRenderPass::CompileGraphics(PlazaRenderGraph* renderGraph) {
		glm::vec2 biggestSize = this->mRenderSize; // glm::vec2(0.0f);
		std::vector<VkImageView> frameBufferAttachments{};
		std::vector<VkSubpassDescription> subPasses{};
		std::vector<VkSubpassDependency> dependencies{};
		std::vector<VkAttachmentDescription> attachmentDescs{};
		std::vector<uint32_t> locations{};
		std::vector<VkAttachmentReference> colorReferences;
		VkAttachmentReference depthReference = {};
		//
		// Iterate only over ouputs and inputs that are used as depth stencil attachment
		for (unsigned int i = 0; i < glm::max(mFramebufferAttachments.size(), mInputBindings.size()); ++i) {
			std::shared_ptr<PlazaShadersBinding> value;
			if (mInputBindings.size() > i && mInputBindings[i]->mUseAsDepthStencilAttachment)
				value = mInputBindings[i];
			else if (mFramebufferAttachments.size() > i)
				value = mFramebufferAttachments[i];
			else
				continue;
			//
			// Get next layout of each output binding of this pass
			int nextPassToUseThisBinding = -1;
			for (unsigned int i = mExecutionIndex; i < renderGraph->mOrderedPasses.size(); ++i) {
				auto& pass = renderGraph->mOrderedPasses.at(i);
				if (pass->GetInputResource<PlazaShadersBinding>(value->mName) != nullptr) {
					nextPassToUseThisBinding = i;
					break;
				}
			}
			//
			PlazaTextureBinding* binding = static_cast<PlazaTextureBinding*>(value.get());
			if (!binding->mResourceName.empty())
				biggestSize = glm::max(biggestSize, glm::vec2(binding->mTexture->mResolution));
			//
			VkImageLayout finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			if (binding->mInitialLayout == PL_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
				finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			else if (binding->mInitialLayout == PL_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
				finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			// if (value->mName == "SceneDepth")
			//	finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			////VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL
			//
			// Find the first pass to use the resource used in this binding
			int firstPassToUseThisResource = mExecutionIndex;
			while (firstPassToUseThisResource < renderGraph->mOrderedPasses.size() && firstPassToUseThisResource > 0) {
				if (renderGraph->mOrderedPasses[firstPassToUseThisResource]->GetInputResource<PlazaTextureBinding>(
						value->mName))
					break;
				firstPassToUseThisResource--;
			}
			if (firstPassToUseThisResource == 0)
				firstPassToUseThisResource = mExecutionIndex;
			if (firstPassToUseThisResource != renderGraph->mOrderedPasses.size() && firstPassToUseThisResource > 0) {
				firstPassToUseThisResource = mExecutionIndex;
				while (firstPassToUseThisResource < renderGraph->mOrderedPasses.size()) {
					if (renderGraph->mOrderedPasses[firstPassToUseThisResource]->GetOutputResource<PlazaTextureBinding>(
							value->mName))
						break;
					firstPassToUseThisResource--;
				}
			}
			//
			// static_cast<VulkanTextureBinding*>(value.get());//renderGraph->mOrderedPasses[glm::max(mExecutionIndex -
			// 1, 0)]->GetInputResource<VulkanTextureBinding>(binding->GetTexture()->mAssetName);
			PlazaTextureBinding* currentPassBinding = nullptr;
			if (firstPassToUseThisResource != renderGraph->mOrderedPasses.size() &&
				firstPassToUseThisResource != mExecutionIndex) {
				currentPassBinding = renderGraph->mOrderedPasses[glm::max(firstPassToUseThisResource, 0)]
										 ->GetInputResource<PlazaTextureBinding>(binding->mTexture.get()->mAssetName);
				if (!currentPassBinding) {
					currentPassBinding =
						renderGraph->mOrderedPasses[glm::max(firstPassToUseThisResource, 0)]
							->GetOutputResource<PlazaTextureBinding>(binding->mTexture.get()->mAssetName);
				}
			}
			//
			VkImageLayout currentLayout = mExecutionIndex == 0 || currentPassBinding == nullptr
											  ? VK_IMAGE_LAYOUT_UNDEFINED
											  : PlImageLayoutToVkImageLayout(currentPassBinding->mInitialLayout);
			//
			PlazaTextureBinding* nextPassBinding =
				nextPassToUseThisBinding >= 0
					? renderGraph->mOrderedPasses[nextPassToUseThisBinding]->GetInputResource<PlazaTextureBinding>(
						  binding->mTexture.get()->mAssetName)
					: nullptr;
			//
			VkImageLayout nextLayout = nextPassToUseThisBinding == -1
										   ? finalLayout
										   : PlImageLayoutToVkImageLayout(nextPassBinding->mInitialLayout);
			//
			VkAttachmentLoadOp loadOp =
				currentLayout == VK_IMAGE_LAYOUT_UNDEFINED ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
			VkAttachmentStoreOp storeOp = nextLayout == VK_IMAGE_LAYOUT_UNDEFINED ? VK_ATTACHMENT_STORE_OP_DONT_CARE
																				  : VK_ATTACHMENT_STORE_OP_STORE;
			VkAttachmentLoadOp loadStencilOp =
				currentLayout == VK_IMAGE_LAYOUT_UNDEFINED ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
			VkAttachmentStoreOp storeStencilOp = nextLayout == VK_IMAGE_LAYOUT_UNDEFINED
													 ? VK_ATTACHMENT_STORE_OP_DONT_CARE
													 : VK_ATTACHMENT_STORE_OP_STORE;
			//
			if (binding->mAttachmentOp == PL_ATTACHMENT_OP_LOAD) {
				loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
				loadStencilOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			}
			else if (binding->mAttachmentOp == PL_ATTACHMENT_OP_CLEAR) {
				loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				loadStencilOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			}
			else if (binding->mAttachmentOp == PL_ATTACHMENT_OP_STORE) {
				storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				storeStencilOp = VK_ATTACHMENT_STORE_OP_STORE;
			}
			//
			if (currentLayout == VK_IMAGE_LAYOUT_UNDEFINED && binding &&
				static_cast<VulkanTexture*>(binding->mTexture.get())->GetFormat() == VK_FORMAT_D32_SFLOAT_S8_UINT &&
				!binding->mUseAsDepthStencilAttachment) {
			}
			//
			attachmentDescs.push_back(plvk::attachmentDescription(
				PlImageFormatToVkFormat(binding->GetTextureInfo().mFormat), VK_SAMPLE_COUNT_1_BIT, loadOp, storeOp,
				loadStencilOp, storeStencilOp, currentLayout, nextLayout));
			//
			VulkanTexture* texture = static_cast<VulkanTexture*>(binding->mTexture.get());
			//
			// Create a new image view on mip 0 if the image view contains more than one mip
			if (texture->mMipCount == 1)
				frameBufferAttachments.push_back(texture->mImageView);
			else
				frameBufferAttachments.push_back(VulkanRenderer::GetRenderer()->CreateImageView(
					texture->mImage, texture->GetFormat(),
					VulkanRenderer::GetRenderer()->GetFormatAspectMask(texture->GetFormat()),
					PlViewTypeToVkImageViewType(texture->GetTextureInfo().mViewType), 1, 1, 0));
			//
			locations.push_back(binding->mLocation);
			//
			VkClearValue clearValue{};

			VkFormat vkFormat = PlImageFormatToVkFormat(binding->mTexture->GetTextureInfo().mFormat);
			bool isDepthOrStencil = VulkanRenderer::GetRenderer()->IsFormatDepth(vkFormat) ||
									VulkanRenderer::GetRenderer()->IsFormatStencil(vkFormat) ||
									VulkanRenderer::GetRenderer()->IsFormatDepthStencil(vkFormat);
			if (isDepthOrStencil)
				clearValue.depthStencil = {1.0f, 0};
			else {
				clearValue.color = {{0.0f, 0.0f, 0.0f, 1.0f}};
			}
			mClearValues.push_back(clearValue);

			// Set depth or color references based on usage
			if (binding->GetTextureInfo().mImageUsage & PL_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT) {
				depthReference.attachment = binding->mLocation;
				depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				continue;
			}
			colorReferences.push_back({binding->mLocation, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
		}
		//
		// Sort the attachments to make location 0 image view be in index 0
		std::vector<VkAttachmentDescription> temporaryAttachmentDescs = attachmentDescs;
		std::vector<VkImageView> temporaryFrameBufferAttachments = frameBufferAttachments;
		for (unsigned int i = 0; i < locations.size(); ++i) {
			attachmentDescs[i] = temporaryAttachmentDescs[locations[i]];
			if (frameBufferAttachments.size() > i)
				frameBufferAttachments[i] = temporaryFrameBufferAttachments[locations[i]];
		}
		//
		// Define the subpass description
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.pColorAttachments = colorReferences.data();
		subpass.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
		if (depthReference.layout != VK_IMAGE_LAYOUT_UNDEFINED)
			subpass.pDepthStencilAttachment = &depthReference;
		//
		// Add external dependencies to ensure synchronization outside the render pass
		dependencies.push_back(
			plvk::subpassDependency(VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
									VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
									VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT));
		//
		// Add subpass dependencies for other stages if needed...
		subPasses.push_back(subpass);
		//
		uint32_t viewMask = 0;
		VkRenderPassMultiviewCreateInfo renderPassMultiviewCI{};
		std::vector<int32_t> viewOffsets(dependencies.size(), 0);
		void* next = nullptr;
		viewMask |= 1u << mMultiViewCount;
		viewMask -= 1;
		renderPassMultiviewCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO;
		renderPassMultiviewCI.subpassCount = 1;
		renderPassMultiviewCI.pViewMasks = &viewMask;
		renderPassMultiviewCI.correlationMaskCount = 0;
		renderPassMultiviewCI.pViewOffsets = viewOffsets.data();
		if (mMultiViewCount > 0) {
			next = &renderPassMultiviewCI;
		}
		//
		mRenderPass = VulkanRenderer::GetRenderer()->CreateRenderPass(attachmentDescs.data(), attachmentDescs.size(),
																	  subPasses.data(), subPasses.size(),
																	  dependencies.data(), dependencies.size(), next);
		//
		// Frame buffer
		// frameBufferAttachments.clear();
		// for (auto& a : mFramebufferAttachments) {
		//	frameBufferAttachments.push_back(static_cast<VulkanTexture*>(a->mTexture.get())->mImageView);
		//}
		if (frameBufferAttachments.size() > 0) {
			mFrameBuffer = VulkanRenderer::GetRenderer()->CreateFramebuffer(
				mRenderPass, biggestSize, frameBufferAttachments.data(), frameBufferAttachments.size(), 1);
		}
		else {
			PL_CORE_WARN(
				"Vulkan RenderGraph Compilation: frameBufferAttachments == 0, outputs cannot contain more than "
				"1 mip level, this can be ignored as it might be intentional");
		}
	}

	void VulkanRenderPass::Compile(PlazaRenderGraph* renderGraph) {
		// Build resources
		for (auto& [key, binding] : mTextures) {
			if (renderGraph->mCompiledBindings.find(binding->mTexture->mAssetName) ==
				renderGraph->mCompiledBindings.end())
				renderGraph->CompileTexture(binding, renderGraph->mCompiledBindings);
			renderGraph->mCompiledBindings.insert(binding->mTexture->mAssetName);
		}

		for (auto& [key, binding] : mSamplers) {
			if (renderGraph->mCompiledBindings.find(binding->mSampler->mName) == renderGraph->mCompiledBindings.end())
				renderGraph->CompileTextureSampler(binding, renderGraph->mCompiledBindings);
			renderGraph->mCompiledBindings.insert(binding->mSampler->mName);
		}

		for (auto& framebufferAttachment : mFramebufferAttachments) {
			if (renderGraph->mCompiledBindings.find(framebufferAttachment->mTexture->mAssetName) ==
				renderGraph->mCompiledBindings.end())
				renderGraph->CompileTexture(framebufferAttachment, renderGraph->mCompiledBindings);
			renderGraph->mCompiledBindings.insert(framebufferAttachment->mTexture->mAssetName);
		}

		for (auto& [key, binding] : mBuffers) {
			if (renderGraph->mCompiledBindings.find(binding->mBuffer->mName) == renderGraph->mCompiledBindings.end())
				renderGraph->CompileBuffer(binding->mBuffer, renderGraph->mCompiledBindings);
			renderGraph->mCompiledBindings.insert(binding->mBuffer->mName);
		}

		if (mRenderMethod != PL_RENDER_PASS_COMPUTE && mRenderMethod != PL_RENDER_PASS_HOLDER)
			CompileGraphics(renderGraph);

		// Build descriptor set layout
		std::vector<VkDescriptorSetLayoutBinding> descriptorSetBindings;
		std::vector<VkDescriptorBindingFlagsEXT> bindingFlags;

		descriptorSetBindings.reserve(mTextures.size() + mSamplers.size() + mBuffers.size());
		bindingFlags.reserve(mTextures.size() + mSamplers.size() + mBuffers.size());

		// Track if we have any variable descriptor count bindings
		bool hasVariableBinding = false;
		uint32_t maxVariableDescriptorCount = 0;
		uint32_t variableBindingIndex = 0;

		// Process texture bindings
		for (const auto& [key, texture] : mTextures) {
			if (mResourcesInfo.find(texture->mName) == mResourcesInfo.end())
				continue;

			VkDescriptorSetLayoutBinding layoutBinding = {};
			layoutBinding.binding = texture->mBinding;
			layoutBinding.descriptorType = mResourcesInfo[texture->mName].descriptorType;
			layoutBinding.stageFlags = mResourcesInfo[texture->mName].stageFlags;
			layoutBinding.pImmutableSamplers = nullptr;

			VkDescriptorBindingFlagsEXT flags = 0;

			if (texture->mDescriptorCount > 1) {
				layoutBinding.descriptorCount = texture->mDescriptorCount;
				flags = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT |
						VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;
				hasVariableBinding = true;
				maxVariableDescriptorCount =
					std::max(maxVariableDescriptorCount, static_cast<uint32_t>(texture->mDescriptorCount));
				variableBindingIndex = static_cast<uint32_t>(descriptorSetBindings.size());
			}
			else {
				layoutBinding.descriptorCount = FRAMES_IN_FLIGHT;
			}

			descriptorSetBindings.push_back(layoutBinding);
			bindingFlags.push_back(flags);
		}

		// Process sampler bindings
		for (const auto& [key, sampler] : mSamplers) {
			if (mResourcesInfo.find(sampler->mName) == mResourcesInfo.end())
				continue;

			VkDescriptorSetLayoutBinding layoutBinding = {};
			layoutBinding.binding = sampler->mBinding;
			layoutBinding.descriptorType = mResourcesInfo[sampler->mName].descriptorType;
			layoutBinding.descriptorCount = FRAMES_IN_FLIGHT;
			layoutBinding.stageFlags = mResourcesInfo[sampler->mName].stageFlags;
			layoutBinding.pImmutableSamplers = nullptr;

			descriptorSetBindings.push_back(layoutBinding);
			bindingFlags.push_back(0);
		}

		// Process buffer bindings
		for (const auto& [key, buffer] : mBuffers) {
			if (mResourcesInfo.find(buffer->mName) == mResourcesInfo.end())
				continue;

			VkDescriptorSetLayoutBinding layoutBinding = {};
			layoutBinding.binding = buffer->mBinding;
			layoutBinding.descriptorType = mResourcesInfo[buffer->mName].descriptorType;
			layoutBinding.stageFlags = mResourcesInfo[buffer->mName].stageFlags;
			layoutBinding.pImmutableSamplers = nullptr;

			VkDescriptorBindingFlagsEXT flags = 0;

			if (buffer->mDescriptorCount > 1) {
				layoutBinding.descriptorCount = buffer->mDescriptorCount;
				flags = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT |
						VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;
				hasVariableBinding = true;
				maxVariableDescriptorCount =
					std::max(maxVariableDescriptorCount, static_cast<uint32_t>(buffer->mDescriptorCount));
				variableBindingIndex = static_cast<uint32_t>(descriptorSetBindings.size());
			}
			else {
				layoutBinding.descriptorCount = FRAMES_IN_FLIGHT;
			}

			descriptorSetBindings.push_back(layoutBinding);
			bindingFlags.push_back(flags);
		}

		// Create descriptor set layout
		VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extendedInfo = {};
		extendedInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
		extendedInfo.pNext = nullptr;
		extendedInfo.pBindingFlags = bindingFlags.data();
		extendedInfo.bindingCount = static_cast<uint32_t>(bindingFlags.size());

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = plvk::descriptorSetLayoutCreateInfo(
			descriptorSetBindings, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT, &extendedInfo);

		if (mDescriptorSetLayout == VK_NULL_HANDLE) {
			vkCreateDescriptorSetLayout(VulkanRenderer::GetRenderer()->mDevice, &descriptorSetLayoutCreateInfo, nullptr,
										&mDescriptorSetLayout);
		}

		// Allocate descriptor sets
		const uint32_t maxFramesInFlight = Application::Get()->mRenderer->mMaxFramesInFlight;
		std::vector<VkDescriptorSetLayout> layouts(maxFramesInFlight, mDescriptorSetLayout);

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = VulkanRenderer::GetRenderer()->mDescriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
		allocInfo.pSetLayouts = layouts.data();

		// Only set variable descriptor count if we have variable bindings
		VkDescriptorSetVariableDescriptorCountAllocateInfoEXT countInfo = {};
		std::vector<uint32_t> variableDescriptorCounts;

		if (hasVariableBinding) {
			variableDescriptorCounts.resize(maxFramesInFlight, maxVariableDescriptorCount);

			countInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT;
			countInfo.pNext = nullptr;
			countInfo.descriptorSetCount = maxFramesInFlight;
			countInfo.pDescriptorCounts = variableDescriptorCounts.data();

			allocInfo.pNext = &countInfo;
		}

		mDescriptorSets.resize(maxFramesInFlight);
		VkResult res =
			vkAllocateDescriptorSets(VulkanRenderer::GetRenderer()->mDevice, &allocInfo, mDescriptorSets.data());
		if (res != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate descriptor sets!");
		}

		// Update descriptor sets
		for (uint32_t frameIndex = 0; frameIndex < maxFramesInFlight; ++frameIndex) {
			const size_t expectedBindings = mTextures.size() + mSamplers.size() + mBuffers.size();

			std::vector<std::vector<VkDescriptorImageInfo>> tempImageInfos;
			std::vector<std::vector<VkDescriptorBufferInfo>> tempBufferInfos;
			std::vector<VkWriteDescriptorSet> descriptorWrites;

			tempImageInfos.reserve(expectedBindings);
			tempBufferInfos.reserve(expectedBindings);
			descriptorWrites.reserve(expectedBindings);

			// Update texture descriptors
			for (const auto& [key, texture] : mTextures) {
				if (mResourcesInfo.find(texture->mName) == mResourcesInfo.end())
					continue;

				std::vector<VkDescriptorImageInfo> imageInfosForBinding;
				const uint32_t descriptorCount =
					texture->mDescriptorCount > 1 ? texture->mDescriptorCount : FRAMES_IN_FLIGHT;
				imageInfosForBinding.reserve(descriptorCount);

				for (uint32_t j = 0; j < descriptorCount; ++j) {
					VkDescriptorImageInfo imageInfo = {};
					imageInfo.imageView = static_cast<VulkanTexture*>(texture->mTexture.get())->mImageView;
					imageInfo.sampler = static_cast<VulkanTexture*>(texture->mTexture.get())->mSampler;
					imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					imageInfosForBinding.push_back(imageInfo);
				}

				if (imageInfosForBinding.empty() || imageInfosForBinding[0].imageView == VK_NULL_HANDLE) {
					PL_CORE_WARN("Invalid image view for texture binding: {}", texture->mName);
					continue;
				}

				tempImageInfos.push_back(std::move(imageInfosForBinding));

				VkWriteDescriptorSet descriptorWrite = {};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = mDescriptorSets[frameIndex];
				descriptorWrite.dstBinding = texture->mBinding;
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorType = mResourcesInfo[texture->mName].descriptorType;
				descriptorWrite.descriptorCount = static_cast<uint32_t>(tempImageInfos.back().size());
				descriptorWrite.pImageInfo = tempImageInfos.back().data();

				descriptorWrites.push_back(descriptorWrite);
			}

			// Update sampler descriptors
			for (const auto& [key, sampler] : mSamplers) {
				if (mResourcesInfo.find(sampler->mName) == mResourcesInfo.end())
					continue;

				std::vector<VkDescriptorImageInfo> imageInfosForBinding;
				imageInfosForBinding.reserve(Application::Get()->mRenderer->mMaxFramesInFlight);

				for (uint32_t j = 0; j < Application::Get()->mRenderer->mMaxFramesInFlight; ++j) {
					VkDescriptorImageInfo imageInfo = {};
					imageInfo.imageView = VK_NULL_HANDLE;
					imageInfo.sampler = static_cast<VulkanTextureSampler*>(sampler->mSampler.get())->mSampler;
					imageInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
					imageInfosForBinding.push_back(imageInfo);
				}

				tempImageInfos.push_back(std::move(imageInfosForBinding));

				VkWriteDescriptorSet descriptorWrite = {};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = mDescriptorSets[frameIndex];
				descriptorWrite.dstBinding = sampler->mBinding;
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
				descriptorWrite.descriptorCount = static_cast<uint32_t>(tempImageInfos.back().size());
				descriptorWrite.pImageInfo = tempImageInfos.back().data();

				descriptorWrites.push_back(descriptorWrite);
			}

			// Update buffer descriptors
			for (const auto& [key, buffer] : mBuffers) {
				if (mResourcesInfo.find(buffer->mName) == mResourcesInfo.end())
					continue;

				std::vector<VkDescriptorBufferInfo> bufferInfosForBinding;
				const uint32_t descriptorCount =
					buffer->mDescriptorCount > 1 ? buffer->mDescriptorCount : FRAMES_IN_FLIGHT;
				bufferInfosForBinding.reserve(descriptorCount);

				for (uint32_t j = 0; j < descriptorCount; ++j) {
					VkDescriptorBufferInfo bufferInfo = {};
					bufferInfo.buffer = static_cast<PlVkBuffer*>(buffer->mBuffer.get())
											->GetBuffer(buffer->mDescriptorCount > 1 ? j : frameIndex);
					bufferInfo.offset = 0;
					bufferInfo.range = (buffer->mBufferType == PL_BUFFER_UNIFORM_BUFFER) ? 64 : VK_WHOLE_SIZE;
					bufferInfosForBinding.push_back(bufferInfo);
				}

				tempBufferInfos.push_back(std::move(bufferInfosForBinding));

				VkWriteDescriptorSet descriptorWrite = {};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = mDescriptorSets[frameIndex];
				descriptorWrite.dstBinding = buffer->mBinding;
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorType = mResourcesInfo[buffer->mName].descriptorType;
				descriptorWrite.descriptorCount = static_cast<uint32_t>(tempBufferInfos.back().size());
				descriptorWrite.pBufferInfo = tempBufferInfos.back().data();

				descriptorWrites.push_back(descriptorWrite);
			}

			if (!descriptorWrites.empty()) {
				vkUpdateDescriptorSets(VulkanRenderer::GetRenderer()->mDevice,
									   static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0,
									   nullptr);
			}
		}

		// Compile pipelines
		for (auto& pipeline : mPipelines) {
			if (!pipeline->mCompiled) {
				this->CompilePipeline(pipeline);
			}
			else {
				VulkanPlazaPipeline* vkPipeline = static_cast<VulkanPlazaPipeline*>(pipeline.get());
				mDescriptorSetLayout = vkPipeline->mShaders->mDescriptorSetLayout != VK_NULL_HANDLE
										   ? vkPipeline->mShaders->mDescriptorSetLayout
										   : vkPipeline->mComputeShaders->mComputeDescriptorSetLayout;
			}
		}

		// Compile child passes
		for (auto& pass : mChildPasses) {
			static_cast<VulkanRenderPass*>(pass.get())->mDescriptorSetLayout = mDescriptorSetLayout;
			pass->Compile(renderGraph);
		}
	}

	void VulkanRenderPass::ReflectPass(PlazaRenderGraph* renderGraph) {
		VulkanRenderGraph* vkRenderGraph = static_cast<VulkanRenderGraph*>(renderGraph);
		if (this->mBaseShaderPath.empty())
			PL_CORE_WARN("Vulkan RenderGraph Compilation: Base shader path is empty");

		// Reflect shader data
		mReflectedShaders = ShaderReflection::GetShadersFromHlsl(this->mBaseShaderPath);
		for (ShaderReflection::Shader& shader : mReflectedShaders) {
			std::filesystem::path compiledPath = ShaderReflection::CompileHlsl(
				this->mBaseShaderPath, shader.mEntryName, shader.mShaderType, "", shader.mExtensions);
			shader.mShadersData.push_back(ShaderReflection::ReadSpirVBinary(compiledPath));
			ShaderReflection::ReflectShaderBindings(shader, ShaderReflection::ReadSpirVBinary(compiledPath),
													shader.mShaderType);
		}

		for (ShaderReflection::Shader& shader : mReflectedShaders) {
			PL_CORE_INFO(shader.mEntryName);
			for (ShaderReflection::ReflectedBinding& reflectedBinding : shader.mReflectedBindings) {
				// Set the associated texture or buffer to ReflectedBinding
				if (this->mTextures.find(reflectedBinding.name) != this->mTextures.end()) {
					reflectedBinding.texture = mTextures.at(reflectedBinding.name)->mTexture;
					mTextures[reflectedBinding.name]->mBinding = reflectedBinding.binding;
					mTextures[reflectedBinding.name]->mLocation = reflectedBinding.binding;
					mTextures[reflectedBinding.name]->mDescriptorCount =
						reflectedBinding.texture->GetTextureInfo().mDescriptorCount;
					// mTextures[reflectedBinding.texture->mAssetName]->mLocation = reflectedBinding.loca;
				}
				if (this->mBuffers.find(reflectedBinding.name) != this->mBuffers.end()) {
					reflectedBinding.buffer = mBuffers.at(reflectedBinding.name)->mBuffer;
					mBuffers[reflectedBinding.name]->mBinding = reflectedBinding.binding;
					mBuffers[reflectedBinding.name]->mLocation = reflectedBinding.binding;
					mBuffers[reflectedBinding.name]->mDescriptorCount = reflectedBinding.descriptorCount;
				}
				if (this->mSamplers.find(reflectedBinding.name) != this->mSamplers.end()) {
					reflectedBinding.sampler = mSamplers.at(reflectedBinding.name)->mSampler;
					mSamplers[reflectedBinding.name]->mBinding = reflectedBinding.binding;
					mSamplers[reflectedBinding.name]->mLocation = reflectedBinding.binding;
					mSamplers[reflectedBinding.name]->mDescriptorCount = reflectedBinding.descriptorCount;
				}

				// Get the framebuffer texture
				if (reflectedBinding.name.starts_with("out.var.SV_Target")) {
					const std::string key = "out.var.SV_Target";
					size_t pos = reflectedBinding.name.find(key);

					if (pos != std::string::npos) {
						// Grab substring after "Target"
						std::string numberStr = reflectedBinding.name.substr(pos + key.size());

						if (!numberStr.empty()) {
							int attachmentIndex = std::stoi(numberStr);
							mFramebufferAttachments[attachmentIndex]->mLocation = attachmentIndex;
							reflectedBinding.texture = mFramebufferAttachments[attachmentIndex]->mTexture;
							mFramebufferAttachments[attachmentIndex]->mInitialLayout =
								GetReflectedBindingOptimalLayout(reflectedBinding);
						}
					}
				}

				// Set resource info, this will later be used to build the textures/buffers and descriptor sets
				VulkanResourceInfo resourceInfo{};
				if (reflectedBinding.reflectedType == ShaderReflection::PL_REFLECTED_TYPE_UNIFORM_BUFFER) {
					resourceInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					resourceInfo.stageFlags = PlRenderStageToVkShaderStage(
						ShaderReflection::ShaderTypeToPlRenderStageFlags(shader.mShaderType));
				}
				else if (reflectedBinding.reflectedType == ShaderReflection::PL_REFLECTED_TYPE_STORAGE_BUFFER) {
					resourceInfo.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
					resourceInfo.stageFlags = PlRenderStageToVkShaderStage(
						ShaderReflection::ShaderTypeToPlRenderStageFlags(shader.mShaderType));
				}
				else if (reflectedBinding.reflectedType == ShaderReflection::PL_REFLECTED_TYPE_SAMPLED_IMAGE) {
					resourceInfo.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
					resourceInfo.stageFlags = PlRenderStageToVkShaderStage(
						ShaderReflection::ShaderTypeToPlRenderStageFlags(shader.mShaderType));
				}
				else if (reflectedBinding.reflectedType == ShaderReflection::PL_REFLECTED_TYPE_SEPARATE_IMAGE) {
					resourceInfo.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
					resourceInfo.stageFlags = PlRenderStageToVkShaderStage(
						ShaderReflection::ShaderTypeToPlRenderStageFlags(shader.mShaderType));
				}
				else if (reflectedBinding.reflectedType == ShaderReflection::PL_REFLECTED_TYPE_SEPARATE_SAMPLER) {
					resourceInfo.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
					resourceInfo.stageFlags = PlRenderStageToVkShaderStage(
						ShaderReflection::ShaderTypeToPlRenderStageFlags(shader.mShaderType));
				}
				else if (reflectedBinding.reflectedType == ShaderReflection::PL_REFLECTED_TYPE_STORAGE_IMAGE) {
					resourceInfo.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
					resourceInfo.stageFlags = PlRenderStageToVkShaderStage(
						ShaderReflection::ShaderTypeToPlRenderStageFlags(shader.mShaderType));
				}
				else {
					PL_CORE_WARN("Vulkan RenderGraph Compilation: Unsupported resource type");
				}

				vkRenderGraph->mResourcesInfo[reflectedBinding.name] = resourceInfo;
				mResourcesInfo[reflectedBinding.name] = resourceInfo;

				// Get the texture associated with the shader binding, the association is written in the node creation
				std::shared_ptr<PlazaShadersBinding> binding = nullptr;
				if (reflectedBinding.texture != nullptr) {
					binding = std::make_shared<PlazaTextureBinding>();
					static_cast<PlazaTextureBinding*>(binding.get())->mTexture = reflectedBinding.texture;
					static_cast<PlazaTextureBinding*>(binding.get())->mInitialLayout =
						GetReflectedBindingOptimalLayout(reflectedBinding);
				}
				else if (reflectedBinding.buffer != nullptr) {
					binding = std::make_shared<PlazaBufferBinding>();
					static_cast<PlazaBufferBinding*>(binding.get())->mBuffer = reflectedBinding.buffer;
				}
				else if (reflectedBinding.sampler != nullptr) {
					binding = std::make_shared<PlazaTextureSamplerBinding>();
					static_cast<PlazaTextureSamplerBinding*>(binding.get())->mSampler = reflectedBinding.sampler;
				}
				else {
					PL_CORE_WARN("Could not find any slot with given name: {}", reflectedBinding.name);
					continue;
				}

				binding->mName = reflectedBinding.name;
				binding->mLocation = reflectedBinding.binding;
				binding->mBinding = reflectedBinding.binding;
				if (reflectedBinding.write == true) {
					AddOutputResource(binding);
				}
				else {
					AddInputResource(binding);
				}
			}

			// Reflect pipeline
			if (shader.mShaderType == ShaderReflection::PL_VERTEX_SHADER) {
			}
		}
	}

	void VulkanRenderPass::GetBindingDescriptorSet(const shared_ptr<PlazaShadersBinding>& binding,
												   std::vector<VkDescriptorSetLayoutBinding>& descriptorSets,
												   std::vector<VkDescriptorBindingFlagsEXT>& bindingFlags) {
		// TODO: DELETE THIS FUNCTION
	}
	void VulkanRenderPass::GetBindingWriteInfo(const shared_ptr<PlazaShadersBinding>& binding, unsigned int i,
											   std::vector<VkWriteDescriptorSet>& descriptorWrites,
											   std::vector<VkDescriptorBufferInfo*>& bufferInfos,
											   std::vector<VkDescriptorImageInfo*>& imageInfos) {
		// TODO: DELETE THIS FUNCTION
	}

	void VulkanRenderPass::BindMainBuffers() {
		VkDeviceSize offsets[1] = {0};
		vkCmdBindVertexBuffers(mCommandBuffer, 0, 1,
							   mRenderMethod == PL_RENDER_PASS_INDIRECT_BUFFER_SKINNED
								   ? &VulkanRenderer::GetRenderer()->mSkinnedVertexBuffer->GetBuffer()
								   : &VulkanRenderer::GetRenderer()->mMainVertexBuffer->GetBuffer(),
							   offsets);
		vkCmdBindVertexBuffers(
			mCommandBuffer, 1, 1,
			&VulkanRenderer::GetRenderer()->mMainInstanceMatrixBuffers[VulkanRenderer::GetRenderer()->mCurrentFrame],
			offsets);
		vkCmdBindIndexBuffer(mCommandBuffer, VulkanRenderer::GetRenderer()->mMainIndexBuffer->GetBuffer(), 0,
							 VK_INDEX_TYPE_UINT32);
	}

	void VulkanRenderPass::BindPipelineBuffers(PlazaPipeline* pipeline) {
		VulkanPlazaPipeline* vulkanPipeline = static_cast<VulkanPlazaPipeline*>(pipeline);

		VkDeviceSize offsets[1] = {0};
		for (std::shared_ptr<PlBufferAttachment> attachment : vulkanPipeline->mVertexBuffers) {
			vkCmdBindVertexBuffers(mCommandBuffer, attachment->mLocation, 1,
								   &static_cast<PlVkBuffer*>(attachment->mBuffer.get())
										->GetBuffer(VulkanRenderer::GetRenderer()->mCurrentFrame),
								   offsets);
		}

		if (vulkanPipeline->mIndexBuffer)
			vkCmdBindIndexBuffer(mCommandBuffer,
								 static_cast<PlVkBuffer*>(vulkanPipeline->mIndexBuffer->mBuffer.get())->GetBuffer(), 0,
								 VK_INDEX_TYPE_UINT32);
	}

	void VulkanRenderPass::BindRenderPass() {
		VkRenderPassBeginInfo renderPassInfo =
			plvk::renderPassBeginInfo(this->mRenderPass, this->mFrameBuffer, mRenderSize.x, mRenderSize.y, 0, 0,
									  mClearValues.size(), mClearValues.data());
		vkCmdBeginRenderPass(mCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = plvk::viewport(0.0f, mFlipViewPort ? mRenderSize.y : 0.0f, mRenderSize.x,
											 mFlipViewPort ? -static_cast<float>(mRenderSize.y) : mRenderSize.y);
		vkCmdSetViewport(mCommandBuffer, 0, 1, &viewport);
		VkRect2D scissor = plvk::rect2D(0, 0, mRenderSize.x, mRenderSize.y);
		vkCmdSetScissor(mCommandBuffer, 0, 1, &scissor);
	}

	void VulkanRenderPass::EndRenderPass() { vkCmdEndRenderPass(mCommandBuffer); }

	bool VulkanRenderGraph::BindPass(std::string passName) {
		if (mPasses.find(passName) == mPasses.end())
			return false;
		VulkanRenderPass* renderPass = this->GetRenderPass(passName);

		renderPass->BindRenderPass();
	}

	PlazaRenderPass* VulkanRenderGraph::AddRenderPass(const std::string& name, int stage, PlRenderPassMode renderMethod,
													  glm::vec2 size, bool flipViewPort) {
		std::shared_ptr<VulkanRenderPass> newRenderPass =
			std::make_shared<VulkanRenderPass>(name, stage, renderMethod, size, flipViewPort);
		mOrderedPasses.push_back(newRenderPass);
		mPasses.emplace(newRenderPass->mName, newRenderPass);
		return mPasses[newRenderPass->mName].get();
	}

	void VulkanRenderGraph::AddTexture(uint64_t descriptorCount, PlImageUsage imageUsage, PlTextureType imageType,
									   PlViewType viewType, PlTextureFormat format, glm::vec3 resolution,
									   uint8_t mipCount, uint16_t layersCount, const std::string& name) {
		mTextures.emplace(name, std::make_shared<VulkanTexture>(descriptorCount, imageUsage, imageType, viewType,
																format, resolution, mipCount, layersCount, name));
	}

	void VulkanRenderGraph::AddBuffer(PlBufferType type, uint64_t maxItems, uint16_t stride, uint8_t bufferCount,
									  PlBufferUsage bufferUsage, PlMemoryUsage memoryUsage, const std::string& name) {
		mBuffers.emplace(
			name, std::make_shared<PlVkBuffer>(type, maxItems, stride, bufferCount, bufferUsage, memoryUsage, name));
	}

	void VulkanRenderGraph::AddSampler(const std::string& name, PlFilter magFilter, PlFilter minFilter,
									   PlSamplerAddressMode addressModeU, PlSamplerAddressMode addressModeV,
									   PlSamplerAddressMode addressModeW, bool useAnisotropy, float maxAnisotropy,
									   PlBorderColor borderColor, bool useUnnormalizedCoordinates, bool useCompare,
									   PlCompareOp compareOp, PlSamplerMipmapMode mipmapMode, float mipLodBias,
									   float minLod, float maxLod) {
		mTextureSamplers.emplace(name, std::make_shared<VulkanTextureSampler>(
										   name, magFilter, minFilter, addressModeU, addressModeV, addressModeW,
										   useAnisotropy, maxAnisotropy, borderColor, useUnnormalizedCoordinates,
										   useCompare, compareOp, mipmapMode, mipLodBias, minLod, maxLod));
	}

	PlazaRenderPass* VulkanRenderPass::AddChildPass(const std::string& name, int stage, PlRenderPassMode renderMethod,
													glm::vec2 size, bool flipViewPort) {
		std::shared_ptr<VulkanRenderPass> pass =
			std::make_shared<VulkanRenderPass>(name, stage, renderMethod, size, flipViewPort);
		mChildPasses.push_back(pass);
		return pass.get();
	}

#pragma region Rendering

	void VulkanRenderGraph::Execute(Scene* scene, uint8_t imageIndex, uint8_t currentFrame) {
		PLAZA_PROFILE_SECTION("Execute RenderGraph");
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		VkCommandBuffer commandBuffer = *mCommandBuffer;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}
		VulkanRenderer::GetRenderer()->mActiveCommandBuffer = &commandBuffer;
		// VulkanRenderer::GetRenderer()->TransitionImageLayout(VulkanRenderer::GetRenderer()->mSwapChainImages[currentFrame],
		// VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

		for (unsigned int i = 0; i < mOrderedPasses.size(); ++i) {
			this->GetRenderPass(mOrderedPasses[i]->mName)->UpdateCommandBuffer(commandBuffer);
			for (const auto& child : mOrderedPasses[i]->mChildPasses) {
				static_cast<VulkanRenderPass*>(child.get())->UpdateCommandBuffer(commandBuffer);
			}
#ifdef GAME_MODE
			if (i == mOrderedPasses.size() - 1) {
				static_cast<VulkanRenderPass*>(mOrderedPasses[i].get())->mFrameBuffer =
					VulkanRenderer::GetRenderer()->mSwapChainFramebuffers[imageIndex];
				static_cast<VulkanRenderPass*>(mOrderedPasses[i].get())->mRenderPass =
					VulkanRenderer::GetRenderer()->mSwapchainRenderPass;
			}
#endif
			this->GetRenderPass(mOrderedPasses[i]->mName)->Execute(scene, this);
		}

		/* Render ImGui if in Editor build */
#ifdef EDITOR_MODE
		std::array<VkClearValue, 1> clearValues{};
		clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
		VkRenderPassBeginInfo renderPassInfo = plvk::renderPassBeginInfo(
			VulkanRenderer::GetRenderer()->mSwapchainRenderPass,
			VulkanRenderer::GetRenderer()->mSwapChainFramebuffers[imageIndex],
			VulkanRenderer::GetRenderer()->mSwapChainExtent.width,
			VulkanRenderer::GetRenderer()->mSwapChainExtent.height, 0, 0, 1, clearValues.data());

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		// vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		// VulkanRenderer::GetRenderer()->mGraphicsPipeline);

		VkViewport viewport = plvk::viewport(0.0f, 0.0f, VulkanRenderer::GetRenderer()->mSwapChainExtent.width,
											 VulkanRenderer::GetRenderer()->mSwapChainExtent.height);
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		VkRect2D scissor = plvk::rect2D(0, 0, VulkanRenderer::GetRenderer()->mSwapChainExtent.width,
										VulkanRenderer::GetRenderer()->mSwapChainExtent.height);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		{
			PLAZA_PROFILE_SECTION("Render ImGui");
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
		}

		vkCmdEndRenderPass(commandBuffer);

#endif

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	void VulkanRenderPass::RenderIndirectBuffer(PlazaPipeline* pipeline) {
		VulkanPlazaPipeline* vulkanPipeline = static_cast<VulkanPlazaPipeline*>(pipeline);
		vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->mShaders->mPipeline);
		vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
								vulkanPipeline->mShaders->mPipelineLayout, 0, 1,
								&mDescriptorSets[VulkanRenderer::GetRenderer()->mCurrentFrame], 0, nullptr);
		for (const PlPushConstants& pushConstant : vulkanPipeline->mPushConstants) {
			vkCmdPushConstants(mCommandBuffer, vulkanPipeline->mShaders->mPipelineLayout,
							   PlRenderStageToVkShaderStage(pushConstant.mStage), pushConstant.mOffset,
							   pushConstant.mStride, pushConstant.mData);
		}

		for (const std::shared_ptr<PlBufferAttachment>& buffer : vulkanPipeline->mVertexBuffers) {
			VkDeviceSize offsets[1] = {0};
			vkCmdBindVertexBuffers(mCommandBuffer, buffer->mLocation, 1,
								   &static_cast<PlVkBuffer*>(buffer->mBuffer.get())
										->GetBuffer(VulkanRenderer::GetRenderer()->mCurrentFrame),
								   offsets);
		}

		vkCmdDrawIndexedIndirect(
			mCommandBuffer,
			VulkanRenderer::GetRenderer()->mIndirectBuffers[VulkanRenderer::GetRenderer()->mCurrentFrame],
			pipeline->mIndirectBufferOffset, VulkanRenderer::GetRenderer()->mIndirectDrawCount,
			sizeof(VkDrawIndexedIndirectCommand));
	}

	void VulkanRenderPass::RenderIndirectBufferShadowMap(PlazaPipeline* pipeline) {
		VulkanPlazaPipeline* vulkanPipeline = static_cast<VulkanPlazaPipeline*>(pipeline);
		vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->mShaders->mPipeline);
		vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
								vulkanPipeline->mShaders->mPipelineLayout, 0, 1,
								&mDescriptorSets[VulkanRenderer::GetRenderer()->mCurrentFrame], 0, nullptr);
		for (const PlPushConstants& pushConstant : vulkanPipeline->mPushConstants) {
			vkCmdPushConstants(mCommandBuffer, vulkanPipeline->mShaders->mPipelineLayout,
							   PlRenderStageToVkShaderStage(pushConstant.mStage), pushConstant.mOffset,
							   pushConstant.mStride, pushConstant.mData);
		}
		vkCmdDrawIndexedIndirect(
			mCommandBuffer,
			VulkanRenderer::GetRenderer()->mIndirectBuffers[VulkanRenderer::GetRenderer()->mCurrentFrame],
			pipeline->mIndirectBufferOffset, VulkanRenderer::GetRenderer()->mIndirectDrawCount,
			sizeof(VkDrawIndexedIndirectCommand));
	}

	void VulkanRenderPass::RenderIndirectBufferSpecificEntity(PlazaPipeline* pipeline) {
		VulkanPlazaPipeline* vulkanPipeline = static_cast<VulkanPlazaPipeline*>(pipeline);
		vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->mShaders->mPipeline);
		vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
								vulkanPipeline->mShaders->mPipelineLayout, 0, 1,
								&mDescriptorSets[VulkanRenderer::GetRenderer()->mCurrentFrame], 0, nullptr);
		for (const PlPushConstants& pushConstant : pipeline->mPushConstants) {
			vkCmdPushConstants(mCommandBuffer, vulkanPipeline->mShaders->mPipelineLayout,
							   PlRenderStageToVkShaderStage(pushConstant.mStage), pushConstant.mOffset,
							   pushConstant.mStride, pushConstant.mData);
		}

		for (const std::shared_ptr<PlBufferAttachment>& buffer : vulkanPipeline->mVertexBuffers) {
			VkDeviceSize offsets[1] = {0};
			vkCmdBindVertexBuffers(mCommandBuffer, buffer->mLocation, 1,
								   &static_cast<PlVkBuffer*>(buffer->mBuffer.get())
										->GetBuffer(VulkanRenderer::GetRenderer()->mCurrentFrame),
								   offsets);
		}

		if (pipeline->mIndirectBuffer && pipeline->mIndirectBuffer->mCurrentItemCount > 0) {
			vkCmdDrawIndexedIndirect(mCommandBuffer,
									 static_cast<PlVkBuffer*>(pipeline->mIndirectBuffer.get())
										 ->GetBuffer(VulkanRenderer::GetRenderer()->mCurrentFrame),
									 pipeline->mIndirectBufferOffset, pipeline->mIndirectBuffer->mCurrentItemCount,
									 sizeof(VkDrawIndexedIndirectCommand));
		}
	}

	void VulkanRenderPass::RenderIndirectBufferSpecificMesh(PlazaPipeline* pipeline) {
		VulkanPlazaPipeline* vulkanPipeline = static_cast<VulkanPlazaPipeline*>(pipeline);
		vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->mShaders->mPipeline);
		vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
								vulkanPipeline->mShaders->mPipelineLayout, 0, 1,
								&mDescriptorSets[VulkanRenderer::GetRenderer()->mCurrentFrame], 0, nullptr);
		for (const PlPushConstants& pushConstant : pipeline->mPushConstants) {
			vkCmdPushConstants(mCommandBuffer, vulkanPipeline->mShaders->mPipelineLayout,
							   PlRenderStageToVkShaderStage(pushConstant.mStage), pushConstant.mOffset,
							   pushConstant.mStride, pushConstant.mData);
		}
		for (auto& meshUuid : pipeline->mCreateInfo.specificUuids) {
			Mesh* mesh = AssetsManager::GetMesh(meshUuid);
			if (!mesh)
				continue;
			vkCmdDrawIndexed(mCommandBuffer, static_cast<uint32_t>(mesh->indices.size()), 1, mesh->indicesOffset,
							 mesh->verticesOffset, mesh->instanceOffset);
		}
	}

	void VulkanRenderPass::RenderIndirectBufferSkinned(PlazaPipeline* pipeline) {
		// VulkanPlazaPipeline* vulkanPipeline = static_cast<VulkanPlazaPipeline*>(pipeline);
		// vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->mShaders->mPipeline);
		// vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		// vulkanPipeline->mShaders->mPipelineLayout, 0, 1,
		// &mDescriptorSets[VulkanRenderer::GetRenderer()->mCurrentFrame], 0, nullptr); for (const PlPushConstants&
		// pushConstant : vulkanPipeline->mPushConstants) { 	vkCmdPushConstants(mCommandBuffer,
		// vulkanPipeline->mShaders->mPipelineLayout, PlRenderStageToVkShaderStage(pushConstant.mStage),
		// pushConstant.mOffset, pushConstant.mStride, pushConstant.mData);
		// }
		// vkCmdDrawIndexedIndirect(mCommandBuffer,
		// VulkanRenderer::GetRenderer()->mIndirectBuffers[VulkanRenderer::GetRenderer()->mCurrentFrame],
		// pipeline->mIndirectBufferOffset, VulkanRenderer::GetRenderer()->mIndirectDrawCount,
		// sizeof(VkDrawIndexedIndirectCommand));
	}

	void VulkanRenderPass::RenderFullScreenQuad(PlazaPipeline* pipeline) {
		VulkanPlazaPipeline* vulkanPipeline = static_cast<VulkanPlazaPipeline*>(pipeline);
		vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->mShaders->mPipeline);
		vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
								vulkanPipeline->mShaders->mPipelineLayout, 0, 1,
								&mDescriptorSets[VulkanRenderer::GetRenderer()->mCurrentFrame], 0, nullptr);
		for (const PlPushConstants& pushConstant : vulkanPipeline->mPushConstants) {
			vkCmdPushConstants(mCommandBuffer, vulkanPipeline->mShaders->mPipelineLayout,
							   PlRenderStageToVkShaderStage(pushConstant.mStage), pushConstant.mOffset,
							   pushConstant.mStride, pushConstant.mData);
		}
		vkCmdDraw(mCommandBuffer, 3, 1, 0, 0);
	}

	void VulkanRenderPass::RenderCube(PlazaPipeline* pipeline) {
		VulkanPlazaPipeline* vulkanPipeline = static_cast<VulkanPlazaPipeline*>(pipeline);
		vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->mShaders->mPipeline);
		vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
								vulkanPipeline->mShaders->mPipelineLayout, 0, 1,
								&mDescriptorSets[VulkanRenderer::GetRenderer()->mCurrentFrame], 0, nullptr);
		for (const PlPushConstants& pushConstant : vulkanPipeline->mPushConstants) {
			vkCmdPushConstants(mCommandBuffer, vulkanPipeline->mShaders->mPipelineLayout,
							   PlRenderStageToVkShaderStage(pushConstant.mStage), pushConstant.mOffset,
							   pushConstant.mStride, pushConstant.mData);
		}
		vkCmdDraw(mCommandBuffer, 36, 1, 0, 0);
	}

	void VulkanRenderPass::RunCompute(PlazaPipeline* pipeline) {
		// TODO: FIX RUN COMPUTE TO WORK WITH NEW RENDER GRAPH
		//  TODO: WHY THIS MANUALLY GETS THE BLOOM TEXTURE???

		VulkanPlazaPipeline* vulkanPipeline = static_cast<VulkanPlazaPipeline*>(pipeline);
		VkPipelineLayout pipelineLayout = vulkanPipeline->mComputeShaders->mComputePipelineLayout;

		// VkCommandBuffer commandBuffer = VulkanRenderer::GetRenderer()->BeginSingleTimeCommands();
		vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
						  vulkanPipeline->mComputeShaders->mComputePipeline);

		vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
								vulkanPipeline->mComputeShaders->mComputePipelineLayout, 0, 1,
								&mDescriptorSets[VulkanRenderer::GetRenderer()->mCurrentFrame], 0, nullptr);

		for (const PlPushConstants& pushConstant : vulkanPipeline->mPushConstants) {
			vkCmdPushConstants(mCommandBuffer, vulkanPipeline->mComputeShaders->mComputePipelineLayout,
							   VK_SHADER_STAGE_COMPUTE_BIT, pushConstant.mOffset, pushConstant.mStride,
							   pushConstant.mData);
		}
		vkCmdDispatch(mCommandBuffer, mDispatchSize.x, mDispatchSize.y, mDispatchSize.z);

		// if (this->GetInputResource<PlazaTextureBinding>("BloomTexture")) {
		//	VkImageMemoryBarrier imageMemoryBarrier = {};
		//	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		//	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
		//	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
		//	imageMemoryBarrier.image =
		//		this->GetInputResource<VulkanTextureBinding>("BloomTexture")->GetTexture()->mImage;
		//	imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		//	imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
		//	imageMemoryBarrier.subresourceRange.levelCount =
		//		this->GetInputResource<VulkanTextureBinding>("BloomTexture")->mTexture->mMipCount;
		//	imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
		//	imageMemoryBarrier.subresourceRange.layerCount = 1;
		//
		//	imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		//	imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		//	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		//	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		//
		//	vkCmdPipelineBarrier(mCommandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		//						 VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
		//
		//  VulkanRenderer::GetRenderer()->EndSingleTimeCommands(commandBuffer);
	}

	void VulkanRenderPass::RenderGui(Scene* scene, PlazaPipeline* pipeline) {
		VulkanPlazaPipeline* vulkanPipeline = static_cast<VulkanPlazaPipeline*>(pipeline);
		vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->mShaders->mPipeline);
		vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
								vulkanPipeline->mShaders->mPipelineLayout, 0, 1,
								&mDescriptorSets[VulkanRenderer::GetRenderer()->mCurrentFrame], 0, nullptr);
		for (const PlPushConstants& pushConstant : pipeline->mPushConstants) {
			vkCmdPushConstants(mCommandBuffer, vulkanPipeline->mShaders->mPipelineLayout,
							   PlRenderStageToVkShaderStage(pushConstant.mStage), pushConstant.mOffset,
							   pushConstant.mStride, pushConstant.mData);
		}
		std::vector<glm::mat4> rectanglesTransform = std::vector<glm::mat4>();
		std::vector<glm::mat4> buttonsTransform = std::vector<glm::mat4>();
		std::vector<glm::mat4> textsTransform = std::vector<glm::mat4>();
		std::vector<glm::mat4>* vector;
		for (const uint64_t& uuid : SceneView<GuiComponent>(scene)) {
			GuiComponent& component = *scene->GetComponent<GuiComponent>(uuid);
			for (auto& [itemUuid, item] : component.mGuiItems) {
				switch (item->mGuiType) {
					case GuiType::PL_GUI_RECTANGLE:
						vector = &rectanglesTransform;
						break;
					case GuiType::PL_GUI_BUTTON:
						vector = &buttonsTransform;
						break;
					case GuiType::PL_GUI_TEXT:
						vector = &buttonsTransform;
						break;
				}
				vector->push_back(item->mTransform);
			}
		}

		PlVkBuffer* buffer =
			VulkanRenderer::GetRenderer()->mRenderGraph->GetBuffer<PlVkBuffer>("RectanglesTransformBuffer");
		Mesh* meshe = AssetsManager::GetMesh(1);
		VulkanRenderer::GetRenderer()->mInstanceModelMatrices.resize(
			VulkanRenderer::GetRenderer()->mInstanceModelMatrices.size() + rectanglesTransform.size());
		if (rectanglesTransform.size() > 0) {
			for (unsigned int i = 0; i < rectanglesTransform.size(); ++i) {
				VulkanRenderer::GetRenderer()->mInstanceModelMatrices[meshe->instanceOffset + i] =
					rectanglesTransform[i];
			}
		}
		void* data;
		size_t bufferSize = sizeof(glm::mat4) * VulkanRenderer::GetRenderer()->mInstanceModelMatrices.size();
		vmaMapMemory(VulkanRenderer::GetRenderer()->mVmaAllocator,
					 buffer->GetAllocation(VulkanRenderer::GetRenderer()->mCurrentFrame), &data);
		memcpy(data, VulkanRenderer::GetRenderer()->mInstanceModelMatrices.data(), bufferSize);
		vmaUnmapMemory(VulkanRenderer::GetRenderer()->mVmaAllocator,
					   buffer->GetAllocation(VulkanRenderer::GetRenderer()->mCurrentFrame));

		VkDeviceSize offsets[1] = {0};
		vkCmdBindVertexBuffers(mCommandBuffer, 1, 1, &buffer->GetBuffer(Application::Get()->mRenderer->mCurrentFrame),
							   offsets);
		for (auto& meshUuid : pipeline->mCreateInfo.specificUuids) {
			Mesh* mesh = AssetsManager::GetMesh(meshUuid);
			if (!mesh)
				continue;
			vkCmdDrawIndexed(mCommandBuffer, static_cast<uint32_t>(mesh->indices.size()),
							 glm::max<int>(rectanglesTransform.size(), 1), mesh->indicesOffset, mesh->verticesOffset,
							 mesh->instanceOffset);
		}
	}

	void VulkanRenderPass::RenderGuiRectangle(Scene* scene, PlazaPipeline* pipeline) {
		VulkanPlazaPipeline* vulkanPipeline = static_cast<VulkanPlazaPipeline*>(pipeline);
		vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->mShaders->mPipeline);
		vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
								vulkanPipeline->mShaders->mPipelineLayout, 0, 1,
								&mDescriptorSets[VulkanRenderer::GetRenderer()->mCurrentFrame], 0, nullptr);
		for (const PlPushConstants& pushConstant : pipeline->mPushConstants) {
			vkCmdPushConstants(mCommandBuffer, vulkanPipeline->mShaders->mPipelineLayout,
							   PlRenderStageToVkShaderStage(pushConstant.mStage), pushConstant.mOffset,
							   pushConstant.mStride, pushConstant.mData);
		}

		std::vector<glm::mat4> rectanglesTransform = std::vector<glm::mat4>();
		std::vector<glm::mat4> buttonsTransform = std::vector<glm::mat4>();
		std::vector<glm::mat4> textsTransform = std::vector<glm::mat4>();
		std::vector<glm::mat4>* vector;
		int id0 = scene->GetComponentId<TransformComponent>();
		int id1 = scene->GetComponentId<AudioListener>();
		int id2 = scene->GetComponentId<MeshRenderer>();
		int id3 = scene->GetComponentId<GuiComponent>();
		int id4 = scene->GetComponentId<Collider>();
		int id5 = scene->GetComponentId<RigidBody>();
		int id6 = scene->GetComponentId<AudioSource>();
		for (uint64_t uuid : SceneView<GuiComponent>(scene)) {
			GuiComponent& component = *scene->GetComponent<GuiComponent>(uuid);
			if (!component.mEnabled)
				continue;
			for (auto& [itemUuid, item] : component.mGuiItems) {
				switch (item->mGuiType) {
					case GuiType::PL_GUI_RECTANGLE:
						vector = &rectanglesTransform;
						break;
					case GuiType::PL_GUI_BUTTON:
						vector = &buttonsTransform;
						break;
					case GuiType::PL_GUI_TEXT:
						vector = &buttonsTransform;
						break;
				}
				vector->push_back(item->mTransform);
			}
		}

		PlVkBuffer* buffer =
			VulkanRenderer::GetRenderer()->mRenderGraph->GetBuffer<PlVkBuffer>("RectanglesTransformBuffer");
		Mesh* meshe = AssetsManager::GetMesh(1);
		VulkanRenderer::GetRenderer()->mInstanceModelMatrices.resize(
			VulkanRenderer::GetRenderer()->mInstanceModelMatrices.size() + rectanglesTransform.size());
		if (rectanglesTransform.size() > 0) {
			for (unsigned int i = 0; i < rectanglesTransform.size(); ++i) {
				VulkanRenderer::GetRenderer()->mInstanceModelMatrices[meshe->instanceOffset + i] =
					rectanglesTransform[i];
			}
		}
		void* data;
		size_t bufferSize = sizeof(glm::mat4) * VulkanRenderer::GetRenderer()->mInstanceModelMatrices.size();
		vmaMapMemory(VulkanRenderer::GetRenderer()->mVmaAllocator,
					 buffer->GetAllocation(VulkanRenderer::GetRenderer()->mCurrentFrame), &data);
		memcpy(data, VulkanRenderer::GetRenderer()->mInstanceModelMatrices.data(), bufferSize);
		vmaUnmapMemory(VulkanRenderer::GetRenderer()->mVmaAllocator,
					   buffer->GetAllocation(VulkanRenderer::GetRenderer()->mCurrentFrame));

		VkDeviceSize offsets[1] = {0};
		vkCmdBindVertexBuffers(mCommandBuffer, 1, 1, &buffer->GetBuffer(Application::Get()->mRenderer->mCurrentFrame),
							   offsets);
		for (auto& meshUuid : pipeline->mCreateInfo.specificUuids) {
			Mesh* mesh = AssetsManager::GetMesh(meshUuid);
			if (!mesh)
				mesh = AssetsManager::GetMesh(1);
			vkCmdDrawIndexed(mCommandBuffer, static_cast<uint32_t>(mesh->indices.size()),
							 glm::max<int>(rectanglesTransform.size(), 1), mesh->indicesOffset, mesh->verticesOffset,
							 mesh->instanceOffset);
		}
	}

	void VulkanRenderPass::RenderGuiButton(Scene* scene, PlazaPipeline* pipeline) {
		VulkanPlazaPipeline* vulkanPipeline = static_cast<VulkanPlazaPipeline*>(pipeline);
		vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->mShaders->mPipeline);
		vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
								vulkanPipeline->mShaders->mPipelineLayout, 0, 1,
								&mDescriptorSets[VulkanRenderer::GetRenderer()->mCurrentFrame], 0, nullptr);

		// for (auto& [key, value] : Scene::GetActiveScene()->guiComponents) {
		// for (auto& [itemUuid, item] : value.mGuiItems) {
		// if (item->mGuiType != GuiType::PL_GUI_BUTTON)
		//	continue;
		//
		// PlVkBuffer* textsBuffer =
		// VulkanRenderer::GetRenderer()->mRenderGraph->GetBuffer<PlVkBuffer>("GuiTextVerticesBuffer"); glm::vec4*
		// mapped; VkCommandBuffer cmdBuffer = mCommandBuffer; vkMapMemory(VulkanRenderer::GetRenderer()->mDevice,
		// textsBuffer->GetMemory(VulkanRenderer::GetRenderer()->mCurrentFrame), 0, VK_WHOLE_SIZE, 0, (void**)&mapped);
		// int numLetters = 0;
		//
		// GuiButton* button = static_cast<GuiButton*>(item.get());
		////for (auto& [key, value] : Scene::GetActiveScene()->UITextRendererComponents) {
		// static_cast<VulkanGuiRenderer*>(VulkanRenderer::GetRenderer()->mGuiRenderer)->AddText(button->mText,
		// button->GetPosition().x, button->GetPosition().y, button->mTextScale,
		// VulkanGuiRenderer::TextAlign::alignLeft, mapped, &numLetters);
		////}
		//
		// vkUnmapMemory(VulkanRenderer::GetRenderer()->mDevice,
		// textsBuffer->GetMemory(VulkanRenderer::GetRenderer()->mCurrentFrame)); mapped = nullptr;
		//
		// VkDeviceSize offsets = 0;
		// vkCmdBindVertexBuffers(cmdBuffer, 0, 1,
		// &textsBuffer->GetBuffer(VulkanRenderer::GetRenderer()->mCurrentFrame), &offsets);
		// vkCmdBindVertexBuffers(cmdBuffer, 1, 1,
		// &textsBuffer->GetBuffer(VulkanRenderer::GetRenderer()->mCurrentFrame), &offsets);
		//
		//// One draw command for every character. This is okay for a debug overlay, but not optimal
		//// In a real-world application one would try to batch draw commands
		// for (uint32_t j = 0; j < numLetters; j++) {
		//	vkCmdDraw(cmdBuffer, 4, 1, j * 4, 0);
		// }
		//}
		//}
	}

	void VulkanRenderPass::RenderGuiText(Scene* scene, PlazaPipeline* pipeline) {
		VulkanPlazaPipeline* vulkanPipeline = static_cast<VulkanPlazaPipeline*>(pipeline);
		vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->mShaders->mPipeline);
		vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
								vulkanPipeline->mShaders->mPipelineLayout, 0, 1,
								&mDescriptorSets[VulkanRenderer::GetRenderer()->mCurrentFrame], 0, nullptr);

		PlVkBuffer* textsBuffer =
			VulkanRenderer::GetRenderer()->mRenderGraph->GetBuffer<PlVkBuffer>("GuiTextVerticesBuffer");

		glm::vec4* mapped = nullptr;
		std::vector<glm::vec4> data = std::vector<glm::vec4>();
		int count = 0;
		int numLetters = 0;
		vmaMapMemory(VulkanRenderer::GetRenderer()->mVmaAllocator,
					 textsBuffer->GetAllocation(VulkanRenderer::GetRenderer()->mCurrentFrame), (void**)&mapped);
		for (const uint64_t& uuid : SceneView<GuiComponent>(scene)) {
			GuiComponent& value = *scene->GetComponent<GuiComponent>(uuid);
			if (!value.mEnabled)
				continue;

			for (auto& [itemUuid, item] : value.mGuiItems) {
				if (item->mGuiType != GuiType::PL_GUI_BUTTON)
					continue;

				GuiButton* button = static_cast<GuiButton*>(item.get());
				VulkanGuiRenderer::AddText(button->mText, button->GetWorldPosition().x, button->GetWorldPosition().y,
										   button->mTextScale, VulkanGuiRenderer::TextAlign::alignCenter, mapped,
										   numLetters);
				count++;
			}
		}
		vmaUnmapMemory(VulkanRenderer::GetRenderer()->mVmaAllocator,
					   textsBuffer->GetAllocation(VulkanRenderer::GetRenderer()->mCurrentFrame));
		mapped = nullptr;
		VkDeviceSize offsets = 0;
		vkCmdBindVertexBuffers(mCommandBuffer, 0, 1,
							   &textsBuffer->GetBuffer(VulkanRenderer::GetRenderer()->mCurrentFrame), &offsets);
		vkCmdBindVertexBuffers(mCommandBuffer, 1, 1,
							   &textsBuffer->GetBuffer(VulkanRenderer::GetRenderer()->mCurrentFrame), &offsets);

		// One draw command for every character. This is okay for a debug overlay, but not optimal
		// In a real-world application one would try to batch draw commands
		for (uint32_t j = 0; j < numLetters; j++) {
			vkCmdDraw(mCommandBuffer, 4, 1, j * 4, 0);
		}
	}

	void VulkanRenderPass::RenderDebug(Scene* scene, PlazaPipeline* pipeline) {
		VulkanPlazaPipeline* vulkanPipeline = static_cast<VulkanPlazaPipeline*>(pipeline);
		vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->mShaders->mPipeline);
		vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
								vulkanPipeline->mShaders->mPipelineLayout, 0, 1,
								&mDescriptorSets[VulkanRenderer::GetRenderer()->mCurrentFrame], 0, nullptr);



		vkCmdDraw(mCommandBuffer, 2, Application::Get()->mRenderer->mDebugRenderer->mDebugLines.size(), 0, 0);
	}

#pragma endregion Rendering

	PlazaRenderPass* VulkanRenderPass::SetTexture(const std::string& slotName, std::shared_ptr<Texture> texture) {
		std::shared_ptr<PlazaTextureBinding> binding = std::make_shared<PlazaTextureBinding>();
		binding->mTexture = texture;
		binding->mName = slotName;
		binding->mResourceName = texture->mAssetName;
		mTextures.emplace(slotName, binding);
		mInputResourcesName.emplace(texture->mAssetName);
		return this;
	}

	PlazaRenderPass* VulkanRenderPass::SetSampler(const std::string& slotName,
												  std::shared_ptr<PlTextureSampler> sampler) {
		std::shared_ptr<PlazaTextureSamplerBinding> binding = std::make_shared<PlazaTextureSamplerBinding>();
		binding->mSampler = sampler;
		binding->mName = slotName;
		binding->mResourceName = sampler->mName;
		mSamplers.emplace(slotName, binding);
		mInputResourcesName.emplace(sampler->mName);
		return this;
	}

	PlazaRenderPass* VulkanRenderPass::SetBuffer(const std::string& slotName, std::shared_ptr<PlBuffer> buffer) {
		std::shared_ptr<PlazaBufferBinding> binding = std::make_shared<PlazaBufferBinding>();
		binding->mBuffer = buffer;
		binding->mName = slotName;
		binding->mResourceName = buffer->mName;
		mBuffers.emplace(slotName, binding);
		mInputResourcesName.emplace(buffer->mName);
		return this;
	}

	PlazaRenderPass* VulkanRenderPass::AddRenderTarget(std::shared_ptr<Texture> texture) {
		std::shared_ptr<PlazaTextureBinding> binding = std::make_shared<PlazaTextureBinding>();
		binding->mTexture = texture;
		binding->mName = texture->mAssetName;
		mFramebufferAttachments.push_back(binding);
		return this;
	}

	PlImageLayout VulkanRenderPass::GetReflectedBindingOptimalLayout(
		const ShaderReflection::ReflectedBinding& binding) {
		if (binding.texture == nullptr)
			return PL_IMAGE_LAYOUT_UNDEFINED;

		VkFormat format = PlImageFormatToVkFormat(binding.texture->GetTextureInfo().mFormat);
		if (binding.write && binding.read) {
			return PL_IMAGE_LAYOUT_GENERAL;
		}
		else if (binding.read) {
			if (VulkanRenderer::GetRenderer()->IsFormatDepthStencil(format))
				return PL_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			else if (VulkanRenderer::GetRenderer()->IsFormatDepth(format))
				return PL_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
			else if (VulkanRenderer::GetRenderer()->IsFormatStencil(format))
				return PL_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;

			return PL_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
		}
		else if (binding.write) {
			if (VulkanRenderer::GetRenderer()->IsFormatDepthStencil(format))
				return PL_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			else if (VulkanRenderer::GetRenderer()->IsFormatDepth(format))
				return PL_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
			else if (VulkanRenderer::GetRenderer()->IsFormatStencil(format))
				return PL_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;

			return PL_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
		}

		return PL_IMAGE_LAYOUT_UNDEFINED;
	}

} // namespace Plaza
