#pragma once
#include "PlazaShaders.h"
#include "Texture.h"
#include "Buffer.h"
#include "Engine/Core/Engine.h"
#include "PlazaPipeline.h"
#include "ShaderReflection.h"
#include "Engine/Core/Renderer/RendererTypes.h"
#include "Engine/Core/Renderer/Mesh.h"

namespace Plaza {
	class PlazaTextureSamplerBinding;
	class PlazaTextureBinding;
	class PlazaBufferBinding;
	class PlazaShadersBinding;
	class PlazaRenderGraph;
	class PLAZA_API PlazaRenderPass {
	  public:
		PlazaRenderPass() {}
		PlazaRenderPass(std::string name, int stage, PlRenderPassMode renderMethod, glm::vec2 size, bool flipViewPort)
			: mName(name), mStage(stage), mRenderMethod(renderMethod), mRenderSize(size), mFlipViewPort(flipViewPort) {}
		PlazaRenderPass(const PlazaRenderPass& other) = default;

		std::string mName = "";
		int16_t mExecutionIndex = 0;
		int mStage = 0;
		PlRenderPassMode mRenderMethod = PL_RENDER_PASS_FULL_SCREEN_QUAD;
		uint16_t mMultiViewCount = 0;
		glm::vec2 mRenderSize = glm::vec2(0, 0);
		glm::vec3 mDispatchSize = glm::vec3(0, 0, 0);
		bool mFlipViewPort = true;

		std::vector<std::shared_ptr<PlazaPipeline>> mPipelines = std::vector<std::shared_ptr<PlazaPipeline>>();
		std::vector<ShaderReflection::Shader> mReflectedShaders = std::vector<ShaderReflection::Shader>();

		std::map<std::string, std::shared_ptr<PlazaRenderPass>> mDependencies =
			std::map<std::string, std::shared_ptr<PlazaRenderPass>>();
		std::map<std::string, std::shared_ptr<PlazaRenderPass>> mDependents =
			std::map<std::string, std::shared_ptr<PlazaRenderPass>>();

		std::filesystem::path mBaseShaderPath;

		std::vector<shared_ptr<PlazaShadersBinding>> mInputBindings = std::vector<shared_ptr<PlazaShadersBinding>>();
		std::map<std::string, shared_ptr<PlazaShadersBinding>> mInputBindingNames =
			std::map<std::string, shared_ptr<PlazaShadersBinding>>();
		std::vector<shared_ptr<PlazaShadersBinding>> mOutputBindings = std::vector<shared_ptr<PlazaShadersBinding>>();
		std::map<std::string, shared_ptr<PlazaShadersBinding>> mOutputBindingNames =
			std::map<std::string, shared_ptr<PlazaShadersBinding>>();

		std::function<void(PlazaRenderGraph*, PlazaRenderPass*, Scene* scene)> mCallback = [](PlazaRenderGraph*, PlazaRenderPass*, Scene* scene) {};

		virtual void ReflectPass(PlazaRenderGraph* renderGraph) {};
		virtual void Compile(PlazaRenderGraph* renderGraph) {};
		virtual void Execute(Scene* scene, PlazaRenderGraph* renderGraph);
		virtual void BindMainBuffers() {};
		virtual void BindPipelineBuffers(PlazaPipeline* pipeline) {};
		virtual void BindRenderPass() {};
		virtual void EndRenderPass() {};

		virtual void RenderIndirectBuffer(PlazaPipeline* pipeline) {};
		virtual void RenderIndirectBufferShadowMap(PlazaPipeline* pipeline) {};
		virtual void RenderIndirectBufferSpecificEntity(PlazaPipeline* pipeline) {};
		virtual void RenderIndirectBufferSpecificMesh(PlazaPipeline* pipeline) {};
		virtual void RenderIndirectBufferSkinned(PlazaPipeline* pipeline) {};
		virtual void RenderFullScreenQuad(PlazaPipeline* pipeline) {};
		virtual void RenderCube(PlazaPipeline* pipeline) {};
		virtual void RunCompute(PlazaPipeline* pipeline) {};
		virtual void RenderGui(Scene* scene, PlazaPipeline* pipeline) {};
		virtual void RenderGuiRectangle(Scene* scene, PlazaPipeline* pipeline) {};
		virtual void RenderGuiButton(Scene* scene, PlazaPipeline* pipeline) {};
		virtual void RenderGuiText(Scene* scene, PlazaPipeline* pipeline) {};
		virtual void CompilePipeline(std::shared_ptr<PlazaPipeline> plazaPipeline) {};
		virtual void TerminatePipeline(std::shared_ptr<PlazaPipeline> plazaPipeline) {};
		virtual void ResetPipelineCompiledBool() {};
		virtual void ReCompileShaders(PlazaRenderGraph* graph, bool resetCompiledBool) {};

		std::shared_ptr<PlazaPipeline> AddPipeline(std::shared_ptr<PlazaPipeline> pipeline) {
			mPipelines.push_back(pipeline);
			return pipeline;
		};
		virtual std::shared_ptr<PlazaPipeline> AddPipeline(const PlPipelineCreateInfo& createInfo) { return nullptr; };

		void SetRecordingCallback(std::function<void(PlazaRenderGraph*, PlazaRenderPass*, Scene*)> callback) {
			mCallback = callback;
		}

		std::unordered_map<std::string, std::shared_ptr<PlazaTextureBinding>> mTextures;
		std::unordered_map<std::string, std::shared_ptr<PlazaBufferBinding>> mBuffers;
		std::unordered_map<std::string, std::shared_ptr<PlazaTextureSamplerBinding>> mSamplers;
		std::vector<std::shared_ptr<PlazaTextureBinding>> mFramebufferAttachments;

		/* Textures */
		virtual PlazaRenderPass* SetTexture(const std::string& slotName, std::shared_ptr<Texture> texture) = 0;
		virtual PlazaRenderPass* SetBuffer(const std::string& slotName, std::shared_ptr<PlBuffer> buffer) = 0;
		virtual PlazaRenderPass* SetSampler(const std::string& slotName, std::shared_ptr<PlTextureSampler> buffer) = 0;
		virtual PlazaRenderPass* AddRenderTarget(std::shared_ptr<Texture> texture) = 0;
		PlazaRenderPass* SetShader(std::filesystem::path shaderPath);
		PlazaRenderPass* SetMultiViewCount(int multiViewCount);

		template <typename T> T* GetInputResource(const std::string& name) {
			if (mInputBindingNames.find(name) == mInputBindingNames.end())
				return nullptr;
			return dynamic_cast<T*>(mInputBindingNames.at(name).get());
		}

		template <typename T> T* GetOutputResource(const std::string& name) {
			if (mOutputBindingNames.find(name) == mOutputBindingNames.end())
				return nullptr;
			return dynamic_cast<T*>(mOutputBindingNames.at(name).get());
		}

		virtual PlazaRenderPass* AddChildPass(const std::string& name, int stage, PlRenderPassMode renderMethod, glm::vec2 size, bool flipViewPort) = 0;
		PlazaRenderPass* AddChildPass(std::shared_ptr<PlazaRenderPass> pass);
		std::vector<std::shared_ptr<PlazaRenderPass>> mChildPasses = std::vector<std::shared_ptr<PlazaRenderPass>>();

		template <class Archive> void serialize(Archive& archive) {
			archive(PL_SER(mName), PL_SER(mStage), PL_SER(mRenderMethod), PL_SER(mMultiViewCount), PL_SER(mRenderSize),
					PL_SER(mDispatchSize), PL_SER(mFlipViewPort), PL_SER(mPipelines), PL_SER(mInputBindings),
					PL_SER(mInputBindingNames), PL_SER(mOutputBindings), PL_SER(mOutputBindingNames),
					PL_SER(mChildPasses));
		}

	  private:
		virtual void CompileGraphics(PlazaRenderGraph* renderGraph) {};

	protected:
		PlazaRenderPass* AddInputResource(std::shared_ptr<PlazaShadersBinding> resource);
		PlazaRenderPass* AddOutputResource(std::shared_ptr<PlazaShadersBinding> resource);
	};
}