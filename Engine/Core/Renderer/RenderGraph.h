#pragma once 
#include "PlazaShaders.h"
#include "Texture.h"
#include "Buffer.h"
#include "Engine/Core/Engine.h"
#include "PlazaPipeline.h"
#include "Engine/Core/Renderer/RendererTypes.h"
#include "Engine/Core/Renderer/Mesh.h"

namespace Plaza {
	class PLAZA_API PlazaShadersBinding {
	public:
		PlazaShadersBinding() {}
		PlazaShadersBinding(const PlazaShadersBinding& other) = default;
		std::string mName = "";
		uint64_t mDescriptorCount = 1;
		PlRenderStage mStage = PL_STAGE_ALL;
		uint8_t mLocation = 0;
		uint8_t mBinding = 0;
		PlBindingType mBindingType = PlBindingType::PL_BINDING_UNDEFINED;
		uint64_t mMaxBindlessResources = 0;
		std::string mResourceName = "";
		bool mUseAsDepthStencilAttachment = false;

		virtual void Compile(std::set<std::string>& compiledBindings) {};
		virtual void Destroy() {};

		template <class Archive>
		void serialize(Archive& archive) {
			archive(PL_SER(mName), PL_SER(mDescriptorCount), PL_SER(mStage), PL_SER(mLocation), PL_SER(mBinding), PL_SER(mBindingType), PL_SER(mMaxBindlessResources));
		}
	};

	class PLAZA_API PlazaBufferBinding : public PlazaShadersBinding {
	public:
		PlazaBufferBinding() {}
		PlazaBufferBinding(const PlazaBufferBinding& other) = default;
		PlazaBufferBinding(uint64_t descriptorCount, uint8_t binding, PlBufferType type, PlRenderStage stage, std::shared_ptr<PlBuffer> buffer) {
			mBinding = binding;
			mDescriptorCount = descriptorCount;
			mStage = stage;
			mBindingType = PL_BINDING_BUFFER;
			mBuffer = buffer;
			mName = buffer->mName;
			mBufferType = type;
			//mStride = stride;
			//mMaxItems = maxItems;
			//mBufferCount = bufferCount;
		}
		PlBufferType mBufferType;

		std::shared_ptr<PlBuffer> mBuffer = nullptr;

		virtual void Compile() {};
		virtual void Destroy() {};

		//template<typename T>
		//T* SetMemoryProperties(uint32_t newProperties) {
		//	mMemoryProperties = static_cast<PlMemoryProperty>(newProperties);
		//	return dynamic_cast<T*>(this);
		//}
		//template<typename T>
		//T* SetBufferUsage(uint32_t newUsage) {
		//	mBufferUsage = static_cast<PlBufferUsage>(newUsage);
		//	return dynamic_cast<T*>(this);
		//}
		//template<typename T>
		//T* SetMemoryUsage(uint32_t newUsage) {
		//	mMemoryUsage = static_cast<PlMemoryUsage>(newUsage);
		//	return dynamic_cast<T*>(this);
		//}

		//PlMemoryProperty GetMemoryProperty() {
		//	return mMemoryProperties;
		//}
		//PlBufferUsage GetBufferUsage() {
		//	return mBufferUsage;
		//}
		//PlMemoryUsage GetMemoryUsage() {
		//	return mMemoryUsage;
		//}

		template <class Archive>
		void serialize(Archive& archive) {
			archive(cereal::base_class<PlazaShadersBinding>(this), PL_SER(mBuffer));
		}
	private:

	};

	class PLAZA_API PlazaTextureBinding : public PlazaShadersBinding {
	public:
		PlazaTextureBinding() {}
		PlazaTextureBinding(const PlazaTextureBinding& other) = default;
		PlazaTextureBinding(uint64_t descriptorCount, uint8_t location, uint8_t binding, PlBufferType bufferType, PlRenderStage renderStage, PlImageLayout initialLayout, uint16_t baseMipLevel,
			uint16_t baseLayerLevel, std::shared_ptr<Texture> texture, PlAttachmentOp attachmentOp = PL_ATTACHMENT_OP_AUTO, bool useAsDepthStencilAttachment = false)
			: mBufferType(bufferType) {
			mLocation = location;
			mBinding = binding;
			mDescriptorCount = descriptorCount;
			mBindingType = PL_BINDING_TEXTURE;
			mStage = renderStage;
			mTexture = texture;
			mInitialLayout = initialLayout;
			mBaseMipLevel = baseMipLevel;
			mBaseLayerLevel = baseLayerLevel;
			mUseAsDepthStencilAttachment = useAsDepthStencilAttachment;
			mAttachmentOp = attachmentOp;
		}

		PlBufferType mBufferType = PL_BUFFER_SAMPLER;
		PlImageLayout mInitialLayout = PL_IMAGE_LAYOUT_UNDEFINED;
		uint16_t mBaseMipLevel = 0;
		uint16_t mBaseLayerLevel = 0;
		PlAttachmentOp mAttachmentOp = PL_ATTACHMENT_OP_AUTO;

		virtual void Compile() {};
		virtual void Destroy() {};

		const TextureInfo& GetTextureInfo() {
			return mTexture->GetTextureInfo();
		}

		std::shared_ptr<Texture> mTexture = nullptr;

		template <class Archive>
		void serialize(Archive& archive) {
			archive(cereal::base_class<PlazaShadersBinding>(this), PL_SER(mBufferType), PL_SER(mInitialLayout), PL_SER(mBaseMipLevel), PL_SER(mBaseLayerLevel), PL_SER(mBaseLayerLevel), PL_SER(mTexture));
		}
	private:
	};

	class PlazaRenderGraph;
	class PLAZA_API PlazaRenderPass {
	public:
		PlazaRenderPass() {}
		PlazaRenderPass(std::string name, int stage, PlRenderPassMode renderMethod, glm::vec2 size, bool flipViewPort) : mName(name), mStage(stage), mRenderMethod(renderMethod), mRenderSize(size), mFlipViewPort(flipViewPort) {}
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

		std::map<std::string, std::shared_ptr<PlazaRenderPass>> mDependencies = std::map<std::string, std::shared_ptr<PlazaRenderPass>>();
		std::map<std::string, std::shared_ptr<PlazaRenderPass>> mDependents = std::map<std::string, std::shared_ptr<PlazaRenderPass>>();

		std::vector<shared_ptr<PlazaShadersBinding>> mInputBindings = std::vector<shared_ptr<PlazaShadersBinding>>();
		std::map<std::string, shared_ptr<PlazaShadersBinding>> mInputBindingNames = std::map<std::string, shared_ptr<PlazaShadersBinding>>();
		std::vector<shared_ptr<PlazaShadersBinding>> mOutputBindings = std::vector<shared_ptr<PlazaShadersBinding>>();
		std::map<std::string, shared_ptr<PlazaShadersBinding>> mOutputBindingNames = std::map<std::string, shared_ptr<PlazaShadersBinding>>();


		std::function<void(PlazaRenderGraph*, PlazaRenderPass*)> mCallback = [](PlazaRenderGraph*, PlazaRenderPass*) {};


		virtual void Compile(PlazaRenderGraph* renderGraph) {};
		virtual void Execute(Scene* scene, PlazaRenderGraph* renderGraph) {
			mCallback(renderGraph, this);
			if (mRenderMethod != PL_RENDER_PASS_HOLDER) {

				if (mRenderMethod != PL_RENDER_PASS_COMPUTE) {
					this->BindRenderPass();
					this->BindMainBuffers();
				}
				for (auto& pipeline : mPipelines) {
					switch (pipeline->mCreateInfo.renderMethod) {
					case PL_RENDER_PASS_FULL_SCREEN_QUAD: this->RenderFullScreenQuad(pipeline.get()); break;
					case PL_RENDER_PASS_INDIRECT_BUFFER: this->RenderIndirectBuffer(pipeline.get()); break;
					case PL_RENDER_PASS_INDIRECT_BUFFER_SHADOW_MAP: this->RenderIndirectBufferShadowMap(pipeline.get()); break;
					case PL_RENDER_PASS_INDIRECT_BUFFER_SPECIFIC_ENTITY: this->RenderIndirectBufferSpecificEntity(pipeline.get()); break;
					case PL_RENDER_PASS_INDIRECT_BUFFER_SPECIFIC_MESH: this->RenderIndirectBufferSpecificMesh(pipeline.get()); break;
					case PL_RENDER_PASS_INDIRECT_BUFFER_SKINNED: this->RenderIndirectBufferSkinned(pipeline.get()); break;
					case PL_RENDER_PASS_CUBE: this->RenderCube(pipeline.get()); break;
					case PL_RENDER_PASS_COMPUTE: this->RunCompute(pipeline.get()); break;
					case PL_RENDER_PASS_GUI: this->RenderGui(scene, pipeline.get()); break;
					case PL_RENDER_PASS_GUI_RECTANGLE: this->RenderGuiRectangle(scene, pipeline.get()); break;
					case PL_RENDER_PASS_GUI_BUTTON: this->RenderGuiButton(scene, pipeline.get()); break;
					case PL_RENDER_PASS_GUI_TEXT: this->RenderGuiText(scene, pipeline.get()); break;
					}
				}
			}

			for (auto& renderPass : mChildPasses) {
				renderPass->Execute(scene, renderGraph);
			}

			if (mRenderMethod != PL_RENDER_PASS_HOLDER && mRenderMethod != PL_RENDER_PASS_COMPUTE)
				this->EndRenderPass();
		};
		virtual void BindMainBuffers() {};
		virtual void BindRenderPass() {};
		virtual void EndRenderPass() {};

		virtual void RenderIndirectBuffer(PlazaPipeline* pipeline) { };
		virtual void RenderIndirectBufferShadowMap(PlazaPipeline* pipeline) { };
		virtual void RenderIndirectBufferSpecificEntity(PlazaPipeline* pipeline) {};
		virtual void RenderIndirectBufferSpecificMesh(PlazaPipeline* pipeline) {};
		virtual void RenderIndirectBufferSkinned(PlazaPipeline* pipeline) { };
		virtual void RenderFullScreenQuad(PlazaPipeline* pipeline) { };
		virtual void RenderCube(PlazaPipeline* pipeline) { };
		virtual void RunCompute(PlazaPipeline* pipeline) { };
		virtual void RenderGui(Scene* scene, PlazaPipeline* pipeline) { };
		virtual void RenderGuiRectangle(Scene* scene, PlazaPipeline* pipeline) { };
		virtual void RenderGuiButton(Scene* scene, PlazaPipeline* pipeline) { };
		virtual void RenderGuiText(Scene* scene, PlazaPipeline* pipeline) { };
		virtual void CompilePipeline(std::shared_ptr<PlazaPipeline> plazaPipeline) { };
		virtual void TerminatePipeline(std::shared_ptr<PlazaPipeline> plazaPipeline) { };
		virtual void ResetPipelineCompiledBool() { };
		virtual void ReCompileShaders(bool resetCompiledBool) { };


		std::shared_ptr<PlazaPipeline> AddPipeline(std::shared_ptr<PlazaPipeline> pipeline) {
			mPipelines.push_back(pipeline);
			return pipeline;
		};
		virtual std::shared_ptr<PlazaPipeline> AddPipeline(PlPipelineCreateInfo createInfo) { return nullptr; };

		void SetRecordingCallback(std::function<void(PlazaRenderGraph*, PlazaRenderPass*)> callback) {
			mCallback = callback;
		}

		PlazaRenderPass* AddInputResource(std::shared_ptr<PlazaShadersBinding> resource) {
			mInputBindings.push_back(resource);
			mInputBindingNames.emplace(resource->mName, resource);
			return this;
		}

		PlazaRenderPass* AddOutputResource(std::shared_ptr<PlazaShadersBinding> resource) {
			mOutputBindings.push_back(resource);
			mOutputBindingNames.emplace(resource->mName, resource);
			return this;
		}

		template<typename T>
		T* GetInputResource(std::string name) {
			if (mInputBindingNames.find(name) == mInputBindingNames.end())
				return nullptr;
			return dynamic_cast<T*>(mInputBindingNames.at(name).get());
		}

		template<typename T>
		T* GetOutputResource(std::string name) {
			if (mOutputBindingNames.find(name) == mOutputBindingNames.end())
				return nullptr;
			return dynamic_cast<T*>(mOutputBindingNames.at(name).get());
		}

		PlazaRenderPass* AddChildPass(std::shared_ptr<PlazaRenderPass> pass) {
			mChildPasses.push_back(pass);
			return pass.get();
		}
		std::vector<std::shared_ptr<PlazaRenderPass>> mChildPasses = std::vector<std::shared_ptr<PlazaRenderPass>>();

		template <class Archive>
		void serialize(Archive& archive) {
			archive(PL_SER(mName), PL_SER(mStage), PL_SER(mRenderMethod), PL_SER(mMultiViewCount), PL_SER(mRenderSize), PL_SER(mDispatchSize), PL_SER(mFlipViewPort),
				PL_SER(mPipelines), PL_SER(mInputBindings), PL_SER(mInputBindingNames), PL_SER(mOutputBindings), PL_SER(mOutputBindingNames), PL_SER(mChildPasses));
		}
	private:
		virtual void CompileGraphics(PlazaRenderGraph* renderGraph) { };
	};

	struct BindingModifiers {
		BindingModifiers() {};
		BindingModifiers(shared_ptr<PlazaShadersBinding> bind) : binding(bind) {};
		std::vector<std::string> writePasses = std::vector<std::string>();
		std::vector<std::string> readPasses = std::vector<std::string>();
		shared_ptr<PlazaShadersBinding> binding = nullptr;
	};

	class PLAZA_API PlazaRenderGraph : public Asset {
	public:
		PlazaRenderGraph() {}
		virtual void BuildDefaultRenderGraph() {

		}

		virtual void Execute(Scene* scene, uint8_t imageIndex, uint8_t currentFrame) {};
		virtual void OrderPasses() {};
		void ExecuteRenderPasses() {
			for (auto& [key, value] : mPasses) {
				value->mCallback;
			}
		}
		void Compile() {
			for (auto& pass : mOrderedPasses) {
				PL_CORE_INFO("Compiling Pass: " + pass->mName);
				pass->Compile(this);
			}
		}

		virtual bool BindPass(std::string passName) { return false; };

		PlazaRenderPass* AddRenderPass(std::shared_ptr<PlazaRenderPass> newRenderPass) {
			mOrderedPasses.push_back(newRenderPass);
			mPasses.emplace(newRenderPass->mName, newRenderPass);
			return mPasses[newRenderPass->mName].get();
		}

		void AddRenderPassCallback(std::string passName, std::function<void(PlazaRenderGraph*, PlazaRenderPass*)> callback) {
			if (mPasses.find(passName) != mPasses.end())
				mPasses[passName]->SetRecordingCallback(callback);
		}

		void AddTexture(std::shared_ptr<Texture> texture) {
			mTextures.emplace(texture->mAssetName, texture);
		}

		void AddBuffer(std::shared_ptr<PlBuffer> buffer) {
			mBuffers.emplace(buffer->mName, buffer);
		}

		template<typename T>
		T* GetTexture(const std::string& name) {
			assert(mTextures.find(name) != mTextures.end());
			return dynamic_cast<T*>(mTextures.at(name).get());
		}

		bool HasTexture(const std::string& name) {
			return mTextures.find(name) != mTextures.end();
		}

		template<typename T>
		T* GetBuffer(const std::string& name) {
			assert(mBuffers.find(name) != mBuffers.end());
			return dynamic_cast<T*>(mBuffers.at(name).get());
		}

		std::shared_ptr<PlazaRenderPass> GetSharedRenderPass(const std::string& name) {
			if (mPasses.find(name) != mPasses.end())
				return mPasses.find(name)->second;
			return nullptr;
		}

		virtual void AddPipeline() {};
		virtual void CreatePipeline(PlPipelineCreateInfo createInfo) {};

		//template<typename T>
		std::shared_ptr<Texture> GetSharedTexture(std::string name) {
			assert(mTextures.find(name) != mTextures.end());
			return mTextures.at(name);//dynamic_cast<std::shared_ptr<T*>>(mTextures.at(name));
		}

		std::shared_ptr<PlBuffer> GetSharedBuffer(std::string name) {
			assert(mBuffers.find(name) != mBuffers.end());
			return mBuffers.at(name);
		}

		std::map<std::string, std::shared_ptr<PlazaRenderPass>> mPasses = std::map<std::string, std::shared_ptr<PlazaRenderPass>>();
		std::map<std::string, std::shared_ptr<PlazaShadersBinding>> mShadersBindings = std::map<std::string, std::shared_ptr<PlazaShadersBinding>>();
		std::set<std::string> mCompiledBindings = std::set<std::string>();

		std::vector<std::shared_ptr<PlazaRenderPass>> mOrderedPasses = std::vector<std::shared_ptr<PlazaRenderPass>>();
		std::vector<std::vector<std::shared_ptr<PlazaShadersBinding>>> mOrderedReadBindings = std::vector<std::vector<std::shared_ptr<PlazaShadersBinding>>>();
		std::vector<std::vector<std::shared_ptr<PlazaShadersBinding>>> mOrderedWriteBindings = std::vector<std::vector<std::shared_ptr<PlazaShadersBinding>>>();
		std::map<uint64_t, TextureInfo> mUsedTexturesInfo = std::map<uint64_t, TextureInfo>();


		static std::vector<PlVertexInputBindingDescription> VertexGetBindingDescription() {
			std::vector<PlVertexInputBindingDescription> bindingDescriptions{};
			bindingDescriptions.push_back(pl::vertexInputBindingDescription(0, sizeof(Vertex), PL_VERTEX_INPUT_RATE_VERTEX));
			bindingDescriptions.push_back(pl::vertexInputBindingDescription(1, sizeof(glm::vec4) * 4, PL_VERTEX_INPUT_RATE_INSTANCE));
			return bindingDescriptions;
		}

		static std::vector<PlVertexInputAttributeDescription> VertexGetAttributeDescriptions() {
			std::vector<PlVertexInputAttributeDescription> attributeDescriptions{};
			attributeDescriptions.push_back(pl::vertexInputAttributeDescription(0, 0, PL_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)));
			attributeDescriptions.push_back(pl::vertexInputAttributeDescription(1, 0, PL_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)));
			attributeDescriptions.push_back(pl::vertexInputAttributeDescription(2, 0, PL_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoords)));
			attributeDescriptions.push_back(pl::vertexInputAttributeDescription(3, 0, PL_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, tangent)));
			attributeDescriptions.push_back(pl::vertexInputAttributeDescription(4, 1, PL_FORMAT_R32G32B32A32_SFLOAT, 0));
			attributeDescriptions.push_back(pl::vertexInputAttributeDescription(5, 1, PL_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 4));
			attributeDescriptions.push_back(pl::vertexInputAttributeDescription(6, 1, PL_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 8));
			attributeDescriptions.push_back(pl::vertexInputAttributeDescription(7, 1, PL_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 12));
			attributeDescriptions.push_back(pl::vertexInputAttributeDescription(8, 0, PL_FORMAT_R32_UINT, offsetof(Vertex, materialIndex)));
			return attributeDescriptions;
		}

		static std::vector<PlVertexInputAttributeDescription> SkinnedVertexGetAttributeDescriptions() {
			std::vector<PlVertexInputAttributeDescription> attributeDescriptions{};
			attributeDescriptions.push_back(pl::vertexInputAttributeDescription(0, 0, PL_FORMAT_R32G32B32_SFLOAT, offsetof(SkinnedVertex, position)));
			attributeDescriptions.push_back(pl::vertexInputAttributeDescription(1, 0, PL_FORMAT_R32G32B32_SFLOAT, offsetof(SkinnedVertex, normal)));
			attributeDescriptions.push_back(pl::vertexInputAttributeDescription(2, 0, PL_FORMAT_R32G32_SFLOAT, offsetof(SkinnedVertex, texCoords)));
			attributeDescriptions.push_back(pl::vertexInputAttributeDescription(3, 0, PL_FORMAT_R32G32B32_SFLOAT, offsetof(SkinnedVertex, tangent)));
			attributeDescriptions.push_back(pl::vertexInputAttributeDescription(4, 1, PL_FORMAT_R32G32B32A32_SFLOAT, 0));
			attributeDescriptions.push_back(pl::vertexInputAttributeDescription(5, 1, PL_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 4));
			attributeDescriptions.push_back(pl::vertexInputAttributeDescription(6, 1, PL_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 8));
			attributeDescriptions.push_back(pl::vertexInputAttributeDescription(7, 1, PL_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 12));
			attributeDescriptions.push_back(pl::vertexInputAttributeDescription(8, 0, PL_FORMAT_R32G32B32A32_SINT, offsetof(SkinnedVertex, boneIds)));
			attributeDescriptions.push_back(pl::vertexInputAttributeDescription(9, 0, PL_FORMAT_R32G32B32A32_SFLOAT, offsetof(SkinnedVertex, weights)));
			attributeDescriptions.push_back(pl::vertexInputAttributeDescription(10, 0, PL_FORMAT_R32_UINT, offsetof(SkinnedVertex, materialIndex)));
			return attributeDescriptions;
		}

		virtual void CompileNotBoundBuffers() {};

		std::map<std::string, std::shared_ptr<PlBuffer>> mBuffers = std::map<std::string, std::shared_ptr<PlBuffer>>();

		void UpdateUsedTexturesInfo() {
			for (const auto& texture : mTextures) {
				mUsedTexturesInfo.emplace(texture.second->GetTextureInfo().mUuid, texture.second->GetTextureInfo());
			}
		}

		template <class Archive>
		void serialize(Archive& archive) {
			archive(cereal::base_class<Asset>(this), PL_SER(mBuffers), PL_SER(mTextures), PL_SER(mPasses), PL_SER(mShadersBindings), PL_SER(mOrderedPasses), PL_SER(mUsedTexturesInfo));

			int index = 0;
			for (const auto& texture : mTextures) {
				texture.second->SetTextureInfo(mUsedTexturesInfo[texture.second->mTextureInfoUuid]);
				index++;
			}
		}
		std::map<std::string, std::shared_ptr<Texture>> mTextures = std::map<std::string, std::shared_ptr<Texture>>();
	private:

	};

	struct EquirectangularToCubeMapPC {
		glm::mat4 mvp;
		bool first;
		float deltaPhi = (2.0f * float(3.14159265358979323846)) / 180.0f;
		float deltaTheta = (0.5f * float(3.14159265358979323846)) / 64.0f;
		float roughness = 1.0f;
		unsigned int numSamples = 32u;
	};
}