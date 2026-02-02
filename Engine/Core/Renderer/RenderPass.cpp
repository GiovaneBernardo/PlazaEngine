#include "Engine/Core/PreCompiledHeaders.h"
#include "RenderGraph.h"
#include "RenderPass.h"
#include "ShaderReflection.h"

namespace Plaza {
	void PlazaRenderPass::Execute(Scene* scene, PlazaRenderGraph* renderGraph) {
		mCallback(renderGraph, this, scene);
		if (mRenderMethod == PL_RENDER_PASS_HOLDER) {
			for (auto& renderPass : mChildPasses) {
				renderPass->Execute(scene, renderGraph);
			}
			return;
		}

		if (mRenderMethod != PL_RENDER_PASS_COMPUTE) {
			this->BindRenderPass();
			this->BindMainBuffers();
		}

		for (auto& pipeline : mPipelines) {
			this->BindPipelineBuffers(pipeline.get());
			switch (pipeline->mCreateInfo.renderMethod) {
				case PL_RENDER_PASS_FULL_SCREEN_QUAD:
					this->RenderFullScreenQuad(pipeline.get());
					break;
				case PL_RENDER_PASS_INDIRECT_BUFFER:
					this->RenderIndirectBuffer(pipeline.get());
					break;
				case PL_RENDER_PASS_INDIRECT_BUFFER_SHADOW_MAP:
					this->RenderIndirectBufferShadowMap(pipeline.get());
					break;
				case PL_RENDER_PASS_INDIRECT_BUFFER_SPECIFIC_ENTITY:
					this->RenderIndirectBufferSpecificEntity(pipeline.get());
					break;
				case PL_RENDER_PASS_INDIRECT_BUFFER_SPECIFIC_MESH:
					this->RenderIndirectBufferSpecificMesh(pipeline.get());
					break;
				case PL_RENDER_PASS_INDIRECT_BUFFER_SKINNED:
					this->RenderIndirectBufferSkinned(pipeline.get());
					break;
				case PL_RENDER_PASS_CUBE:
					this->RenderCube(pipeline.get());
					break;
				case PL_RENDER_PASS_COMPUTE:
					this->RunCompute(pipeline.get());
					break;
				case PL_RENDER_PASS_GUI:
					this->RenderGui(scene, pipeline.get());
					break;
				case PL_RENDER_PASS_GUI_RECTANGLE:
					this->RenderGuiRectangle(scene, pipeline.get());
					break;
				case PL_RENDER_PASS_GUI_BUTTON:
					this->RenderGuiButton(scene, pipeline.get());
					break;
				case PL_RENDER_PASS_GUI_TEXT:
					this->RenderGuiText(scene, pipeline.get());
					break;
			}
		}

		for (auto& renderPass : mChildPasses) {
			renderPass->Execute(scene, renderGraph);
		}

		if (mRenderMethod != PL_RENDER_PASS_COMPUTE)
			this->EndRenderPass();
	};

	PlazaRenderPass* PlazaRenderPass::SetShader(std::filesystem::path shaderPath) {
		mBaseShaderPath = shaderPath;
		return this;
	}

	PlazaRenderPass* PlazaRenderPass::SetMultiViewCount(int multiViewCount) {
		mMultiViewCount = multiViewCount;
		return this;
	}

	PlazaRenderPass* PlazaRenderPass::AddChildPass(std::shared_ptr<PlazaRenderPass> pass) {
		mChildPasses.push_back(pass);
		return pass.get();
	}

	PlazaRenderPass* PlazaRenderPass::AddInputResource(std::shared_ptr<PlazaShadersBinding> resource) {
		mInputBindings.push_back(resource);
		mInputBindingNames.emplace(resource->mName, resource);
		return this;
	}

	PlazaRenderPass* PlazaRenderPass::AddOutputResource(std::shared_ptr<PlazaShadersBinding> resource) {
		mOutputBindings.push_back(resource);
		mOutputBindingNames.emplace(resource->mName, resource);
		return this;
	}
} // namespace Plaza
