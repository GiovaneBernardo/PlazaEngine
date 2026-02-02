#pragma once
#include "PlazaShaders.h"
#include "Texture.h"
#include "Buffer.h"
#include "Engine/Core/Engine.h"
#include "PlazaPipeline.h"
#include "Engine/Core/Renderer/RendererTypes.h"
#include "RenderPass.h"
#include "ShaderReflection.h"

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
		// TODO: REMOVE UNUSED mResourceName
		std::string mResourceName = "";
		bool mUseAsDepthStencilAttachment = false;

		virtual void Compile(std::set<std::string>& compiledBindings) {};
		virtual void Destroy() {};

		template <class Archive> void serialize(Archive& archive) {
			archive(PL_SER(mName), PL_SER(mDescriptorCount), PL_SER(mStage), PL_SER(mLocation), PL_SER(mBinding),
					PL_SER(mBindingType), PL_SER(mMaxBindlessResources));
		}
	};

	class PLAZA_API PlazaBufferBinding : public PlazaShadersBinding {
	  public:
		PlazaBufferBinding() {}
		PlazaBufferBinding(const PlazaBufferBinding& other) = default;
		PlazaBufferBinding(uint64_t descriptorCount, uint8_t binding, PlBufferType type, PlRenderStage stage,
						   std::shared_ptr<PlBuffer> buffer) {
			mBinding = binding;
			mDescriptorCount = descriptorCount;
			mStage = stage;
			mBindingType = PL_BINDING_BUFFER;
			mBuffer = buffer;
			mName = buffer->mName;
			mBufferType = type;
			// mStride = stride;
			// mMaxItems = maxItems;
			// mBufferCount = bufferCount;
		}
		PlBufferType mBufferType;

		std::shared_ptr<PlBuffer> mBuffer = nullptr;

		virtual void Compile() {};
		virtual void Destroy() {};

		template <class Archive> void serialize(Archive& archive) {
			archive(cereal::base_class<PlazaShadersBinding>(this), PL_SER(mBuffer));
		}

	  private:
	};

	class PLAZA_API PlazaTextureBinding : public PlazaShadersBinding {
	  public:
		PlazaTextureBinding() {}
		PlazaTextureBinding(const PlazaTextureBinding& other) = default;
		PlazaTextureBinding(uint64_t descriptorCount, uint8_t location, uint8_t binding, PlBufferType bufferType,
							PlRenderStage renderStage, PlImageLayout initialLayout, uint16_t baseMipLevel,
							uint16_t baseLayerLevel, std::shared_ptr<Texture> texture,
							PlAttachmentOp attachmentOp = PL_ATTACHMENT_OP_AUTO,
							bool useAsDepthStencilAttachment = false)
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

		const TextureInfo& GetTextureInfo() { return mTexture->GetTextureInfo(); }

		std::shared_ptr<Texture> mTexture = nullptr;

		template <class Archive> void serialize(Archive& archive) {
			archive(cereal::base_class<PlazaShadersBinding>(this), PL_SER(mBufferType), PL_SER(mInitialLayout),
					PL_SER(mBaseMipLevel), PL_SER(mBaseLayerLevel), PL_SER(mBaseLayerLevel), PL_SER(mTexture));
		}

	  private:
	};

	class PLAZA_API PlazaTextureSamplerBinding : public PlazaShadersBinding {
	  public:
		PlazaTextureSamplerBinding() {}
		PlazaTextureSamplerBinding(const PlazaTextureSamplerBinding& other) = default;

		PlBufferType mBufferType = PL_BUFFER_SAMPLER;

		virtual void Compile() {};
		virtual void Destroy() {};

		std::shared_ptr<PlTextureSampler> mSampler = nullptr;

		template <class Archive> void serialize(Archive& archive) {
			archive(cereal::base_class<PlazaShadersBinding>(this), PL_SER(mBufferType), PL_SER(mSampler));
		}

	  private:
	};

	class PlazaRenderGraph;

	struct BindingModifiers {
		BindingModifiers() {};
		BindingModifiers(shared_ptr<PlazaShadersBinding> bind) : binding(bind) {};
		std::vector<std::string> writePasses = std::vector<std::string>();
		std::vector<std::string> readPasses = std::vector<std::string>();
		shared_ptr<PlazaShadersBinding> binding = nullptr;
	};

	class PLAZA_API PlazaRenderGraph : public Asset {
	  public:
		Renderer* mRenderer;
		PlazaRenderGraph(Renderer* renderer) { mRenderer = renderer; }
		void BuildDefaultRenderGraph();

		virtual void Execute(Scene* scene, uint8_t imageIndex, uint8_t currentFrame) {};
		virtual void OrderPasses() {};
		void ExecuteRenderPasses() {
			for (auto& [key, value] : mPasses) {
				value->mCallback;
			}
		}
		void Compile() {
			for (auto& pass : mOrderedPasses) {
				PL_CORE_INFO("Reflecting Pass: " + pass->mName);
				pass->ReflectPass(this);
			}
			for (auto& pass : mOrderedPasses) {
				PL_CORE_INFO("Compiling Pass: " + pass->mName);
				pass->Compile(this);
			}
		}
		virtual void CompileBuffer(std::shared_ptr<PlBuffer> buffer, std::set<std::string>& compiledBindings) = 0;
		virtual void CompileTexture(std::shared_ptr<PlazaTextureBinding> texture,
									std::set<std::string>& compiledBindings) = 0;
		virtual void CompileTextureSampler(std::shared_ptr<PlazaTextureSamplerBinding> binding,
										   std::set<std::string>& compiledBindings) = 0;

		virtual bool BindPass(std::string passName) { return false; };

		virtual PlazaRenderPass* AddRenderPass(const std::string& name, int stage, PlRenderPassMode renderMethod,
											   glm::vec2 size, bool flipViewPort) = 0;

		void AddRenderPassCallback(std::string passName,
								   std::function<void(PlazaRenderGraph*, PlazaRenderPass*, Scene*)> callback) {
			if (mPasses.find(passName) != mPasses.end())
				mPasses[passName]->SetRecordingCallback(callback);
		}

		/* Deprecated */
		void AddTexture(std::shared_ptr<Texture> texture) { mTextures.emplace(texture->mAssetName, texture); }
		void AddBuffer(std::shared_ptr<PlBuffer> buffer) { mBuffers.emplace(buffer->mName, buffer); }
		/* ---------- */

		// Resources creation
		virtual void AddTexture(uint64_t descriptorCount, PlImageUsage imageUsage, PlTextureType imageType,
								PlViewType viewType, PlTextureFormat format, glm::vec3 resolution, uint8_t mipCount,
								uint16_t layersCount, const std::string& name) = 0;

		virtual void AddBuffer(PlBufferType type, uint64_t maxItems, uint16_t stride, uint8_t bufferCount,
							   PlBufferUsage bufferUsage, PlMemoryUsage memoryUsage, const std::string& name) = 0;

		virtual void AddSampler(const std::string& name, PlFilter mMagFilter = PL_FILTER_LINEAR,
								PlFilter mMinFilter = PL_FILTER_LINEAR,
								PlSamplerAddressMode mAddressModeU = PL_SAMPLER_ADDRESS_MODE_REPEAT,
								PlSamplerAddressMode mAddressModeV = PL_SAMPLER_ADDRESS_MODE_REPEAT,
								PlSamplerAddressMode mAddressModeW = PL_SAMPLER_ADDRESS_MODE_REPEAT,
								bool mUseAnisotropy = true, float mMaxAnisotropy = 16.0f,
								PlBorderColor mBorderColor = PL_BORDER_COLOR_INT_OPAQUE_BLACK,
								bool mUseUnnormalizedCoordinates = false, bool mUseCompare = false,
								PlCompareOp mCompareOp = PL_COMPARE_OP_ALWAYS,
								PlSamplerMipmapMode mMipmapMode = PL_SAMPLER_MIPMAP_MODE_LINEAR,
								float mMipLodBias = 0.0f, float mMinLod = 0.0f, float mMaxLod = 0.0f) = 0;

		PlazaRenderPass* GetRenderPass(const std::string& name) {
			if (mPasses.find(name) != mPasses.end())
				return mPasses.find(name)->second.get();
			return nullptr;
		}

		template <typename T> T* GetTexture(const std::string& name) {
			assert(mTextures.find(name) != mTextures.end());
			return dynamic_cast<T*>(mTextures.at(name).get());
		}

		bool HasTexture(const std::string& name) { return mTextures.find(name) != mTextures.end(); }

		template <typename T> T* GetBuffer(const std::string& name) {
			assert(mBuffers.find(name) != mBuffers.end());
			return dynamic_cast<T*>(mBuffers.at(name).get());
		}

		std::shared_ptr<PlazaRenderPass> GetSharedRenderPass(const std::string& name) {
			if (mPasses.find(name) != mPasses.end())
				return mPasses.find(name)->second;
			return nullptr;
		}

		virtual void CreatePipeline(PlPipelineCreateInfo createInfo) {};

		// template<typename T>
		std::shared_ptr<Texture> GetSharedTexture(std::string name) {
			assert(mTextures.find(name) != mTextures.end());
			return mTextures.at(name);
		}

		std::shared_ptr<PlTextureSampler> GetSharedTextureSampler(std::string name) {
			assert(mTextureSamplers.find(name) != mTextureSamplers.end());
			return mTextureSamplers.at(name);
		}

		std::shared_ptr<PlBuffer> GetSharedBuffer(std::string name) {
			assert(mBuffers.find(name) != mBuffers.end());
			return mBuffers.at(name);
		}

		std::map<std::string, std::shared_ptr<PlazaRenderPass>> mPasses =
			std::map<std::string, std::shared_ptr<PlazaRenderPass>>();
		std::map<std::string, std::shared_ptr<PlazaShadersBinding>> mShadersBindings =
			std::map<std::string, std::shared_ptr<PlazaShadersBinding>>();
		std::set<std::string> mCompiledBindings = std::set<std::string>();

		std::vector<std::shared_ptr<PlazaRenderPass>> mOrderedPasses = std::vector<std::shared_ptr<PlazaRenderPass>>();
		std::vector<std::vector<std::shared_ptr<PlazaShadersBinding>>> mOrderedReadBindings =
			std::vector<std::vector<std::shared_ptr<PlazaShadersBinding>>>();
		std::vector<std::vector<std::shared_ptr<PlazaShadersBinding>>> mOrderedWriteBindings =
			std::vector<std::vector<std::shared_ptr<PlazaShadersBinding>>>();
		std::map<uint64_t, TextureInfo> mUsedTexturesInfo = std::map<uint64_t, TextureInfo>();

		static std::vector<PlVertexInputBindingDescription> VertexGetBindingDescription() {
			std::vector<PlVertexInputBindingDescription> bindingDescriptions{};
			bindingDescriptions.push_back(
				pl::vertexInputBindingDescription(0, sizeof(Vertex), PL_VERTEX_INPUT_RATE_VERTEX));
			bindingDescriptions.push_back(
				pl::vertexInputBindingDescription(1, sizeof(glm::vec4) * 4, PL_VERTEX_INPUT_RATE_INSTANCE));
			return bindingDescriptions;
		}

		static std::vector<PlVertexInputAttributeDescription> VertexGetAttributeDescriptions() {
			std::vector<PlVertexInputAttributeDescription> attributeDescriptions{};
			attributeDescriptions.push_back(
				pl::vertexInputAttributeDescription(0, 0, PL_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)));
			attributeDescriptions.push_back(
				pl::vertexInputAttributeDescription(1, 0, PL_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)));
			attributeDescriptions.push_back(
				pl::vertexInputAttributeDescription(2, 0, PL_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoords)));
			attributeDescriptions.push_back(
				pl::vertexInputAttributeDescription(3, 0, PL_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, tangent)));
			attributeDescriptions.push_back(
				pl::vertexInputAttributeDescription(4, 1, PL_FORMAT_R32G32B32A32_SFLOAT, 0));
			attributeDescriptions.push_back(
				pl::vertexInputAttributeDescription(5, 1, PL_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 4));
			attributeDescriptions.push_back(
				pl::vertexInputAttributeDescription(6, 1, PL_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 8));
			attributeDescriptions.push_back(
				pl::vertexInputAttributeDescription(7, 1, PL_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 12));
			attributeDescriptions.push_back(
				pl::vertexInputAttributeDescription(8, 0, PL_FORMAT_R32_UINT, offsetof(Vertex, materialIndex)));
			return attributeDescriptions;
		}

		static std::vector<PlVertexInputAttributeDescription> SkinnedVertexGetAttributeDescriptions() {
			std::vector<PlVertexInputAttributeDescription> attributeDescriptions{};
			attributeDescriptions.push_back(pl::vertexInputAttributeDescription(0, 0, PL_FORMAT_R32G32B32_SFLOAT,
																				offsetof(SkinnedVertex, position)));
			attributeDescriptions.push_back(
				pl::vertexInputAttributeDescription(1, 0, PL_FORMAT_R32G32B32_SFLOAT, offsetof(SkinnedVertex, normal)));
			attributeDescriptions.push_back(
				pl::vertexInputAttributeDescription(2, 0, PL_FORMAT_R32G32_SFLOAT, offsetof(SkinnedVertex, texCoords)));
			attributeDescriptions.push_back(pl::vertexInputAttributeDescription(3, 0, PL_FORMAT_R32G32B32_SFLOAT,
																				offsetof(SkinnedVertex, tangent)));
			attributeDescriptions.push_back(
				pl::vertexInputAttributeDescription(4, 1, PL_FORMAT_R32G32B32A32_SFLOAT, 0));
			attributeDescriptions.push_back(
				pl::vertexInputAttributeDescription(5, 1, PL_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 4));
			attributeDescriptions.push_back(
				pl::vertexInputAttributeDescription(6, 1, PL_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 8));
			attributeDescriptions.push_back(
				pl::vertexInputAttributeDescription(7, 1, PL_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 12));
			attributeDescriptions.push_back(pl::vertexInputAttributeDescription(8, 0, PL_FORMAT_R32G32B32A32_SINT,
																				offsetof(SkinnedVertex, boneIds)));
			attributeDescriptions.push_back(pl::vertexInputAttributeDescription(9, 0, PL_FORMAT_R32G32B32A32_SFLOAT,
																				offsetof(SkinnedVertex, weights)));
			attributeDescriptions.push_back(
				pl::vertexInputAttributeDescription(10, 0, PL_FORMAT_R32_UINT, offsetof(SkinnedVertex, materialIndex)));
			return attributeDescriptions;
		}

		virtual void CompileNotBoundBuffers() {};

		std::map<std::string, std::shared_ptr<PlBuffer>> mBuffers = std::map<std::string, std::shared_ptr<PlBuffer>>();

		void UpdateUsedTexturesInfo() {
			for (const auto& texture : mTextures) {
				mUsedTexturesInfo.emplace(texture.second->GetTextureInfo().mUuid, texture.second->GetTextureInfo());
			}
		}

		template <class Archive> void serialize(Archive& archive) {
			archive(cereal::base_class<Asset>(this), PL_SER(mBuffers), PL_SER(mTextures), PL_SER(mPasses),
					PL_SER(mShadersBindings), PL_SER(mOrderedPasses), PL_SER(mUsedTexturesInfo));

			int index = 0;
			for (const auto& texture : mTextures) {
				texture.second->SetTextureInfo(mUsedTexturesInfo[texture.second->mTextureInfoUuid]);
				index++;
			}
		}
		std::map<std::string, std::shared_ptr<Texture>> mTextures = std::map<std::string, std::shared_ptr<Texture>>();
		std::map<std::string, std::shared_ptr<PlTextureSampler>> mTextureSamplers =
			std::map<std::string, std::shared_ptr<PlTextureSampler>>();

	  private:
		void BuildResources();
		void BuildNodes();

	  protected:
		PlazaRenderPass* AddRenderPass(std::shared_ptr<PlazaRenderPass> newRenderPass) {
			mOrderedPasses.push_back(newRenderPass);
			mPasses.emplace(newRenderPass->mName, newRenderPass);
			return mPasses[newRenderPass->mName].get();
		}
	};

	// Buffers structs

	struct EquirectangularToCubeMapPC {
		glm::mat4 mvp;
		bool first;
		float deltaPhi = (2.0f * float(3.14159265358979323846)) / 180.0f;
		float deltaTheta = (0.5f * float(3.14159265358979323846)) / 64.0f;
		float roughness = 1.0f;
		unsigned int numSamples = 32u;
	};

	// Buffer types
	struct UniformBufferObject {
		glm::mat4 projection;				   // 64 bytes
		glm::mat4 view;						   // 64 bytes
		glm::mat4 model;					   // 64 bytes
		int cascadeCount;					   // 4 bytes
		float farPlane;						   // 4 bytes
		float nearPlane;					   // 4 bytes
		alignas(16) glm::vec4 lightDirection;  // 16 bytes, forced alignment to 16 bytes
		glm::vec4 viewPos;					   // 16 bytes
		glm::mat4 lightSpaceMatrices[16];	   // 16 * 64 bytes = 1024 bytes
		glm::vec4 cascadePlaneDistances[16];   // 16 * 16 bytes = 256 bytes
		glm::vec4 directionalLightColor;	   // 16 bytes
		glm::vec4 ambientLightColor;		   // 16 bytes
		alignas(4) uint32_t showCascadeLevels; // 4 bytes, bool aligns to 4 bytes (use uint32_t)
		float gamma;						   // 4 bytes
	};
	struct alignas(16) DeferredLightingPassUbo {
		glm::mat4 projection;
		glm::mat4 view;
		alignas(4) uint32_t showCascadeLevels;
		float farPlane;
		float nearPlane;
		float gamma;
		float exposure;
		int cascadeCount;
		int lightCount;
		alignas(16) glm::vec4 viewPos;
		glm::vec4 lightDirection;
		glm::vec4 ambientLightColor;
		glm::vec4 directionalLightColor;
		alignas(16) glm::vec2 screenSize;
		alignas(16) glm::vec3 clusterSize;
		float _padding0;
		glm::mat4 lightSpaceMatrices[16];
		glm::vec4 cascadePlaneDistances[16];
	};



	struct ShadowPassUBO {
		glm::mat4 lightSpaceMatrices[32];
	};

	struct alignas(16) MaterialData {
		glm::vec4 color = glm::vec4(1.0f);
		float intensity = 1.0f;
		int diffuseIndex = -1;
		int normalIndex = -1;
		int roughnessIndex = -1;
		int metalnessIndex = -1;
		float roughnessFloat = 0.5f;
		float metalnessFloat = 0.5f;
		float flipX = 1.0f;
		float flipY = 1.0f;
	};

	struct LightSorterPC {
		glm::mat4 view;
		glm::mat4 projection;
		glm::mat4 invProjection;
		glm::mat4 invView;
		int32_t lightCount;
		glm::vec3 numGroups;
		glm::vec2 screenSize;
		glm::vec2 clusterSize;
	};
} // namespace Plaza
