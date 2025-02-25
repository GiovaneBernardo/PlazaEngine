#include "Engine/Core/PreCompiledHeaders.h"
#include "RenderGraphEditor.h"
#include "Engine/Application/FileDialog/FileDialog.h"
#include "Engine/Core/Renderer/Vulkan/VulkanRenderGraph.h"

#include <ThirdParty/pfr/include/boost/pfr/core.hpp>
#include <ThirdParty/pfr/include/boost/pfr/core_name.hpp>
#include <iostream>
#include <string>
#include "Engine/Application/FileDialog/FileDialog.h"
#include "Engine/Core/Renderer/RenderGraph.h"

namespace Plaza::Editor {
	void RenderGraphEditor::Init() {
		return;
		this->AddNodeToCreateByType<PlTextureType>("Render Graph / Enums");
		this->AddNodeToCreateByType<PlViewType>("Render Graph / Enums");
		this->AddNodeToCreateByType<PlBufferType>("Render Graph / Enums");
		this->AddNodeToCreateByType<PlTextureFormat>("Render Graph / Enums");
		this->AddNodeToCreateByType<PlImageUsage>("Render Graph / Enums");
		this->AddNodeToCreateByType<PlImageLayout>("Render Graph / Enums");
		this->AddNodeToCreateByType<PlRenderStage>("Render Graph / Enums");
		this->AddNodeToCreateByType<PlBindingType>("Render Graph / Enums");
		this->AddNodeToCreateByType<PlMemoryProperty>("Render Graph / Enums");
		this->AddNodeToCreateByType<PlBufferUsage>("Render Graph / Enums");
		this->AddNodeToCreateByType<PlMemoryUsage>("Render Graph / Enums");
		this->AddNodeToCreateByType<PlRenderPassMode>("Render Graph / Enums");
		this->AddNodeToCreateByType<PlPrimitiveTopology>("Render Graph / Enums");
		this->AddNodeToCreateByType<PlPolygonMode>("Render Graph / Enums");
		this->AddNodeToCreateByType<PlCullMode>("Render Graph / Enums");
		this->AddNodeToCreateByType<PlFrontFace>("Render Graph / Enums");
		this->AddNodeToCreateByType<PlSampleCount>("Render Graph / Enums");
		this->AddNodeToCreateByType<PlCompareOp>("Render Graph / Enums");
		this->AddNodeToCreateByType<PlBlendFactor>("Render Graph / Enums");
		this->AddNodeToCreateByType<PlBlendOp>("Render Graph / Enums");
		this->AddNodeToCreateByType<PlColorComponentFlags>("Render Graph / Enums");
		this->AddNodeToCreateByType<PlLogicOp>("Render Graph / Enums");
		this->AddNodeToCreateByType<PlStencilOp>("Render Graph / Enums");
		this->AddNodeToCreateByType<PlDynamicState>("Render Graph / Enums");
		this->AddNodeToCreateByType<PlVertexInputRate>("Render Graph / Enums");
		this->AddNodeToCreateByType<PlSamplerAddressMode>("Render Graph / Enums");

		this->AddNodeToCreateByType<PlPipelineRasterizationStateCreateInfo>("Render Graph / Structs");
		this->AddNodeToCreateByType<PlPipelineColorBlendAttachmentState>("Render Graph / Structs");
		this->AddNodeToCreateByType<PlPipelineColorBlendStateCreateInfo>("Render Graph / Structs");
		this->AddNodeToCreateByType<PlStencilOpState>("Render Graph / Structs");
		this->AddNodeToCreateByType<PlPipelineDepthStencilStateCreateInfo>("Render Graph / Structs");
		this->AddNodeToCreateByType<PlViewport>("Render Graph / Structs");
		this->AddNodeToCreateByType<PlRect2D>("Render Graph / Structs");
		this->AddNodeToCreateByType<PlPipelineViewportStateCreateInfo>("Render Graph / Structs");
		this->AddNodeToCreateByType<PlPipelineMultisampleStateCreateInfo>("Render Graph / Structs");
		this->AddNodeToCreateByType<PlVertexInputBindingDescription>("Render Graph / Structs");
		this->AddNodeToCreateByType<PlVertexInputAttributeDescription>("Render Graph / Structs");
		this->AddNodeToCreateByType<PlPipelineShaderStageCreateInfo>("Render Graph / Structs");
		this->AddNodeToCreateByType<PlPushConstantRange>("Render Graph / Structs");
		this->AddNodeToCreateByType<PlPipelineCreateInfo>("Render Graph / Structs");

		/* Texture */
		this->AddNodeToCreateByType<TextureNodeStruct>("Render Graph / Structs");
		this->AddNodeToCreateByType<BufferNodeStruct>("Render Graph / Structs");

		//Node textureNode = Node();
		//textureNode.name = typeid(Texture).name();
		//this->AddInputPin(textureNode, Pin(0, "Name", PinType::String, PinKind::Constant)).SetValue<std::string>("Texture");
		//this->AddOutputPin(textureNode, Pin(0, "Out", PinType::Object, PinKind::Output)).value.SetType<Texture>();
		//textureNode.processFunction = [](Node& node) {
		//
		//	};
		//this->AddNodeToCreate(textureNode);

		/* Texture binding */
		Node textureBindingNode = Node();
		textureBindingNode.name = typeid(PlazaTextureBinding).name();
		this->AddInputPin(textureBindingNode, Pin(0, "Descriptor Count", PinType::Int, PinKind::Constant)).SetValue<int>(0);
		this->AddInputPin(textureBindingNode, Pin(0, "Location", PinType::Int, PinKind::Constant)).SetValue<int>(0);
		this->AddInputPin(textureBindingNode, Pin(0, "Binding", PinType::Int, PinKind::Constant)).SetValue<int>(0);
		this->AddInputPin(textureBindingNode, Pin(0, "Buffer Type", PinType::Enum, PinKind::Constant)).SetEnumValue<PlBufferType>(0);
		this->AddInputPin(textureBindingNode, Pin(0, "Render Stage", PinType::Enum, PinKind::Constant)).SetEnumValue<PlRenderStage>(0);
		this->AddInputPin(textureBindingNode, Pin(0, "Initial Layout", PinType::Enum, PinKind::Constant)).SetEnumValue<PlImageLayout>(0);
		this->AddInputPin(textureBindingNode, Pin(0, "Base Mip Level", PinType::Int, PinKind::Constant)).SetValue<int>(0);
		this->AddInputPin(textureBindingNode, Pin(0, "Base Layer Level", PinType::Int, PinKind::Constant)).SetValue<int>(0);
		this->AddInputPin(textureBindingNode, Pin(0, "Resource Name", PinType::String, PinKind::Constant)).SetValue<std::string>("");
		this->AddOutputPin(textureBindingNode, Pin(0, "Out", PinType::Object, PinKind::Output)).value.SetType<PlazaTextureBinding>();

		textureBindingNode.processFunction = [](Node& node) {
			PlazaTextureBinding binding{};
			binding.mDescriptorCount = node.inputs[0].GetValue<int>();
			binding.mLocation = node.inputs[1].GetValue<int>();
			binding.mBinding = node.inputs[2].GetValue<int>();
			binding.mBufferType = node.inputs[3].GetValue<PlBufferType>();
			binding.mStage = node.inputs[4].GetValue<PlRenderStage>();
			binding.mInitialLayout = node.inputs[5].GetValue<PlImageLayout>();
			binding.mBaseMipLevel = node.inputs[6].GetValue<int>();
			binding.mBaseLayerLevel = node.inputs[7].GetValue<int>();
			binding.mBindingType = PlBindingType::PL_BINDING_TEXTURE;
			binding.mResourceName = node.inputs[8].GetValue<std::string>();
			node.outputs[0].SetValue(binding);
			};
		this->AddNodeToCreate(textureBindingNode);

		/* Buffer binding */
		Node bufferBindingNode = Node();
		bufferBindingNode.name = typeid(PlazaBufferBinding).name();
		this->AddInputPin(bufferBindingNode, Pin(0, "Descriptor Count", PinType::Int, PinKind::Constant)).SetValue<int>(0);
		this->AddInputPin(bufferBindingNode, Pin(0, "Binding", PinType::Int, PinKind::Constant)).SetValue<int>(0);
		this->AddInputPin(bufferBindingNode, Pin(0, "Render Stage", PinType::Enum, PinKind::Constant)).SetEnumValue<PlRenderStage>(0);
		this->AddInputPin(bufferBindingNode, Pin(0, "Resource Name", PinType::String, PinKind::Constant)).SetValue<std::string>("");
		this->AddOutputPin(bufferBindingNode, Pin(0, "Out", PinType::Object, PinKind::Output)).value.SetType<PlazaBufferBinding>();

		bufferBindingNode.processFunction = [](Node& node) {
			PlazaBufferBinding binding{};
			binding.mDescriptorCount = node.inputs[0].GetValue<int>();
			binding.mBinding = node.inputs[1].GetValue<int>();
			binding.mStage = node.inputs[2].GetValue<PlRenderStage>();
			binding.mBindingType = PlBindingType::PL_BINDING_BUFFER;
			binding.mResourceName = node.inputs[3].GetValue<std::string>();
			node.outputs[0].SetValue(binding);
			};
		this->AddNodeToCreate(bufferBindingNode);

		/* Render pass */
		Node renderPassNode = Node();
		renderPassNode.name = typeid(PlazaRenderPass).name();
		this->AddInputPin(renderPassNode, Pin(0, "Pass Name", PinType::String, PinKind::Constant)).SetValue<std::string>("Render Pass");
		this->AddInputPin(renderPassNode, Pin(0, "Stages", PinType::Enum, PinKind::Constant)).SetEnumValue<PlRenderStage>(0);
		this->AddInputPin(renderPassNode, Pin(0, "Render Mode", PinType::Enum, PinKind::Constant)).SetEnumValue<PlRenderPassMode>(0);
		this->AddInputPin(renderPassNode, Pin(0, "Multi view count", PinType::Int, PinKind::Constant)).SetValue<int>(0);
		this->AddInputPin(renderPassNode, Pin(0, "Render Size", PinType::Vector2, PinKind::Constant)).SetValue<glm::vec2>(glm::vec2(1920.0f, 1080.0f));
		this->AddInputPin(renderPassNode, Pin(0, "Dispatch Size", PinType::Vector3, PinKind::Constant)).SetValue<glm::vec3>(glm::vec3(0.0f));
		this->AddInputPin(renderPassNode, Pin(0, "Flip Viewport", PinType::BoolType, PinKind::Constant)).SetValue(true);
		this->AddInputPin(renderPassNode, Pin(0, "Input Textures", PinType::Array, PinKind::Input)).SetValue<PlazaTextureBinding>(PlazaTextureBinding());
		renderPassNode.inputs.back().isVector = true;
		this->AddInputPin(renderPassNode, Pin(0, "Input Buffers", PinType::Array, PinKind::Input)).SetValue<PlazaBufferBinding>(PlazaBufferBinding());
		renderPassNode.inputs.back().isVector = true;
		this->AddInputPin(renderPassNode, Pin(0, "Output Textures", PinType::Array, PinKind::Input)).SetValue<PlazaTextureBinding>(PlazaTextureBinding());
		renderPassNode.inputs.back().isVector = true;
		this->AddInputPin(renderPassNode, Pin(0, "Output Buffers", PinType::Array, PinKind::Input)).SetValue<PlazaBufferBinding>(PlazaBufferBinding());
		renderPassNode.inputs.back().isVector = true;
		this->AddInputPin(renderPassNode, Pin(0, "Pipelines", PinType::Array, PinKind::Input)).SetValue<PlPipelineCreateInfo>(PlPipelineCreateInfo());
		renderPassNode.inputs.back().isVector = true;
		this->AddOutputPin(renderPassNode, Pin(0, "Out", PinType::Object, PinKind::Output)).value.SetType<PlazaRenderPass>();
		this->AddOutputPin(renderPassNode, Pin(0, "Input Textures Out", PinType::Object, PinKind::Output)).value.SetType<PlazaTextureBinding>();
		this->AddOutputPin(renderPassNode, Pin(0, "Input Buffers Out", PinType::Object, PinKind::Output)).value.SetType<PlazaBufferBinding>();
		this->AddOutputPin(renderPassNode, Pin(0, "Output Textures Out", PinType::Object, PinKind::Output)).value.SetType<PlazaTextureBinding>();
		this->AddOutputPin(renderPassNode, Pin(0, "Output Buffers Out", PinType::Object, PinKind::Output)).value.SetType<PlazaBufferBinding>();
		this->AddOutputPin(renderPassNode, Pin(0, "Pipelines Out", PinType::Object, PinKind::Output)).value.SetType<PlazaRenderPass>();
		renderPassNode.processFunction = [](Node& node) {
			PlazaRenderPass pass{};
			pass.mName = node.inputs[0].GetValue<std::string>();
			pass.mStage = node.inputs[1].GetValue<PlRenderStage>();
			pass.mRenderMethod = node.inputs[2].GetValue<PlRenderPassMode>();
			pass.mMultiViewCount = node.inputs[3].GetValue<int>();
			pass.mRenderSize = node.inputs[4].GetValue<glm::vec2>();
			pass.mDispatchSize = node.inputs[5].GetValue<glm::vec3>();
			pass.mFlipViewPort = node.inputs[6].GetValue<bool>();
			node.outputs[0].SetValue(pass);

			std::vector<PlazaTextureBinding> inputTextures = std::vector<PlazaTextureBinding>();
			for (Node& subNode : node.inputs[7].subNodes) {
				subNode.processFunction(subNode);
				inputTextures.push_back(subNode.outputs[0].GetValue<PlazaTextureBinding>());
			}
			node.outputs[1].SetValue(inputTextures);

			std::vector<PlazaBufferBinding> inputBuffers = std::vector<PlazaBufferBinding>();
			for (Node& subNode : node.inputs[8].subNodes) {
				subNode.processFunction(subNode);
				inputBuffers.push_back(subNode.outputs[0].GetValue<PlazaBufferBinding>());
			}
			node.outputs[2].SetValue(inputBuffers);

			std::vector<PlazaTextureBinding> outputTextures = std::vector<PlazaTextureBinding>();
			for (Node& subNode : node.inputs[9].subNodes) {
				subNode.processFunction(subNode);
				outputTextures.push_back(subNode.outputs[0].GetValue<PlazaTextureBinding>());
			}
			node.outputs[3].SetValue(outputTextures);

			std::vector<PlazaBufferBinding> outputBuffers = std::vector<PlazaBufferBinding>();
			for (Node& subNode : node.inputs[10].subNodes) {
				subNode.processFunction(subNode);
				outputBuffers.push_back(subNode.outputs[0].GetValue<PlazaBufferBinding>());
			}
			node.outputs[4].SetValue(outputBuffers);

			std::vector<PlPipelineCreateInfo> pipelines = std::vector<PlPipelineCreateInfo>();
			for (Node& subNode : node.inputs[11].subNodes) {
				subNode.processFunction(subNode);
				pipelines.push_back(subNode.outputs[0].GetValue<PlPipelineCreateInfo>());
			}
			node.outputs[5].SetValue(pipelines);


			};
		this->AddNodeToCreate(renderPassNode);

		this->SetFinalNode<PlPipelineCreateInfo>();
		this->InitNodeEditor();
		//this->SpawnNode(typeid(PlPipelineCreateInfo).name()).inputs[0].SetValue<std::string>("Draw to SwapChain Pass");

	};

	void RenderGraphEditor::Process() {
		ax::NodeEditor::SetCurrentEditor(mContext);
		std::vector<uintptr_t> orderedNodeIds = this->GetOrderedNodes();
		std::set<uintptr_t> executedNodeIds = std::set<uintptr_t>();
		//orderedNodeIds.resize(static_cast<size_t>(nodeCount));
		//ax::NodeEditor::GetOrderedNodeIds(orderedNodeIds.data(), nodeCount);
		PlPipelineCreateInfo info{};
		//GetNode(mFinalNodeId)->outputs[0].SetValue<PlPipelineCreateInfo>(info);
		for (int i = orderedNodeIds.size() - 1; i >= 0; i--) {
			uintptr_t nodeId = orderedNodeIds[i];
			if (executedNodeIds.find(nodeId) != executedNodeIds.end() || GetNode(nodeId) == nullptr)
				continue;
			GetNode(nodeId)->Process();//.processFunction(mNodes.at(nodeId));
			executedNodeIds.emplace(nodeId);
			//auto it = std::find_if(mNodes.begin(), mNodes.end(), [&](const Node& node) {
			//	return node.id == nodeId.Get();
			//	});
			////if (it != mNodes.end())
			////	std::swap(*it, mNodes[i]);
		}
		ax::NodeEditor::SetCurrentEditor(nullptr);

		//for (auto& node : mNodes) {
		//	node.processFunction(node);
		//}
		//Node& finalNode = *GetNode(mFinalNodeId);
		//if (finalNode.inputs[0].nodes.size() > 1)
		//	info = finalNode.inputs[0].nodes[1]->outputs[0].GetValue<PlPipelineCreateInfo>();

		VulkanRenderGraph* renderGraph = new VulkanRenderGraph();

		for (auto& [key, node] : mNodesData.mNodes) {
			Any& value = node.outputs[0].value;
			if (value.type() == typeid(TextureNodeStruct)) {
				TextureNodeStruct texture = node.outputs[0].GetValue<TextureNodeStruct>();
				renderGraph->AddTexture(std::make_shared<VulkanTexture>(texture.descriptorCount, texture.imageUsage, texture.imageType, texture.viewType, texture.format, texture.resolution, texture.mipCount, texture.layersCount, texture.name));
			}
			else if (value.type() == typeid(BufferNodeStruct)) {
				BufferNodeStruct buffer = node.outputs[0].GetValue<BufferNodeStruct>();
				buffer.bufferCount = 2;
				renderGraph->AddBuffer(std::make_shared<PlVkBuffer>(buffer.type, buffer.maxItems, buffer.stride, buffer.bufferCount, buffer.bufferUsage, buffer.memoryUsage, buffer.name));
			}
		}

		for (auto& [key, node] : mNodesData.mNodes) {
			Any& value = node.outputs[0].value;
			if (value.type() == typeid(PlPipelineCreateInfo)) {
				//PlPipelineCreateInfo info = node.outputs[0].GetValue<PlPipelineCreateInfo>();
				//renderGraph->CreatePipeline(info);
			}
			else if (value.type() == typeid(PlazaRenderPass)) {
				PlazaRenderPass pass = node.outputs[0].GetValue<PlazaRenderPass>();
				//std::string name, int stage, PlRenderPassMode renderMethod, glm::vec2 size, bool flipViewPort) : PlazaRenderPass(name, stage, renderMethod, size, flipViewPort
				PlazaRenderPass* addedPass = renderGraph->AddRenderPass(std::make_shared<VulkanRenderPass>(pass.mName, pass.mStage, pass.mRenderMethod, pass.mRenderSize, pass.mFlipViewPort));

				std::vector<PlazaTextureBinding> inputTextures = node.outputs[1].GetValue<std::vector<PlazaTextureBinding>>();
				for (const PlazaTextureBinding& binding : inputTextures) {
					addedPass->AddInputResource(std::make_shared<VulkanTextureBinding>(
						binding.mDescriptorCount, binding.mLocation, binding.mBinding, binding.mBufferType, binding.mStage, binding.mInitialLayout, binding.mBaseMipLevel, binding.mBaseLayerLevel, renderGraph->GetSharedTexture(binding.mResourceName)));
				}
				std::vector<PlazaBufferBinding> inputBuffers = node.outputs[2].GetValue<std::vector<PlazaBufferBinding>>();
				for (const PlazaBufferBinding& binding : inputBuffers) {
					addedPass->AddInputResource(std::make_shared<VulkanBufferBinding>(binding.mDescriptorCount, binding.mBinding, binding.mBufferType, binding.mStage, renderGraph->GetSharedBuffer(binding.mResourceName)));
				}

				std::vector<PlazaTextureBinding> outputTextures = node.outputs[2].GetValue<std::vector<PlazaTextureBinding>>();
				for (const PlazaTextureBinding& binding : outputTextures) {
					addedPass->AddOutputResource(std::make_shared<VulkanTextureBinding>(
						binding.mDescriptorCount, binding.mLocation, binding.mBinding, binding.mBufferType, binding.mStage, binding.mInitialLayout, binding.mBaseMipLevel, binding.mBaseLayerLevel, renderGraph->GetSharedTexture(binding.mResourceName)));
				}
				std::vector<PlazaBufferBinding> outputBuffers = node.outputs[2].GetValue<std::vector<PlazaBufferBinding>>();
				for (const PlazaBufferBinding& binding : outputBuffers) {
					addedPass->AddOutputResource(std::make_shared<VulkanBufferBinding>(binding.mDescriptorCount, binding.mBinding, binding.mBufferType, binding.mStage, renderGraph->GetSharedBuffer(binding.mResourceName)));
				}

				std::vector<PlPipelineCreateInfo> pipelinesInfo = node.outputs[5].GetValue<std::vector<PlPipelineCreateInfo>>();
				for (PlPipelineCreateInfo& info : pipelinesInfo) {
					for (auto& stage : info.shaderStages) {
						if (stage.shadersPath == "mainShaders.vert")
							stage.shadersPath = "main1.vert";
						if (stage.shadersPath == "mainShaders.frag")
							stage.shadersPath = "main2.frag";
						stage.shadersPath = AssetsManager::GetShadersPath(stage.shadersPath);
					}
					addedPass->AddPipeline(info);
				}
			}

		}

		renderGraph->OrderPasses();
		renderGraph->UpdateUsedTexturesInfo();
		renderGraph->Compile();
		renderGraph->CompileNotBoundBuffers();

		Application::Get()->mRenderer->mRenderGraph = renderGraph;
		VulkanRenderer::GetRenderer()->mRenderGraph = renderGraph;
		Application::Get()->mRenderer->UpdateImGuiDisplayTexture(Application::Get()->mRenderer->mRenderGraph->GetTexture<VulkanTexture>("FinalTexture"));

		//AssetsSerializer::SerializeFile<VulkanRenderGraph>(*renderGraph, FileDialog::SaveFileDialog(""), Application::Get()->mSettings.mRenderGraphSerializationMode);

		PL_CORE_INFO("Alright");
	}

	void RenderGraphEditor::LoadRenderGraphNodes(PlazaRenderGraph* renderGraph) {
		PL_CORE_INFO("Loaded");
	}

	void RenderGraphEditor::SpawnLockedNodes() {
		/*
		struct BufferNodeStruct {
			PlBufferType type = PlBufferType::PL_BUFFER_UNIFORM_BUFFER;
			uint64_t maxItems = 0;
			uint16_t stride = 0;
			uint8_t bufferCount = 0;
			PlBufferUsage bufferUsage = PlBufferUsage::PL_BUFFER_USAGE_STORAGE_BUFFER;
			PlMemoryUsage memoryUsage = PlMemoryUsage::PL_MEMORY_USAGE_CPU_TO_GPU;
			std::string name = "";
			std::string resourceName = "";

			template <class Archive>
			void serialize(Archive& archive) {
				archive(PL_SER(type), PL_SER(maxItems), PL_SER(stride), PL_SER(bufferCount), PL_SER(bufferUsage), PL_SER(memoryUsage), PL_SER(name), PL_SER(resourceName));
			}
		};
		*/


		//Node& materialsBufferNode = this->SpawnNode(typeid(BufferNodeStruct).name());
		//materialsBufferNode.inputs[0].SetValue(PlBufferType::PL_BUFFER_STORAGE_BUFFER, false);
		//materialsBufferNode.inputs[1].SetValue<uint64_t>(1024 * 16, true);
		//materialsBufferNode.inputs[2].SetValue(sizeof(Material), false);
		//materialsBufferNode.inputs[3].SetValue(Application::Get()->mRenderer->mMaxFramesInFlight, false);
		//materialsBufferNode.inputs[4].SetValue(PL_BUFFER_USAGE_STORAGE_BUFFER, false);
		//materialsBufferNode.inputs[5].SetValue(PL_MEMORY_USAGE_CPU_TO_GPU, false);
		//materialsBufferNode.inputs[6].SetValue<std::string>("MaterialsBuffer", false);
		//materialsBufferNode.inputs[7].SetValue<std::string>("MaterialsBuffer", false);
	}
}