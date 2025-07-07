#include "Engine/Core/PreCompiledHeaders.h"
#include "RenderGraph.h"

#include "Renderer.h"
#include "stb_font_consolas_24_latin1.inl"
#include "Engine/Core/Scene.h"

namespace Plaza {
	void PlazaRenderGraph::BuildDefaultRenderGraph() {
		BuildResources();
		BuildNodes();
	}

	void PlazaRenderGraph::BuildResources() {
		const int maxOutlineMeshes = 8192;

		/* Textures */
		PlImageUsage inImageUsageFlags =
			static_cast<PlImageUsage>(PL_IMAGE_USAGE_COLOR_ATTACHMENT | PL_IMAGE_USAGE_SAMPLED |
									  PL_IMAGE_USAGE_TRANSFER_DST | PL_IMAGE_USAGE_TRANSFER_SRC);
		PlImageUsage outImageUsageFlags =
			static_cast<PlImageUsage>(PL_IMAGE_USAGE_TRANSFER_DST | PL_IMAGE_USAGE_TRANSFER_SRC |
									  PL_IMAGE_USAGE_SAMPLED | PL_IMAGE_USAGE_COLOR_ATTACHMENT);
		PlImageUsage depthTextureFlags =
			static_cast<PlImageUsage>(PL_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT | PL_IMAGE_USAGE_SAMPLED |
									  PL_IMAGE_USAGE_TRANSFER_DST | PL_IMAGE_USAGE_TRANSFER_SRC);
		PlImageUsage equirectangularImageUsage =
			static_cast<PlImageUsage>(PL_IMAGE_USAGE_COLOR_ATTACHMENT | PL_IMAGE_USAGE_SAMPLED |
									  PL_IMAGE_USAGE_TRANSFER_DST | PL_IMAGE_USAGE_TRANSFER_SRC);
		const unsigned int bufferCount = Application::Get()->mRenderer->mMaxFramesInFlight;
		const unsigned int shadowMapResolution = 2048;
		const unsigned int skyboxResolution = 512;
		const unsigned int irradianceSize = 64;
		const unsigned int brdfSize = 512;
		const glm::vec2 screenSize = Application::Get()->appSizes->sceneSize;
		const glm::vec2 deferredTileSize = glm::vec2(32, 32);
		const uint32_t clusterCount =
			glm::ceil(screenSize.x / deferredTileSize.x + 1) * glm::ceil(screenSize.y / deferredTileSize.y + 1);

		this->AddTexture(mRenderer->mMaxBindlessTextures, inImageUsageFlags, PL_TYPE_2D, PL_VIEW_TYPE_2D,
						 PL_FORMAT_R32G32B32A32_SFLOAT, glm::vec3(1, 1, 1), 1, 0, "TexturesBuffer");

		this->AddTexture(
			1, equirectangularImageUsage, PL_TYPE_2D, PL_VIEW_TYPE_2D, PL_FORMAT_R8_UNORM,
			glm::vec3(STB_FONT_consolas_24_latin1_BITMAP_WIDTH, STB_FONT_consolas_24_latin1_BITMAP_HEIGHT, 1), 1, 1,
			"FontTexture");
		this->AddTexture(1, equirectangularImageUsage, PL_TYPE_2D, PL_VIEW_TYPE_2D, PL_FORMAT_R32G32B32A32_SFLOAT,
						 glm::vec3(skyboxResolution, skyboxResolution, 1), 1, 1, "EquirectangularTexture");
		this->AddTexture(1, equirectangularImageUsage, PL_TYPE_2D, PL_VIEW_TYPE_CUBE, PL_FORMAT_R32G32B32A32_SFLOAT,
						 glm::vec3(skyboxResolution, skyboxResolution, 1), 1, 6, "CubeMapTexture");
		this->AddTexture(1, inImageUsageFlags, PL_TYPE_2D, PL_VIEW_TYPE_2D, PL_FORMAT_R16G16_SFLOAT,
						 glm::vec3(brdfSize, brdfSize, 1), 1, 1, "SamplerBRDFLUT");
		this->AddTexture(1, inImageUsageFlags, PL_TYPE_2D, PL_VIEW_TYPE_CUBE, PL_FORMAT_R32G32B32A32_SFLOAT,
						 glm::vec3(skyboxResolution, skyboxResolution, 1), 0, 6, "PreFilterMap");
		this->AddTexture(1, inImageUsageFlags, PL_TYPE_2D, PL_VIEW_TYPE_CUBE, PL_FORMAT_R32G32B32A32_SFLOAT,
						 glm::vec3(irradianceSize, irradianceSize, 1), 1, 6, "IrradianceMap");
		this->AddTexture(1, depthTextureFlags, PL_TYPE_2D, PL_VIEW_TYPE_2D_ARRAY, PL_FORMAT_D32_SFLOAT,
						 glm::vec3(shadowMapResolution, shadowMapResolution, 1), 1,
						 mRenderer->mRendererSettings.mLightingSettings.mCascadeCount, "ShadowsDepthMap");
		this->AddTexture(1, outImageUsageFlags, PL_TYPE_2D, PL_VIEW_TYPE_2D, PL_FORMAT_R32G32B32A32_SFLOAT,
						 glm::vec3(Application::Get()->appSizes->sceneSize, 1), 1, 1, "GDiffuse");
		this->AddTexture(1, outImageUsageFlags, PL_TYPE_2D, PL_VIEW_TYPE_2D, PL_FORMAT_R32G32B32A32_SFLOAT,
						 glm::vec3(Application::Get()->appSizes->sceneSize, 1), 1, 1, "GNormal");
		this->AddTexture(1, outImageUsageFlags, PL_TYPE_2D, PL_VIEW_TYPE_2D, PL_FORMAT_R32G32B32A32_SFLOAT,
						 glm::vec3(Application::Get()->appSizes->sceneSize, 1), 1, 1, "GOthers");
		this->AddTexture(1, depthTextureFlags, PL_TYPE_2D, PL_VIEW_TYPE_2D, PL_FORMAT_D32_SFLOAT,
						 glm::vec3(Application::Get()->appSizes->sceneSize, 1), 1, 1, "SceneDepth");
		this->AddTexture(1, depthTextureFlags, PL_TYPE_2D, PL_VIEW_TYPE_2D, PL_FORMAT_D32_SFLOAT_S8_UINT,
						 glm::vec3(Application::Get()->appSizes->sceneSize, 1), 1, 1, "OutlineStencil");

		TextureInfo info{};

		info.mPath = "deferred";
		this->GetTexture<Texture>("GDiffuse")->CreateTextureInfo(info);
		this->GetTexture<Texture>("GNormal")->CreateTextureInfo(info);
		this->GetTexture<Texture>("GOthers")->CreateTextureInfo(info);
		this->GetTexture<Texture>("SceneDepth")->CreateTextureInfo(info);

		this->AddTexture(1, static_cast<PlImageUsage>(outImageUsageFlags | PL_IMAGE_USAGE_STORAGE), PL_TYPE_2D,
						 PL_VIEW_TYPE_2D, PL_FORMAT_R32G32B32A32_SFLOAT,
						 glm::vec3(Application::Get()->appSizes->sceneSize, 1), 1, 1, "SceneTexture");
		this->AddTexture(1, outImageUsageFlags, PL_TYPE_2D, PL_VIEW_TYPE_2D, PL_FORMAT_R32G32B32A32_SFLOAT,
						 glm::vec3(Application::Get()->appSizes->sceneSize, 1), 1, 1, "OutFinalPostProcessTexture");
		this->AddTexture(1, outImageUsageFlags, PL_TYPE_2D, PL_VIEW_TYPE_2D, PL_FORMAT_R8G8B8A8_UNORM,
						 glm::vec3(Application::Get()->appSizes->sceneSize, 1), 1, 1, "FinalTexture");
		this->AddTexture(1, outImageUsageFlags, PL_TYPE_2D, PL_VIEW_TYPE_2D, PL_FORMAT_R8G8B8A8_UNORM,
						 glm::vec3(Application::Get()->appSizes->sceneSize, 1), 1, 1, "PostProcessedTexture");
		this->AddTexture(1, outImageUsageFlags, PL_TYPE_2D, PL_VIEW_TYPE_2D, PL_FORMAT_R8G8B8A8_UNORM,
						 glm::vec3(Application::Get()->appSizes->sceneSize, 1), 1, 1, "OutlineTexture");
		TextureInfo outlineInfo = this->GetTexture<Texture>("OutlineTexture")->GetTextureInfo();
		outlineInfo.mSamplerAddressMode = PL_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		this->GetTexture<Texture>("OutlineTexture")->SetTextureInfo(outlineInfo);

		this->AddTexture(1, outImageUsageFlags, PL_TYPE_2D, PL_VIEW_TYPE_2D, PL_FORMAT_R8G8B8A8_UNORM,
						 glm::vec3(Application::Get()->appSizes->sceneSize, 1), 1, 1, "OutlineBlurredTexture");

		this->AddTexture(1, PlImageUsage(outImageUsageFlags | PL_IMAGE_USAGE_STORAGE), PL_TYPE_2D, PL_VIEW_TYPE_2D,
						 PL_FORMAT_R32G32B32A32_SFLOAT, glm::vec3(Application::Get()->appSizes->sceneSize, 1), 0, 1,
						 "BloomTexture");
		TextureInfo bloomInfo = this->GetTexture<Texture>("BloomTexture")->GetTextureInfo();
		bloomInfo.mInitialLayout = PL_IMAGE_LAYOUT_GENERAL;
		bloomInfo.mSamplerAddressMode = PL_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		this->GetTexture<Texture>("BloomTexture")->SetTextureInfo(bloomInfo);

		this->AddTexture(1, outImageUsageFlags, PL_TYPE_2D, PL_VIEW_TYPE_2D, PL_FORMAT_R32G32B32A32_SFLOAT,
						 glm::vec3(Application::Get()->appSizes->sceneSize, 1), 1, 1, "SSRTexture");

		this->AddTexture(1, PlImageUsage(outImageUsageFlags | PL_IMAGE_USAGE_STORAGE), PL_TYPE_2D, PL_VIEW_TYPE_2D,
						 PL_FORMAT_R32G32B32A32_SFLOAT, glm::vec3(1, 1, 1), 1, 1, "LuminanceTexture");
		TextureInfo luminanceInfo = this->GetTexture<Texture>("LuminanceTexture")->GetTextureInfo();
		luminanceInfo.mInitialLayout = PL_IMAGE_LAYOUT_GENERAL;
		this->GetTexture<Texture>("LuminanceTexture")->SetTextureInfo(luminanceInfo);

		this->AddTexture(1, outImageUsageFlags, PL_TYPE_2D, PL_VIEW_TYPE_2D, PL_FORMAT_R32G32B32A32_SFLOAT,
						 glm::vec3(8, 8, 1), 1, 1, "SceneDownSampledTexture");

		std::string skyboxPath;
#ifdef EDITOR_MODE
		skyboxPath = FilesManager::sEngineFolder.string() + "/Editor/DefaultAssets/Skybox/";
#else
		skyboxPath = Application::Get()->exeDirectory + "/";
#endif

		TextureInfo equirectangularInfo = this->GetTexture<Texture>("EquirectangularTexture")->GetTextureInfo();
		equirectangularInfo.mPath = skyboxPath + "starmap_4k.jpg"; //"autumn_field_puresky_4k.hdr";
		equirectangularInfo.mIsHdr = true;
		this->GetTexture<Texture>("EquirectangularTexture")->SetTextureInfo(equirectangularInfo);

		/* Buffers */
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
		struct DeferredLightingPassUbo {
			glm::mat4 projection;				   // 64 bytes
			glm::mat4 view;						   // 64 bytes
			alignas(4) uint32_t showCascadeLevels; // 4 bytes, must align to 4 bytes
			float farPlane;						   // 4 bytes
			float nearPlane;					   // 4 bytes
			float gamma;						   // 4 bytes
			float exposure;						   // 4 bytes
			int cascadeCount;					   // 4 bytes
			int lightCount;						   // 4 bytes
			alignas(16) glm::vec4 viewPos;		   // 16 bytes
			glm::vec4 lightDirection;			   // 16 bytes
			glm::vec4 ambientLightColor;		   // 16 bytes
			glm::vec4 directionalLightColor;	   // 16 bytes
			glm::mat4 lightSpaceMatrices[16];	   // 16 * 64 bytes = 1024 bytes
			glm::vec4 cascadePlaneDistances[16];   // 16 * 16 bytes = 256 bytes
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

		this->AddBuffer(PL_BUFFER_UNIFORM_BUFFER, 1, sizeof(ShadowPassUBO), bufferCount, PL_BUFFER_USAGE_UNIFORM_BUFFER,
						PL_MEMORY_USAGE_CPU_TO_GPU, "ShadowPassUBO");
		this->AddBuffer(PL_BUFFER_UNIFORM_BUFFER, 1, sizeof(UniformBufferObject), bufferCount,
						PL_BUFFER_USAGE_UNIFORM_BUFFER, PL_MEMORY_USAGE_CPU_TO_GPU, "GPassUBO");
		this->AddBuffer(PL_BUFFER_UNIFORM_BUFFER, 1, sizeof(DeferredLightingPassUbo), bufferCount,
						PL_BUFFER_USAGE_UNIFORM_BUFFER, PL_MEMORY_USAGE_CPU_TO_GPU, "DeferredPassUBO");
		this->AddBuffer(PL_BUFFER_STORAGE_BUFFER, 1024 * 16, sizeof(glm::mat4), bufferCount,
						PL_BUFFER_USAGE_STORAGE_BUFFER, PL_MEMORY_USAGE_CPU_TO_GPU, "BoneMatricesBuffer");
		this->AddBuffer(PL_BUFFER_STORAGE_BUFFER, 1024 * 16, sizeof(MaterialData), bufferCount,
						PL_BUFFER_USAGE_STORAGE_BUFFER, PL_MEMORY_USAGE_CPU_TO_GPU, "MaterialsBuffer");
		this->AddBuffer(PL_BUFFER_STORAGE_BUFFER, 1024 * 256 * 64, sizeof(unsigned int), bufferCount,
						PL_BUFFER_USAGE_STORAGE_BUFFER, PL_MEMORY_USAGE_CPU_TO_GPU, "RenderGroupOffsetsBuffer");
		this->AddBuffer(PL_BUFFER_STORAGE_BUFFER, 1024 * 256 * 64, sizeof(unsigned int), bufferCount,
						PL_BUFFER_USAGE_STORAGE_BUFFER, PL_MEMORY_USAGE_CPU_TO_GPU,
						"RenderGroupMaterialsOffsetsBuffer");
		this->AddBuffer(PL_BUFFER_STORAGE_BUFFER, 1024 * 32, sizeof(RendererSettings::LightStruct), bufferCount,
						static_cast<PlBufferUsage>(PL_BUFFER_USAGE_STORAGE_BUFFER | PL_BUFFER_USAGE_TRANSFER_DST),
						PL_MEMORY_USAGE_CPU_TO_GPU, "LightsBuffer");
		this->AddBuffer(PL_BUFFER_STORAGE_BUFFER, clusterCount, sizeof(RendererSettings::Tile), bufferCount,
						static_cast<PlBufferUsage>(PL_BUFFER_USAGE_STORAGE_BUFFER | PL_BUFFER_USAGE_TRANSFER_DST),
						PL_MEMORY_USAGE_CPU_TO_GPU, "ClustersBuffer");
		this->AddBuffer(PL_BUFFER_STORAGE_BUFFER, clusterCount, sizeof(glm::vec2), bufferCount,
						static_cast<PlBufferUsage>(PL_BUFFER_USAGE_STORAGE_BUFFER | PL_BUFFER_USAGE_TRANSFER_DST),
						PL_MEMORY_USAGE_CPU_TO_GPU, "TilesDepthBuffer");
		this->AddBuffer(PL_BUFFER_STORAGE_BUFFER, 1024, sizeof(glm::mat4), bufferCount,
						static_cast<PlBufferUsage>(PL_BUFFER_USAGE_VERTEX_BUFFER), PL_MEMORY_USAGE_CPU_TO_GPU,
						"RectanglesTransformBuffer");
		this->AddBuffer(PL_BUFFER_STORAGE_BUFFER, 1024 * 2, sizeof(glm::vec4), bufferCount,
						static_cast<PlBufferUsage>(PL_BUFFER_USAGE_VERTEX_BUFFER), PL_MEMORY_USAGE_CPU_TO_GPU,
						"GuiTextVerticesBuffer");

#ifdef EDITOR_MODE
		this->AddBuffer(PL_BUFFER_STORAGE_BUFFER, 8192, sizeof(glm::mat4), bufferCount,
						static_cast<PlBufferUsage>(PL_BUFFER_USAGE_VERTEX_BUFFER), PL_MEMORY_USAGE_CPU_TO_GPU,
						"OutlineMatrixBuffer");
#endif
	}

	void GetChildrendUuid(std::vector<uint64_t>& vector, Entity* entity, Scene* scene);
	std::vector<glm::mat4> GetShadowMatrices(RendererSettings::LightingSettings& settings, const glm::mat4& camProj,
											 const glm::mat4& camView);

	void PlazaRenderGraph::BuildNodes() {
		const int maxOutlineMeshes = 8192;
		const unsigned int bufferCount = Application::Get()->mRenderer->mMaxFramesInFlight;
		const unsigned int shadowMapResolution = 2048;
		const unsigned int skyboxResolution = 512;
		const unsigned int irradianceSize = 64;
		const unsigned int brdfSize = 512;
		const glm::vec2 screenSize = Application::Get()->appSizes->sceneSize;
		const glm::vec2 deferredTileSize = glm::vec2(32, 32);
		const uint32_t clusterCount =
			glm::ceil(screenSize.x / deferredTileSize.x + 1) * glm::ceil(screenSize.y / deferredTileSize.y + 1);

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
		struct DeferredLightingPassUbo {
			glm::mat4 projection;				   // 64 bytes
			glm::mat4 view;						   // 64 bytes
			alignas(4) uint32_t showCascadeLevels; // 4 bytes, must align to 4 bytes
			float farPlane;						   // 4 bytes
			float nearPlane;					   // 4 bytes
			float gamma;						   // 4 bytes
			float exposure;						   // 4 bytes
			int cascadeCount;					   // 4 bytes
			int lightCount;						   // 4 bytes
			alignas(16) glm::vec4 viewPos;		   // 16 bytes
			glm::vec4 lightDirection;			   // 16 bytes
			glm::vec4 ambientLightColor;		   // 16 bytes
			glm::vec4 directionalLightColor;	   // 16 bytes
			glm::mat4 lightSpaceMatrices[16];	   // 16 * 64 bytes = 1024 bytes
			glm::vec4 cascadePlaneDistances[16];   // 16 * 16 bytes = 256 bytes
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

		this->AddRenderPass("Shadow Pass", PL_STAGE_VERTEX | PL_STAGE_FRAGMENT,
							PL_RENDER_PASS_INDIRECT_BUFFER_SHADOW_MAP,
							glm::vec2(shadowMapResolution, shadowMapResolution), true)
			->AddInputBuffer(1, 0, PlBufferType::PL_BUFFER_UNIFORM_BUFFER, PL_STAGE_VERTEX,
							 this->GetSharedBuffer("ShadowPassUBO"))
			->AddInputBuffer(1, 1, PL_BUFFER_STORAGE_BUFFER, PL_STAGE_VERTEX,
							 this->GetSharedBuffer("BoneMatricesBuffer"))
			->AddOutputTexture(1, 0, 0, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT,
							   PL_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 0, 0,
							   this->GetSharedTexture("ShadowsDepthMap"));

		this->GetRenderPass("Shadow Pass")->mMultiViewCount =
			mRenderer->mRendererSettings.mLightingSettings.mCascadeCount;
		this->GetRenderPass("Shadow Pass")
			->AddPipeline(pl::pipelineCreateInfo(
				"ShadowMapping", PL_RENDER_PASS_INDIRECT_BUFFER_SHADOW_MAP,
				{pl::pipelineShaderStageCreateInfo(
					 PL_STAGE_VERTEX,
					 FilesManager::sEngineFolder.string() + "/Shaders/shadows/cascadedShadowDepthShaders.vert", "main"),
				 pl::pipelineShaderStageCreateInfo(PL_STAGE_FRAGMENT,
												   FilesManager::sEngineFolder.string() +
													   "/Shaders/shadows/cascadedShadowDepthShaders.frag",
												   "main")},
				{pl::vertexInputBindingDescription(0, sizeof(Vertex), PL_VERTEX_INPUT_RATE_VERTEX),
				 pl::vertexInputBindingDescription(1, sizeof(glm::vec4) * 4, PL_VERTEX_INPUT_RATE_INSTANCE)},
				{pl::vertexInputAttributeDescription(0, 0, PL_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)),
				 pl::vertexInputAttributeDescription(5, 1, PL_FORMAT_R32G32B32A32_SFLOAT, 0),
				 pl::vertexInputAttributeDescription(6, 1, PL_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 4),
				 pl::vertexInputAttributeDescription(7, 1, PL_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 8),
				 pl::vertexInputAttributeDescription(8, 1, PL_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 12)},
				PL_TOPOLOGY_TRIANGLE_LIST, false,
				pl::pipelineRasterizationStateCreateInfo(false, false, PL_POLYGON_MODE_FILL, 1.0f, false, 0.0f, 0.0f,
														 0.0f, PL_CULL_MODE_BACK, PL_FRONT_FACE_COUNTER_CLOCKWISE),
				pl::pipelineColorBlendStateCreateInfo({pl::pipelineColorBlendAttachmentState(true)}),
				pl::pipelineDepthStencilStateCreateInfo(true, true, PL_COMPARE_OP_LESS),
				pl::pipelineViewportStateCreateInfo(1, 1),
				pl::pipelineMultisampleStateCreateInfo(PL_SAMPLE_COUNT_1_BIT, 0),
				{PL_DYNAMIC_STATE_VIEWPORT, PL_DYNAMIC_STATE_SCISSOR}, {}));

		static ShadowPassUBO shadowPassUbo{};
		this->AddRenderPassCallback(
			"Shadow Pass", [&](PlazaRenderGraph* plazaRenderGraph, PlazaRenderPass* plazaRenderPass, Scene* scene) {
				if (mRenderer->mRendererSettings.mLightingSettings.mUpdateCascades)
					mRenderer->mRendererSettings.mLightingSettings.mShadowCascadeMatrices =
						GetShadowMatrices(mRenderer->mRendererSettings.mLightingSettings,
										  Application::Get()->activeCamera->GetProjectionMatrix(),
										  Application::Get()->activeCamera->GetViewMatrix());
				std::vector<glm::mat4> mats = mRenderer->mRendererSettings.mLightingSettings.mShadowCascadeMatrices;
				for (int i = 0; i < mRenderer->mRendererSettings.mLightingSettings.mCascadeCount; ++i) {
					if (shadowPassUbo.lightSpaceMatrices->length() > i && mats.size() > i)
						shadowPassUbo.lightSpaceMatrices[i] = mats[i];
					else
						shadowPassUbo.lightSpaceMatrices[i] = glm::mat4(1.0f);
				}
				plazaRenderGraph->GetSharedBuffer("ShadowPassUBO")
					->UpdateData<ShadowPassUBO>(Application::Get()->mRenderer->mCurrentFrame, shadowPassUbo);
			});

		glm::vec2 gPassSize = Application::Get()->appSizes->sceneSize;
		this->AddRenderPass("Deferred Geometry Pass", PL_STAGE_VERTEX | PL_STAGE_FRAGMENT,
							PL_RENDER_PASS_INDIRECT_BUFFER, glm::vec2(gPassSize.x, gPassSize.y), true)
			->AddInputBuffer(1, 0, PlBufferType::PL_BUFFER_UNIFORM_BUFFER, PL_STAGE_ALL,
							 this->GetSharedBuffer("GPassUBO"))
			->AddInputTexture(mRenderer->mMaxBindlessTextures, 0, 20, PL_BUFFER_COMBINED_IMAGE_SAMPLER,
							  PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0,
							  this->GetSharedTexture("TexturesBuffer"))
			->AddInputBuffer(1, 19, PL_BUFFER_STORAGE_BUFFER, PL_STAGE_VERTEX, this->GetSharedBuffer("MaterialsBuffer"))
			->AddInputBuffer(1, 1, PL_BUFFER_STORAGE_BUFFER, PL_STAGE_VERTEX,
							 this->GetSharedBuffer("BoneMatricesBuffer"))
			->AddInputBuffer(1, 2, PL_BUFFER_STORAGE_BUFFER, PL_STAGE_VERTEX,
							 this->GetSharedBuffer("RenderGroupOffsetsBuffer"))
			->AddInputBuffer(1, 3, PL_BUFFER_STORAGE_BUFFER, PL_STAGE_VERTEX,
							 this->GetSharedBuffer("RenderGroupMaterialsOffsetsBuffer"))
			->AddInputTexture(1, 0, 7, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT,
							  PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("CubeMapTexture"))
			->AddInputTexture(1, 0, 8, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT,
							  PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0,
							  this->GetSharedTexture("EquirectangularTexture"))
			//->AddOutputTexture(1, 0, 0, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT,
			// PL_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0, 0, this->GetSharedTexture("GPosition"))
			->AddOutputTexture(1, 0, 0, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
							   0, 0, this->GetSharedTexture("GNormal"))
			->AddOutputTexture(1, 1, 0, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
							   0, 0, this->GetSharedTexture("GDiffuse"))
			->AddOutputTexture(1, 2, 0, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
							   0, 0, this->GetSharedTexture("GOthers"))
			->AddOutputTexture(1, 3, 0, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT,
							   PL_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 0, 0,
							   this->GetSharedTexture("SceneDepth"));

		this->GetRenderPass("Deferred Geometry Pass")
			->AddPipeline(pl::pipelineCreateInfo(
				"MainShaders", PL_RENDER_PASS_INDIRECT_BUFFER,
				{pl::pipelineShaderStageCreateInfo(
					 PL_STAGE_VERTEX,
					 FilesManager::sEngineFolder.string() + "/Shaders/Vulkan/deferred/geometryPass.vert", "main"),
				 pl::pipelineShaderStageCreateInfo(
					 PL_STAGE_FRAGMENT,
					 FilesManager::sEngineFolder.string() + "/Shaders/Vulkan/deferred/geometryPass.frag", "main")},
				VertexGetBindingDescription(), VertexGetAttributeDescriptions(), PL_TOPOLOGY_TRIANGLE_LIST, false,
				pl::pipelineRasterizationStateCreateInfo(
					false, false, mRenderer->mShowWireframe ? PL_POLYGON_MODE_LINE : PL_POLYGON_MODE_FILL, 1.0f, false,
					0.0f, 0.0f, 0.0f, PL_CULL_MODE_BACK, PL_FRONT_FACE_COUNTER_CLOCKWISE),
				pl::pipelineColorBlendStateCreateInfo({pl::pipelineColorBlendAttachmentState(true),
													   pl::pipelineColorBlendAttachmentState(true),
													   pl::pipelineColorBlendAttachmentState(true)}),
				pl::pipelineDepthStencilStateCreateInfo(true, true, PL_COMPARE_OP_LESS_OR_EQUAL),
				pl::pipelineViewportStateCreateInfo(1, 1),
				pl::pipelineMultisampleStateCreateInfo(PL_SAMPLE_COUNT_1_BIT, 0),
				{PL_DYNAMIC_STATE_VIEWPORT, PL_DYNAMIC_STATE_SCISSOR}, {}));

		this->GetRenderPass("Deferred Geometry Pass")
			->AddPipeline(pl::pipelineCreateInfo(
				"Skinned", PL_RENDER_PASS_INDIRECT_BUFFER_SKINNED,
				{pl::pipelineShaderStageCreateInfo(PL_STAGE_VERTEX,
												   FilesManager::sEngineFolder.string() +
													   "/Shaders/Vulkan/deferred/geometrySkinnedPass.vert",
												   "main"),
				 pl::pipelineShaderStageCreateInfo(
					 PL_STAGE_FRAGMENT,
					 FilesManager::sEngineFolder.string() + "/Shaders/Vulkan/deferred/geometryPass.frag", "main")},
				VertexGetBindingDescription(), SkinnedVertexGetAttributeDescriptions(), PL_TOPOLOGY_TRIANGLE_LIST,
				false,
				pl::pipelineRasterizationStateCreateInfo(false, false, PL_POLYGON_MODE_FILL, 1.0f, false, 0.0f, 0.0f,
														 0.0f, PL_CULL_MODE_BACK, PL_FRONT_FACE_COUNTER_CLOCKWISE),
				pl::pipelineColorBlendStateCreateInfo({pl::pipelineColorBlendAttachmentState(true),
													   pl::pipelineColorBlendAttachmentState(true),
													   pl::pipelineColorBlendAttachmentState(true)}),
				pl::pipelineDepthStencilStateCreateInfo(true, true, PL_COMPARE_OP_LESS_OR_EQUAL),
				pl::pipelineViewportStateCreateInfo(1, 1),
				pl::pipelineMultisampleStateCreateInfo(PL_SAMPLE_COUNT_1_BIT, 0),
				{PL_DYNAMIC_STATE_VIEWPORT, PL_DYNAMIC_STATE_SCISSOR}, {}));

		struct DeferredGeometrySkyboxPC {
			glm::mat4 projection;
			glm::mat4 view;
			float skyboxIntensity;
			float gamma;
			float exposure;
			float useless;
		};

		uint64_t skyboxMeshUuid = 2;
		this->GetRenderPass("Deferred Geometry Pass")
			->AddPipeline(pl::pipelineCreateInfo(
				"Skybox", PL_RENDER_PASS_INDIRECT_BUFFER_SPECIFIC_MESH,
				{pl::pipelineShaderStageCreateInfo(
					 PL_STAGE_VERTEX, FilesManager::sEngineFolder.string() + "/Shaders/Vulkan/skybox/skybox.vert",
					 "main"),
				 pl::pipelineShaderStageCreateInfo(
					 PL_STAGE_FRAGMENT, FilesManager::sEngineFolder.string() + "/Shaders/Vulkan/skybox/skybox.frag",
					 "main")},
				VertexGetBindingDescription(), VertexGetAttributeDescriptions(), PL_TOPOLOGY_TRIANGLE_LIST, false,
				pl::pipelineRasterizationStateCreateInfo(false, false, PL_POLYGON_MODE_FILL, 1.0f, false, 0.0f, 0.0f,
														 0.0f, PL_CULL_MODE_NONE, PL_FRONT_FACE_CLOCKWISE),
				pl::pipelineColorBlendStateCreateInfo({pl::pipelineColorBlendAttachmentState(true),
													   pl::pipelineColorBlendAttachmentState(true),
													   pl::pipelineColorBlendAttachmentState(true)}),
				pl::pipelineDepthStencilStateCreateInfo(true, false, PL_COMPARE_OP_LESS_OR_EQUAL),
				pl::pipelineViewportStateCreateInfo(1, 1),
				pl::pipelineMultisampleStateCreateInfo(PL_SAMPLE_COUNT_1_BIT, 0),
				{PL_DYNAMIC_STATE_VIEWPORT, PL_DYNAMIC_STATE_SCISSOR},
				{pl::pushConstantRange(PL_STAGE_ALL, 0, sizeof(DeferredGeometrySkyboxPC))}, {skyboxMeshUuid}));

		this->AddRenderPassCallback("Deferred Geometry Pass", [&](PlazaRenderGraph* plazaRenderGraph,
																  PlazaRenderPass* plazaRenderPass, Scene* scene) {
			static UniformBufferObject ubo{};
			ubo.projection = Application::Get()->activeCamera->GetProjectionMatrix();
			ubo.view = Application::Get()->activeCamera->GetViewMatrix();
			ubo.model = glm::mat4(1.0f);

			ubo.cascadeCount = 9;
			ubo.farPlane = 15000.0f;
			ubo.nearPlane = 0.01f;

			glm::vec3 lightDir = mRenderer->mRendererSettings.mLightingSettings.mLightDirection;
			glm::vec3 lightDistance = glm::vec3(100.0f, 400.0f, 0.0f);
			glm::vec3 lightPos;

			ubo.lightDirection = glm::vec4(lightDir, 1.0f);
			ubo.viewPos = glm::vec4(Application::Get()->activeCamera->Position, 1.0f);

			ubo.directionalLightColor =
				glm::vec4(mRenderer->mRendererSettings.mLightingSettings.directionalLightColor *
						  mRenderer->mRendererSettings.mLightingSettings.directionalLightIntensity);
			ubo.directionalLightColor.w = mRenderer->mRendererSettings.mLightingSettings.directionalLightIntensity;
			ubo.ambientLightColor = glm::vec4(mRenderer->mRendererSettings.mLightingSettings.ambientLightColor *
											  mRenderer->mRendererSettings.mLightingSettings.ambientLightIntensity);
			ubo.gamma = mRenderer->gamma;

			for (int i = 0; i < mRenderer->mRendererSettings.mLightingSettings.mCascadeCount; ++i) {
				ubo.lightSpaceMatrices[i] = shadowPassUbo.lightSpaceMatrices[i];
				ubo.cascadePlaneDistances[i] =
					glm::vec4(mRenderer->mRendererSettings.mLightingSettings.shadowCascadeLevels[i], 1.0f, 1.0f, 1.0f);
			}

			ubo.showCascadeLevels = Application::Get()->showCascadeLevels;
			plazaRenderGraph->GetSharedBuffer("GPassUBO")
				->UpdateData<UniformBufferObject>(Application::Get()->mRenderer->mCurrentFrame, ubo);

			plazaRenderPass->mPipelines[2]->UpdatePushConstants<DeferredGeometrySkyboxPC>(
				0, DeferredGeometrySkyboxPC(Application::Get()->activeCamera->GetProjectionMatrix(),
											Application::Get()->activeCamera->GetViewMatrix(),
											mRenderer->mSkyboxIntensity, mRenderer->gamma, mRenderer->exposure, 0.0f));
		});

		this->GetRenderPass("Deferred Geometry Pass")
			->GetInputResource<PlazaShadersBinding>("TexturesBuffer")
			->mMaxBindlessResources = mRenderer->mMaxBindlessTextures;

		// Lights sorter
		struct LightSorterPC {
			glm::mat4 view;
			glm::mat4 projection;
			int lightCount;
			bool first;
			glm::vec2 screenSize;
			glm::vec2 clusterSize;
		};

		this->AddRenderPass("Light Sorter Pass", PL_STAGE_COMPUTE, PL_RENDER_PASS_COMPUTE, gPassSize, false)
			->AddInputTexture(1, 0, 3, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_COMPUTE,
							  PL_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, 0, 0,
							  this->GetSharedTexture("SceneDepth"))
			->AddOutputBuffer(1, 0, PL_BUFFER_STORAGE_BUFFER, PL_STAGE_COMPUTE, this->GetSharedBuffer("LightsBuffer"))
			->AddOutputBuffer(1, 1, PL_BUFFER_STORAGE_BUFFER, PL_STAGE_COMPUTE, this->GetSharedBuffer("ClustersBuffer"))
			->AddOutputBuffer(1, 8, PL_BUFFER_STORAGE_BUFFER, PL_STAGE_COMPUTE,
							  this->GetSharedBuffer("TilesDepthBuffer"))
			->AddPipeline(pl::pipelineCreateInfo(
				"LightSorter", PL_RENDER_PASS_COMPUTE,
				{pl::pipelineShaderStageCreateInfo(
					PL_STAGE_COMPUTE,
					FilesManager::sEngineFolder.string() + "/Shaders/Vulkan/lighting/lightSorter.comp", "main")},
				{}, {}, {}, {}, {}, {}, {}, {}, {}, {},
				{pl::pushConstantRange(PL_STAGE_COMPUTE, 0, sizeof(LightSorterPC))}));

		this->AddRenderPassCallback(
			"Light Sorter Pass",
			[&, gPassSize](PlazaRenderGraph* plazaRenderGraph, PlazaRenderPass* plazaRenderPass, Scene* scene) {
				Application::Get()->mThreadsManager->mFrameRendererAfterGeometry->Update();
				glm::vec2 clusterSize = glm::vec2(32.0f);
				glm::vec2 clusterCount = glm::ceil(gPassSize / clusterSize);

				plazaRenderPass->mPipelines[0]->UpdatePushConstants<LightSorterPC>(
					0, LightSorterPC(Application::Get()->activeCamera->GetViewMatrix(),
									 Application::Get()->activeCamera->GetProjectionMatrix(),
									 mRenderer->mRendererSettings.mLightingSettings.mLightsCount, true, gPassSize,
									 clusterSize));
				plazaRenderPass->mDispatchSize = glm::vec3(clusterCount.x, clusterCount.y, 1);
			});

		// Deferred Lighting
		this->AddRenderPass("Deferred Lighting Pass", PL_STAGE_VERTEX | PL_STAGE_FRAGMENT,
							PL_RENDER_PASS_FULL_SCREEN_QUAD, gPassSize, false)
			->AddInputBuffer(1, 15, PlBufferType::PL_BUFFER_UNIFORM_BUFFER, PL_STAGE_FRAGMENT,
							 this->GetSharedBuffer("DeferredPassUBO"))
			->AddInputTexture(1, 0, 6, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT,
							  PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("SamplerBRDFLUT"))
			->AddInputTexture(1, 0, 7, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT,
							  PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("PreFilterMap"))
			->AddInputTexture(1, 0, 8, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT,
							  PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("IrradianceMap"))
			->AddInputTexture(1, 0, 9, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT,
							  PL_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, 0, 0,
							  this->GetSharedTexture("ShadowsDepthMap"))
			->AddInputTexture(1, 0, 10, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT,
							  PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0,
							  this->GetSharedTexture("EquirectangularTexture"))
			->AddInputTexture(1, 0, 0, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT,
							  PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("GNormal"))
			->AddInputTexture(1, 0, 1, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT,
							  PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("GDiffuse"))
			->AddInputTexture(1, 0, 2, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT,
							  PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("GOthers"))
			->AddInputTexture(1, 0, 3, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT,
							  PL_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, 0, 0,
							  this->GetSharedTexture("SceneDepth"))
			->AddInputBuffer(1, 4, PL_BUFFER_STORAGE_BUFFER, PL_STAGE_FRAGMENT, this->GetSharedBuffer("LightsBuffer"))
			->AddInputBuffer(1, 5, PL_BUFFER_STORAGE_BUFFER, PL_STAGE_FRAGMENT, this->GetSharedBuffer("ClustersBuffer"))
			->AddOutputTexture(1, 0, 0, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
							   0, 0, this->GetSharedTexture("SceneTexture"));

		this->GetRenderPass("Deferred Lighting Pass")
			->AddPipeline(pl::pipelineCreateInfo(
				"LightingPassShaders", PL_RENDER_PASS_FULL_SCREEN_QUAD,
				{pl::pipelineShaderStageCreateInfo(
					 PL_STAGE_VERTEX,
					 FilesManager::sEngineFolder.string() + "/Shaders/Vulkan/lighting/deferredPass.vert", "main"),
				 pl::pipelineShaderStageCreateInfo(
					 PL_STAGE_FRAGMENT,
					 FilesManager::sEngineFolder.string() + "/Shaders/Vulkan/lighting/deferredPass.frag", "main")},
				{}, {}, PL_TOPOLOGY_TRIANGLE_LIST, false,
				pl::pipelineRasterizationStateCreateInfo(false, false, PL_POLYGON_MODE_FILL, 1.0f, false, 0.0f, 0.0f,
														 0.0f, PL_CULL_MODE_NONE, PL_FRONT_FACE_COUNTER_CLOCKWISE),
				pl::pipelineColorBlendStateCreateInfo({pl::pipelineColorBlendAttachmentState(true)}),
				pl::pipelineDepthStencilStateCreateInfo(false, false, PL_COMPARE_OP_ALWAYS),
				pl::pipelineViewportStateCreateInfo(1, 1),
				pl::pipelineMultisampleStateCreateInfo(PL_SAMPLE_COUNT_1_BIT, 0),
				{PL_DYNAMIC_STATE_VIEWPORT, PL_DYNAMIC_STATE_SCISSOR}, {}));

		this->AddRenderPassCallback("Deferred Lighting Pass", [&](PlazaRenderGraph* plazaRenderGraph,
																  PlazaRenderPass* plazaRenderPass, Scene* scene) {
			static DeferredLightingPassUbo ubo{};
			ubo.projection = Application::Get()->activeCamera->GetProjectionMatrix();
			ubo.view = Application::Get()->activeCamera->GetViewMatrix();

			ubo.cascadeCount = 9;
			ubo.farPlane = 15000.0f;
			ubo.nearPlane = 0.01f;

			glm::vec3 lightDir = mRenderer->mRendererSettings.mLightingSettings.mLightDirection;
			glm::vec3 lightDistance = glm::vec3(100.0f, 400.0f, 0.0f);
			glm::vec3 lightPos;

			ubo.lightDirection = glm::vec4(lightDir, 1.0f);
			ubo.viewPos = glm::vec4(Application::Get()->activeCamera->Position, 1.0f);

			ubo.directionalLightColor =
				glm::vec4(mRenderer->mRendererSettings.mLightingSettings.directionalLightColor *
						  mRenderer->mRendererSettings.mLightingSettings.directionalLightIntensity);
			ubo.ambientLightColor = glm::vec4(mRenderer->mRendererSettings.mLightingSettings.ambientLightColor *
											  mRenderer->mRendererSettings.mLightingSettings.ambientLightIntensity);
			ubo.gamma = mRenderer->gamma;
			ubo.exposure = mRenderer->exposure;

			for (int i = 0; i < 16; ++i) {
				ubo.lightSpaceMatrices[i] = shadowPassUbo.lightSpaceMatrices[i];
				if (i <= 8)
					ubo.cascadePlaneDistances[i] = glm::vec4(
						mRenderer->mRendererSettings.mLightingSettings.shadowCascadeLevels[i], 1.0f, 1.0f, 1.0f);
				else
					ubo.cascadePlaneDistances[i] = glm::vec4(
						mRenderer->mRendererSettings.mLightingSettings.shadowCascadeLevels[8], 1.0f, 1.0f, 1.0f);
			}

			ubo.showCascadeLevels = Application::Get()->showCascadeLevels;

			ubo.lightCount = mRenderer->mRendererSettings.mLightingSettings.mLightsCount;
			plazaRenderGraph->GetSharedBuffer("DeferredPassUBO")
				->UpdateData<DeferredLightingPassUbo>(Application::Get()->mRenderer->mCurrentFrame, ubo);
		});

		/* Bloom */
		struct BloomPassPC {
			glm::vec4 u_threshold;
			glm::vec2 u_texel_size;
			int u_mip_level;
			int u_use_threshold;
			float u_bloom_intensity;
		};
		this->AddRenderPass("Bloom Pass", PL_STAGE_COMPUTE, PL_RENDER_PASS_HOLDER, gPassSize, false)
			->AddInputTexture(1, 0, 0, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_COMPUTE,
							  PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("BloomTexture"))
			->AddInputTexture(1, 0, 1, PL_BUFFER_STORAGE_IMAGE, PL_STAGE_COMPUTE, PL_IMAGE_LAYOUT_GENERAL, 0, 0,
							  this->GetSharedTexture("BloomTexture"))
			->AddInputTexture(1, 0, 2, PL_BUFFER_STORAGE_IMAGE, PL_STAGE_COMPUTE, PL_IMAGE_LAYOUT_GENERAL, 0, 0,
							  this->GetSharedTexture("SceneTexture"))
			->AddOutputTexture(1, 0, 0, PL_BUFFER_STORAGE_IMAGE, PL_STAGE_COMPUTE, PL_IMAGE_LAYOUT_GENERAL, 0, 0,
							   this->GetSharedTexture("BloomTexture"))
			->SetRecordingCallback(
				[&](PlazaRenderGraph* plazaRenderGraph, PlazaRenderPass* plazaRenderPass, Scene* scene) {
					VulkanTexture* texture = this->GetTexture<VulkanTexture>("SceneTexture");
					mRenderer->CopyTexture(this->GetTexture<VulkanTexture>("SceneTexture"),
										   this->GetTexture<VulkanTexture>("BloomTexture"), PL_IMAGE_LAYOUT_GENERAL);
				});

		uint32_t downScaleLimit = 10;
		uint32_t width = gPassSize.x / 2;
		uint32_t height = gPassSize.y / 2;
		uint8_t bloomMipCount = 1;
		for (uint8_t i = 0; i < 12; ++i) {
			width = width / 2;
			height = height / 2;
			if (width < downScaleLimit || height < downScaleLimit)
				break;
			++bloomMipCount;
		}
		bool pingPong = true;
		PlPipelineShaderStageCreateInfo downScaleShaders = pl::pipelineShaderStageCreateInfo(
			PL_STAGE_COMPUTE,
			VulkanShadersCompiler::Compile(FilesManager::sEngineFolder.string() +
										   "/Shaders/Vulkan/bloom/bloomDownScale.comp"),
			"main");
		PlPipelineShaderStageCreateInfo upScaleShaders =
			pl::pipelineShaderStageCreateInfo(PL_STAGE_COMPUTE,
											  VulkanShadersCompiler::Compile(FilesManager::sEngineFolder.string() +
																			 "/Shaders/Vulkan/bloom/bloomUpScale.comp"),
											  "main");
		PlPipelineCreateInfo bloomPipelineCreateInfo =
			pl::pipelineCreateInfo("BloomShaders", PL_RENDER_PASS_COMPUTE, {downScaleShaders}, {}, {}, {}, {}, {}, {},
								   {}, {}, {}, {}, {pl::pushConstantRange(PL_STAGE_COMPUTE, 0, sizeof(BloomPassPC))});
		std::shared_ptr<PlazaPipeline> bloomDownScalePipeline =
			this->GetRenderPass("Bloom Pass")->AddPipeline(bloomPipelineCreateInfo);

		glm::uvec2 mipSize = glm::uvec2(gPassSize.x / 2, gPassSize.y / 2);
		// DownScale
		for (unsigned int i = 0; i < bloomMipCount - 1; ++i) {
			this->GetRenderPass("Bloom Pass")
				->AddChildPass("Bloom Pass " + std::to_string(i) + " DownScale", PL_STAGE_COMPUTE,
							   PL_RENDER_PASS_COMPUTE, gPassSize, false)
				->AddInputTexture(1, 0, 0, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_COMPUTE, PL_IMAGE_LAYOUT_GENERAL,
								  i, 0, this->GetSharedTexture("BloomTexture"))
				->AddInputTexture(1, 0, 1, PL_BUFFER_STORAGE_IMAGE, PL_STAGE_COMPUTE, PL_IMAGE_LAYOUT_GENERAL, i + 1, 0,
								  this->GetSharedTexture("BloomTexture"))
				->AddPipeline(bloomDownScalePipeline);
			this->GetRenderPass("Bloom Pass")
				->mChildPasses.back()
				->SetRecordingCallback([&, mipSize, i](PlazaRenderGraph* plazaRenderGraph,
													   PlazaRenderPass* plazaRenderPass, Scene* scene) {
					float threshold = mRenderer->mRendererSettings.mBloomSettings.mThreshold;
					float knee = mRenderer->mRendererSettings.mBloomSettings.mKnee;
					BloomPassPC constant{};
					constant.u_texel_size = 1.0f / glm::vec2(mipSize);
					constant.u_mip_level = i;
					constant.u_threshold = glm::vec4(threshold, threshold - knee, 2.0f * knee, 0.25f * knee);
					constant.u_use_threshold = i == 0 ? 1 : 0;
					constant.u_bloom_intensity = mRenderer->mRendererSettings.mBloomSettings.mBloomIntensity;
					plazaRenderPass->mDispatchSize =
						glm::vec3(glm::ceil(float(mipSize.x) / 8), glm::ceil(float(mipSize.y) / 8), 1);
					plazaRenderPass->mPipelines[0]->UpdatePushConstants<BloomPassPC>(
						0, BloomPassPC(constant.u_threshold, constant.u_texel_size, constant.u_mip_level,
									   constant.u_use_threshold, constant.u_bloom_intensity));
				});

			mipSize = mipSize / 2u;
		}
		// UpScale
		bloomPipelineCreateInfo.shaderStages = {upScaleShaders};
		std::shared_ptr<PlazaPipeline> bloomUpScalePipeline =
			this->GetRenderPass("Bloom Pass")->AddPipeline(bloomPipelineCreateInfo);
		for (uint8_t i = bloomMipCount - 1; i >= 1; --i) {
			mipSize.x =
				glm::max(1.0, glm::floor(float(Application::Get()->appSizes->sceneSize.x) / glm::pow(2.0, i - 1)));
			mipSize.y =
				glm::max(1.0, glm::floor(float(Application::Get()->appSizes->sceneSize.y) / glm::pow(2.0, i - 1)));
			if (i == 0) {
				this->GetRenderPass("Bloom Pass")
					->AddChildPass("Bloom Pass " + std::to_string(i) + " UpScale", PL_STAGE_COMPUTE,
								   PL_RENDER_PASS_COMPUTE, gPassSize, false)
					->AddInputTexture(1, 0, 0, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_COMPUTE,
									  PL_IMAGE_LAYOUT_GENERAL, 1, 0, this->GetSharedTexture("BloomTexture"))
					->AddInputTexture(1, 0, 1, PL_BUFFER_STORAGE_IMAGE, PL_STAGE_COMPUTE, PL_IMAGE_LAYOUT_GENERAL, i, 0,
									  this->GetSharedTexture("BloomTexture"))
					->AddPipeline(bloomUpScalePipeline);
			}
			else {
				this->GetRenderPass("Bloom Pass")
					->AddChildPass("Bloom Pass " + std::to_string(i) + " UpScale", PL_STAGE_COMPUTE,
								   PL_RENDER_PASS_COMPUTE, gPassSize, false)
					->AddInputTexture(1, 0, 0, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_COMPUTE,
									  PL_IMAGE_LAYOUT_GENERAL, i, 0, this->GetSharedTexture("BloomTexture"))
					->AddInputTexture(1, 0, 1, PL_BUFFER_STORAGE_IMAGE, PL_STAGE_COMPUTE, PL_IMAGE_LAYOUT_GENERAL,
									  i - 1, 0, this->GetSharedTexture("BloomTexture"))
					->AddPipeline(bloomUpScalePipeline);
			}
			this->GetRenderPass("Bloom Pass")
				->mChildPasses.back()
				->SetRecordingCallback([&, mipSize, i](PlazaRenderGraph* plazaRenderGraph,
													   PlazaRenderPass* plazaRenderPass, Scene* scene) {
					float threshold = mRenderer->mRendererSettings.mBloomSettings.mThreshold;
					float knee = mRenderer->mRendererSettings.mBloomSettings.mKnee;
					BloomPassPC constant{};
					constant.u_texel_size = 1.0f / glm::vec2(mipSize);
					constant.u_mip_level = i;
					constant.u_threshold = glm::vec4(threshold, threshold - knee, 2.0f * knee, 0.25f * knee);
					constant.u_use_threshold = i == 0 ? 1 : 0;
					constant.u_bloom_intensity = mRenderer->mRendererSettings.mBloomSettings.mBloomIntensity;
					plazaRenderPass->mDispatchSize =
						glm::vec3(glm::ceil(float(mipSize.x) / 8), glm::ceil(float(mipSize.y) / 8), 1);
					plazaRenderPass->mPipelines[0]->UpdatePushConstants<BloomPassPC>(
						0, BloomPassPC(constant.u_threshold, constant.u_texel_size, constant.u_mip_level,
									   constant.u_use_threshold, constant.u_bloom_intensity));
				});
		}

		// Screen Space reflections
		this->AddRenderPass("Screen Space Reflections Pass", PL_STAGE_VERTEX | PL_STAGE_FRAGMENT,
							PL_RENDER_PASS_FULL_SCREEN_QUAD, gPassSize, false)
			->AddInputTexture(1, 0, 0, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT,
							  PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("GNormal"))
			->AddInputTexture(1, 0, 1, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_GENERAL, 0,
							  0, this->GetSharedTexture("BloomTexture"))
			->AddInputTexture(1, 0, 2, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT,
							  PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("GOthers"))
			->AddInputTexture(1, 0, 3, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT,
							  PL_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, 0, 0,
							  this->GetSharedTexture("SceneDepth"))
			->AddOutputTexture(1, 0, 0, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
							   0, 0, this->GetSharedTexture("SSRTexture"));

		struct SSRPushConstants {
			glm::vec4 screenSize;
			glm::vec4 cameraPos;
			glm::mat4 projection;
			glm::mat4 view;
			glm::mat4 lensProjection;
		};
		this->GetRenderPass("Screen Space Reflections Pass")
			->AddPipeline(pl::pipelineCreateInfo(
				"ScreenSpaceReflectionsShaders", PL_RENDER_PASS_FULL_SCREEN_QUAD,
				{pl::pipelineShaderStageCreateInfo(
					 PL_STAGE_VERTEX, FilesManager::sEngineFolder.string() + "/Shaders/Vulkan/ssr/ssr.vert", "main"),
				 pl::pipelineShaderStageCreateInfo(
					 PL_STAGE_FRAGMENT, FilesManager::sEngineFolder.string() + "/Shaders/Vulkan/ssr/ssr.frag", "main")},
				{}, {}, PL_TOPOLOGY_TRIANGLE_LIST, false,
				pl::pipelineRasterizationStateCreateInfo(false, false, PL_POLYGON_MODE_FILL, 1.0f, false, 0.0f, 0.0f,
														 0.0f, PL_CULL_MODE_NONE, PL_FRONT_FACE_COUNTER_CLOCKWISE),
				pl::pipelineColorBlendStateCreateInfo({pl::pipelineColorBlendAttachmentState(true)}),
				pl::pipelineDepthStencilStateCreateInfo(false, false, PL_COMPARE_OP_ALWAYS),
				pl::pipelineViewportStateCreateInfo(1, 1),
				pl::pipelineMultisampleStateCreateInfo(PL_SAMPLE_COUNT_1_BIT, 0),
				{PL_DYNAMIC_STATE_VIEWPORT, PL_DYNAMIC_STATE_SCISSOR},
				{pl::pushConstantRange(PL_STAGE_FRAGMENT, 0, sizeof(SSRPushConstants))}, {}));
		this->AddRenderPassCallback(
			"Screen Space Reflections Pass",
			[&](PlazaRenderGraph* plazaRenderGraph, PlazaRenderPass* plazaRenderPass, Scene* scene) {
				plazaRenderPass->mPipelines[0]->UpdatePushConstants<SSRPushConstants>(
					0, SSRPushConstants(glm::vec4(Application::Get()->appSizes->sceneSize, 0.0f, 1.0f),
										glm::vec4(Application::Get()->activeCamera->Position, 1.0f),
										Application::Get()->activeCamera->GetProjectionMatrix(),
										Application::Get()->activeCamera->GetViewMatrix(),
										Application::Get()->activeCamera->GetProjectionMatrix()));
			});

		// Get scene luminance
		this->AddRenderPass("Luminance Pass", PL_STAGE_COMPUTE, PL_RENDER_PASS_COMPUTE, gPassSize, false)
			->AddInputTexture(1, 0, 0, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_COMPUTE,
							  PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0,
							  this->GetSharedTexture("SceneDownSampledTexture"))
			->AddInputTexture(1, 0, 1, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_COMPUTE,
							  PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("SSRTexture"))
			->AddInputTexture(1, 0, 2, PL_BUFFER_STORAGE_IMAGE, PL_STAGE_COMPUTE, PL_IMAGE_LAYOUT_GENERAL, 0, 0,
							  this->GetSharedTexture("LuminanceTexture"), PL_ATTACHMENT_OP_AUTO)
			->SetRecordingCallback(
				[&](PlazaRenderGraph* plazaRenderGraph, PlazaRenderPass* plazaRenderPass, Scene* scene) {
					// VulkanTexture* texture = this->GetTexture<VulkanTexture>("SSRTexture");
					// VulkanTexture* downSampledTexture = this->GetTexture<VulkanTexture>("SceneDownSampledTexture");
					//
					// mRenderer->TransitionImageLayout(
					//	texture->mImage, PlImageFormatToVkFormat(texture->GetTextureInfo().mFormat),
					//	PlImageLayoutToVkImageLayout(PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
					//	PlImageLayoutToVkImageLayout(PL_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL), 1, 1, 1, false,
					//	*mCommandBuffer);
					//
					// mRenderer->CopyDifferentSizeTexture(
					//	texture, PlImageLayoutToVkImageLayout(PL_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL),
					//	this->GetTexture<VulkanTexture>("SceneDownSampledTexture"),
					//	PlImageLayoutToVkImageLayout(PL_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL), // FIXED
					//	*mCommandBuffer);
					//
					// mRenderer->TransitionImageLayout(
					//	texture->mImage, PlImageFormatToVkFormat(texture->GetTextureInfo().mFormat),
					//	PlImageLayoutToVkImageLayout(PL_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL),
					//	PlImageLayoutToVkImageLayout(PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL), 1, 1, 1, false,
					//	*mCommandBuffer);
					//
					// mRenderer->TransitionImageLayout(
					//	downSampledTexture->mImage,
					//PlImageFormatToVkFormat(downSampledTexture->GetTextureInfo().mFormat),
					//	PlImageLayoutToVkImageLayout(PL_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL),
					//	PlImageLayoutToVkImageLayout(PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL), 1, 1, 1, false,
					//	*mCommandBuffer);
					//
					// plazaRenderPass->mDispatchSize = glm::vec3(1, 1, 1);
				});

		struct LuminancePassPC {};
		PlPipelineCreateInfo luminancePipelineCreateInfo = pl::pipelineCreateInfo(
			"LuminanceAveragerShaders", PL_RENDER_PASS_COMPUTE,
			{pl::pipelineShaderStageCreateInfo(
				PL_STAGE_COMPUTE,
				FilesManager::sEngineFolder.string() + "/Shaders/Vulkan/luminance/luminanceAverager.comp", "main")},
			{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {});
		this->GetRenderPass("Luminance Pass")->AddPipeline(luminancePipelineCreateInfo);

		// Final Post Processing
		this->AddRenderPass("Final Post Processing Pass", PL_STAGE_VERTEX | PL_STAGE_FRAGMENT,
							PL_RENDER_PASS_FULL_SCREEN_QUAD, gPassSize, false)
			->AddInputTexture(1, 0, 0, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT,
							  PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("SSRTexture"))
			->AddInputTexture(1, 0, 1, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT,
							  PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("FontTexture"))
			->AddInputTexture(1, 0, 2, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT,
							  PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0,
							  this->GetSharedTexture("LuminanceTexture"))
			->AddOutputTexture(1, 0, 0, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
							   0, 0, this->GetSharedTexture("FinalTexture"), PL_ATTACHMENT_OP_LOAD);

		struct FinalPostProcessingPC {
			float exposure;
			float gamma;
		};
		this->GetRenderPass("Final Post Processing Pass")
			->AddPipeline(pl::pipelineCreateInfo(
				"FinalShaders", PL_RENDER_PASS_FULL_SCREEN_QUAD,
				{pl::pipelineShaderStageCreateInfo(
					 PL_STAGE_VERTEX, FilesManager::sEngineFolder.string() + "/Shaders/Vulkan/swapchainDraw.vert",
					 "main"),
				 pl::pipelineShaderStageCreateInfo(
					 PL_STAGE_FRAGMENT, FilesManager::sEngineFolder.string() + "/Shaders/Vulkan/swapchainDraw.frag",
					 "main")},
				{}, {}, PL_TOPOLOGY_TRIANGLE_LIST, false,
				pl::pipelineRasterizationStateCreateInfo(false, false, PL_POLYGON_MODE_FILL, 1.0f, false, 0.0f, 0.0f,
														 0.0f, PL_CULL_MODE_NONE, PL_FRONT_FACE_COUNTER_CLOCKWISE),
				pl::pipelineColorBlendStateCreateInfo({pl::pipelineColorBlendAttachmentState(true)}),
				pl::pipelineDepthStencilStateCreateInfo(false, false, PL_COMPARE_OP_ALWAYS),
				pl::pipelineViewportStateCreateInfo(1, 1),
				pl::pipelineMultisampleStateCreateInfo(PL_SAMPLE_COUNT_1_BIT, 0),
				{PL_DYNAMIC_STATE_VIEWPORT, PL_DYNAMIC_STATE_SCISSOR},
				{pl::pushConstantRange(PL_STAGE_FRAGMENT, 0, sizeof(FinalPostProcessingPC))}));

		// Gui
		struct GuiShadersPC {
			glm::mat4 matrix;
		};
		PlPipelineCreateInfo guiPipelineInfo = pl::pipelineCreateInfo(
			"GuiShaders", PL_RENDER_PASS_GUI_RECTANGLE,
			{pl::pipelineShaderStageCreateInfo(
				 PL_STAGE_VERTEX, FilesManager::sEngineFolder.string() + "/Shaders/Vulkan/gui/rectangle.vert", "main"),
			 pl::pipelineShaderStageCreateInfo(
				 PL_STAGE_FRAGMENT, FilesManager::sEngineFolder.string() + "/Shaders/Vulkan/gui/rectangle.frag",
				 "main")},
			VertexGetBindingDescription(), VertexGetAttributeDescriptions(), PL_TOPOLOGY_TRIANGLE_LIST, false,
			pl::pipelineRasterizationStateCreateInfo(false, false, PL_POLYGON_MODE_FILL, 1.0f, false, 0.0f, 0.0f, 0.0f,
													 PL_CULL_MODE_NONE, PL_FRONT_FACE_COUNTER_CLOCKWISE),
			pl::pipelineColorBlendStateCreateInfo({pl::pipelineColorBlendAttachmentState(
				false, PL_BLEND_FACTOR_SRC_ALPHA, PL_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, PL_BLEND_OP_ADD,
				PL_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, PL_BLEND_FACTOR_ONE)}),
			pl::pipelineDepthStencilStateCreateInfo(false, false, PL_COMPARE_OP_ALWAYS),
			pl::pipelineViewportStateCreateInfo(1, 1), pl::pipelineMultisampleStateCreateInfo(PL_SAMPLE_COUNT_1_BIT, 0),
			{PL_DYNAMIC_STATE_VIEWPORT, PL_DYNAMIC_STATE_SCISSOR},
			{pl::pushConstantRange(PL_STAGE_ALL, 0, sizeof(GuiShadersPC))}, {1});
		this->GetRenderPass("Final Post Processing Pass")->AddPipeline(guiPipelineInfo);
		guiPipelineInfo.renderMethod = PL_RENDER_PASS_GUI_BUTTON;
		guiPipelineInfo.shaderStages = {
			pl::pipelineShaderStageCreateInfo(
				PL_STAGE_VERTEX, FilesManager::sEngineFolder.string() + "/Shaders/Vulkan/gui/button.vert", "main"),
			pl::pipelineShaderStageCreateInfo(
				PL_STAGE_FRAGMENT, FilesManager::sEngineFolder.string() + "/Shaders/Vulkan/gui/button.frag", "main")};
		this->GetRenderPass("Final Post Processing Pass")->AddPipeline(guiPipelineInfo);

		guiPipelineInfo.depthStencilState = pl::pipelineDepthStencilStateCreateInfo(false, false, PL_COMPARE_OP_ALWAYS);
		guiPipelineInfo.topology = PL_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		guiPipelineInfo.vertexBindingDescriptions = {
			pl::vertexInputBindingDescription(0, sizeof(glm::vec4), PL_VERTEX_INPUT_RATE_VERTEX),
			pl::vertexInputBindingDescription(1, sizeof(glm::vec4), PL_VERTEX_INPUT_RATE_VERTEX)};
		guiPipelineInfo.vertexAttributeDescriptions = {
			pl::vertexInputAttributeDescription(0, 0, PL_FORMAT_R32G32_SFLOAT, 0),
			pl::vertexInputAttributeDescription(1, 1, PL_FORMAT_R32G32_SFLOAT, sizeof(glm::vec2))};
		guiPipelineInfo.renderMethod = PL_RENDER_PASS_GUI_TEXT;
		guiPipelineInfo.shaderStages = {
			pl::pipelineShaderStageCreateInfo(
				PL_STAGE_VERTEX, FilesManager::sEngineFolder.string() + "/Shaders/Vulkan/gui/text.vert", "main"),
			pl::pipelineShaderStageCreateInfo(
				PL_STAGE_FRAGMENT, FilesManager::sEngineFolder.string() + "/Shaders/Vulkan/gui/text.frag", "main")};
		this->GetRenderPass("Final Post Processing Pass")->AddPipeline(guiPipelineInfo);

		this->AddRenderPassCallback("Final Post Processing Pass", [&](PlazaRenderGraph* plazaRenderGraph,
																	  PlazaRenderPass* plazaRenderPass, Scene* scene) {
			plazaRenderPass->mPipelines[0]->UpdatePushConstants<FinalPostProcessingPC>(
				0, FinalPostProcessingPC(mRenderer->exposure, mRenderer->gamma));
			plazaRenderPass->mPipelines[1]->UpdatePushConstants<GuiShadersPC>(
				0, GuiShadersPC(Application::Get()->activeCamera->GetOrthogonalMatrix()));
			plazaRenderPass->mPipelines[2]->UpdatePushConstants<GuiShadersPC>(
				0, GuiShadersPC(Application::Get()->activeCamera->GetOrthogonalMatrix()));
			plazaRenderPass->mPipelines[3]->UpdatePushConstants<GuiShadersPC>(
				0, GuiShadersPC(Application::Get()->activeCamera->GetOrthogonalMatrix()));
		});

		// Outline
		this->AddRenderPass("OutlineDrawPass", PL_STAGE_VERTEX | PL_STAGE_FRAGMENT,
							PL_RENDER_PASS_INDIRECT_BUFFER_SPECIFIC_ENTITY, gPassSize, true)
			->AddOutputTexture(1, 0, 0, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
							   0, 0, this->GetSharedTexture("OutlineTexture"))
			->AddOutputTexture(1, 1, 0, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT,
							   PL_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 0, 0,
							   this->GetSharedTexture("OutlineStencil"));
		struct OutlinePC {
			glm::mat4 projection;
			glm::mat4 view;
		};

		this->AddRenderPassCallback("OutlineDrawPass", [&](PlazaRenderGraph* plazaRenderGraph,
														   PlazaRenderPass* plazaRenderPass, Scene* scene) {
			plazaRenderPass->mPipelines[0]->UpdatePushConstants<OutlinePC>(
				0, OutlinePC(Application::Get()->activeCamera->GetProjectionMatrix(),
							 Application::Get()->activeCamera->GetViewMatrix()));

			if (!plazaRenderPass->mPipelines[0]->mIndirectBuffer) {
				std::shared_ptr<PlVkBuffer> buffer = std::make_shared<PlVkBuffer>();
				buffer->CreateBuffer(sizeof(VkDrawIndexedIndirectCommand) * 1024 * 16,
									 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
									 VMA_MEMORY_USAGE_CPU_TO_GPU, 0, Application::Get()->mRenderer->mMaxFramesInFlight);
				plazaRenderPass->mPipelines[0]->mIndirectBuffer = buffer;
				plazaRenderPass->mPipelines[0]->mVertexBuffers.push_back(
					std::make_shared<PlBufferAttachment>(plazaRenderGraph->GetSharedBuffer("OutlineMatrixBuffer"), 1));
			}

			plazaRenderPass->mPipelines[0]->mCreateInfo.specificUuids.clear();
			if (Editor::selectedGameObject && scene->GetEntity(Editor::selectedGameObject->uuid)) {
				GetChildrendUuid(plazaRenderPass->mPipelines[0]->mCreateInfo.specificUuids,
								scene->GetEntity(Editor::selectedGameObject->uuid), scene);
				if (plazaRenderPass->mPipelines[0]->mCreateInfo.specificUuids.size() >= maxOutlineMeshes) {
					PL_CORE_WARN("Trying to draw more than {} meshes outline! only the first {} meshes will be drawn",
								 maxOutlineMeshes, maxOutlineMeshes);
				}
			}

			std::vector<VkDrawIndexedIndirectCommand> indirectCommands;
			int totalInstances = 0;
			std::vector<glm::mat4> matrices = std::vector<glm::mat4>();
			for (const auto& uuid : plazaRenderPass->mPipelines[0]->mCreateInfo.specificUuids) {
				auto& value = *scene->GetComponent<MeshRenderer>(uuid)->renderGroup;
				const size_t& instanceCount = value.instanceModelMatrices.size();
				matrices.push_back(scene->GetComponent<TransformComponent>(uuid)->GetWorldMatrix());

				VkDrawIndexedIndirectCommand indirectCommand{};
				indirectCommand.firstIndex = value.mesh->indicesOffset;
				indirectCommand.vertexOffset = value.mesh->verticesOffset;
				indirectCommand.firstInstance = totalInstances;
				indirectCommand.indexCount = value.mesh->indicesCount;
				indirectCommand.instanceCount = 1;
				indirectCommands.push_back(indirectCommand);
				totalInstances++;
			}

			plazaRenderPass->mPipelines[0]->mIndirectBuffer->mCurrentItemCount = indirectCommands.size();
			if (indirectCommands.size() > 0)
				plazaRenderPass->mPipelines[0]->mIndirectBuffer->UpdateData<VkDrawIndexedIndirectCommand>(
					Application::Get()->mRenderer->mCurrentFrame, indirectCommands.data(), indirectCommands.size());

			if (matrices.size() > 0)
				plazaRenderGraph->GetBuffer<PlBuffer>("OutlineMatrixBuffer")
					->UpdateData<glm::mat4>(Application::Get()->mRenderer->mCurrentFrame, matrices.data(),
											matrices.size());
		});

		PlStencilOpState stencilOpState = PlStencilOpState(PL_STENCIL_OP_KEEP, PL_STENCIL_OP_REPLACE,
														   PL_STENCIL_OP_KEEP, PL_COMPARE_OP_ALWAYS, 0xFF, 0xFF, 1);

		PlPipelineCreateInfo outlinePipelineInfo = pl::pipelineCreateInfo(
			"Outline", PL_RENDER_PASS_INDIRECT_BUFFER_SPECIFIC_ENTITY,
			{pl::pipelineShaderStageCreateInfo(
				 PL_STAGE_VERTEX, FilesManager::sEngineFolder.string() + "/Shaders/Vulkan/outline/outline.vert",
				 "main"),
			 pl::pipelineShaderStageCreateInfo(
				 PL_STAGE_FRAGMENT, FilesManager::sEngineFolder.string() + "/Shaders/Vulkan/outline/outline.frag",
				 "main")},
			{}, {}, PL_TOPOLOGY_TRIANGLE_LIST, false,
			pl::pipelineRasterizationStateCreateInfo(false, false, PL_POLYGON_MODE_FILL, 1.0f, false, 0.0f, 0.0f, 0.0f,
													 PL_CULL_MODE_NONE, PL_FRONT_FACE_COUNTER_CLOCKWISE),
			pl::pipelineColorBlendStateCreateInfo({pl::pipelineColorBlendAttachmentState(true)}),
			pl::pipelineDepthStencilStateCreateInfo(true, true, PL_COMPARE_OP_ALWAYS, false, true, stencilOpState,
													stencilOpState),
			pl::pipelineViewportStateCreateInfo(1, 1), pl::pipelineMultisampleStateCreateInfo(PL_SAMPLE_COUNT_1_BIT, 0),
			{PL_DYNAMIC_STATE_VIEWPORT, PL_DYNAMIC_STATE_SCISSOR},
			{pl::pushConstantRange(PL_STAGE_VERTEX, 0, sizeof(OutlinePC))}, {});

		std::vector<PlVertexInputBindingDescription> bindingDescriptions{};
		bindingDescriptions.push_back(
			pl::vertexInputBindingDescription(0, sizeof(Vertex), PL_VERTEX_INPUT_RATE_VERTEX));
		bindingDescriptions.push_back(
			pl::vertexInputBindingDescription(1, sizeof(glm::vec4) * 4, PL_VERTEX_INPUT_RATE_INSTANCE));
		outlinePipelineInfo.vertexBindingDescriptions = bindingDescriptions;
		std::vector<PlVertexInputAttributeDescription> attributeDescriptions{};
		attributeDescriptions.push_back(
			pl::vertexInputAttributeDescription(0, 0, PL_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)));
		attributeDescriptions.push_back(pl::vertexInputAttributeDescription(1, 1, PL_FORMAT_R32G32B32A32_SFLOAT, 0));
		attributeDescriptions.push_back(
			pl::vertexInputAttributeDescription(2, 1, PL_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 4));
		attributeDescriptions.push_back(
			pl::vertexInputAttributeDescription(3, 1, PL_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 8));
		attributeDescriptions.push_back(
			pl::vertexInputAttributeDescription(4, 1, PL_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 12));
		outlinePipelineInfo.vertexAttributeDescriptions = attributeDescriptions;

		this->GetRenderPass("OutlineDrawPass")->AddPipeline(outlinePipelineInfo);

		outlinePipelineInfo.vertexAttributeDescriptions = {};
		outlinePipelineInfo.vertexBindingDescriptions = {};
		outlinePipelineInfo.pushConstants = {};
		outlinePipelineInfo.pipelineName = "OutlineBlurPass";
		outlinePipelineInfo.renderMethod = PL_RENDER_PASS_FULL_SCREEN_QUAD;
		outlinePipelineInfo.shaderStages = {
			pl::pipelineShaderStageCreateInfo(
				PL_STAGE_VERTEX, FilesManager::sEngineFolder.string() + "/Shaders/Vulkan/fullScreenQuad.vert", "main"),
			pl::pipelineShaderStageCreateInfo(
				PL_STAGE_FRAGMENT, FilesManager::sEngineFolder.string() + "/Shaders/Vulkan/outline/outlineBlur.frag",
				"main")};

		this->AddRenderPass("OutlineBlurPass", PL_STAGE_VERTEX | PL_STAGE_FRAGMENT, PL_RENDER_PASS_FULL_SCREEN_QUAD,
							gPassSize, false)
			->AddInputTexture(1, 0, 0, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT,
							  PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("OutlineTexture"))
			->AddOutputTexture(1, 0, 0, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
							   0, 0, this->GetSharedTexture("OutlineBlurredTexture"));
		this->GetRenderPass("OutlineBlurPass")->AddPipeline(outlinePipelineInfo);

		outlinePipelineInfo.depthStencilState =
			pl::pipelineDepthStencilStateCreateInfo(true, false, PL_COMPARE_OP_LESS_OR_EQUAL, false, false);
		outlinePipelineInfo.shaderStages = {
			pl::pipelineShaderStageCreateInfo(
				PL_STAGE_VERTEX, FilesManager::sEngineFolder.string() + "/Shaders/Vulkan/fullScreenQuad.vert", "main"),
			pl::pipelineShaderStageCreateInfo(
				PL_STAGE_FRAGMENT,
				FilesManager::sEngineFolder.string() + "/Shaders/Vulkan/outline/outlineSceneMerge.frag", "main")};

		outlinePipelineInfo.pipelineName = "OutlineSceneMergePass";
		this->AddRenderPass("OutlineSceneMergePass", PL_STAGE_VERTEX | PL_STAGE_FRAGMENT,
							PL_RENDER_PASS_FULL_SCREEN_QUAD, gPassSize, false)
			->AddInputTexture(1, 0, 1, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT,
							  PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0,
							  this->GetSharedTexture("OutlineBlurredTexture"))
			->AddInputTexture(1, 0, 0, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
							  0, 0, this->GetSharedTexture("FinalTexture"), PL_ATTACHMENT_OP_LOAD, true)
			->AddInputTexture(1, 1, 2, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT,
							  PL_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 0, 0,
							  this->GetSharedTexture("OutlineStencil"), PL_ATTACHMENT_OP_LOAD, true);
		//->AddOutputTexture(1, 1, 0, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT,
		// PL_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 0, 0, this->GetSharedTexture("OutlineStencil"),
		// PL_ATTACHMENT_OP_LOAD, true);
		stencilOpState = PlStencilOpState(PL_STENCIL_OP_KEEP, PL_STENCIL_OP_KEEP, PL_STENCIL_OP_KEEP,
										  PL_COMPARE_OP_NOT_EQUAL, 0xFF, 0xFF, 0x01);
		outlinePipelineInfo.depthStencilState = pl::pipelineDepthStencilStateCreateInfo(
			false, false, PL_COMPARE_OP_ALWAYS, false, true, stencilOpState, stencilOpState);

		this->GetRenderPass("OutlineSceneMergePass")->AddPipeline(outlinePipelineInfo);

		// DebugRendererNodes(Application::Get()->activeCamera->GetViewport(), "FinalTexture");

		this->OrderPasses();
		this->UpdateUsedTexturesInfo();
	}

	void VulkanRenderGraph::DebugRendererNodes(const PlViewport& viewport, const std::string& textureToDraw) {
		this->AddRenderPass("DebugRendererPass", PL_STAGE_VERTEX | PL_STAGE_FRAGMENT, PL_RENDER_PASS_FULL_SCREEN_QUAD,
							glm::vec2(viewport.x, viewport.y), false)
			->AddInputTexture(1, 0, 0, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
							  0, 0, this->GetSharedTexture(textureToDraw), PL_ATTACHMENT_OP_LOAD, true);

		struct DebugPC {
			glm::mat4 viewMatrix;
		};

		this->GetRenderPass("DebugRendererPass")
			->AddPipeline(pl::pipelineCreateInfo(
				"Skybox", PL_RENDER_PASS_INDIRECT_BUFFER,
				{pl::pipelineShaderStageCreateInfo(
					 PL_STAGE_VERTEX,
					 FilesManager::sEngineFolder.string() + "/Shaders/Vulkan/debug/debugTriangles.vert", "main"),
				 pl::pipelineShaderStageCreateInfo(
					 PL_STAGE_FRAGMENT,
					 FilesManager::sEngineFolder.string() + "/Shaders/Vulkan/debug/debugTriangles.frag", "main")},
				VertexGetBindingDescription(), VertexGetAttributeDescriptions(), PL_TOPOLOGY_TRIANGLE_LIST, false,
				pl::pipelineRasterizationStateCreateInfo(false, false, PL_POLYGON_MODE_FILL, 1.0f, false, 0.0f, 0.0f,
														 0.0f, PL_CULL_MODE_NONE, PL_FRONT_FACE_CLOCKWISE),
				pl::pipelineColorBlendStateCreateInfo({pl::pipelineColorBlendAttachmentState(true),
													   pl::pipelineColorBlendAttachmentState(true),
													   pl::pipelineColorBlendAttachmentState(true)}),
				pl::pipelineDepthStencilStateCreateInfo(true, false, PL_COMPARE_OP_LESS_OR_EQUAL),
				pl::pipelineViewportStateCreateInfo(1, 1),
				pl::pipelineMultisampleStateCreateInfo(PL_SAMPLE_COUNT_1_BIT, 0),
				{PL_DYNAMIC_STATE_VIEWPORT, PL_DYNAMIC_STATE_SCISSOR},
				{pl::pushConstantRange(PL_STAGE_ALL, 0, sizeof(DebugPC))}));
	}

	VulkanRenderGraph* VulkanRenderGraph::BuildSkyboxRenderGraph() {
		VulkanRenderGraph* skyboxRenderGraph = new VulkanRenderGraph();

		const unsigned int faceSize = 512;
		const unsigned int brdfSize = 512;
		const unsigned int irradianceSize = 64;

		static EquirectangularToCubeMapPC pushConstants{};

		PlPipelineCreateInfo pipelineCreateInfo = pl::pipelineCreateInfo(
			"EquirectangularToCubeMapShaders", PL_RENDER_PASS_CUBE,
			{pl::pipelineShaderStageCreateInfo(
				 PL_STAGE_VERTEX,
				 FilesManager::sEngineFolder.string() + "/Shaders/Vulkan/skybox/equirectangularToCubemap.vert", "main"),
			 pl::pipelineShaderStageCreateInfo(PL_STAGE_FRAGMENT,
											   FilesManager::sEngineFolder.string() +
												   "/Shaders/Vulkan/skybox/equirectangularToCubemap.frag",
											   "main")},
			{}, {}, PL_TOPOLOGY_TRIANGLE_LIST, false,
			pl::pipelineRasterizationStateCreateInfo(false, false, PL_POLYGON_MODE_FILL, 1.0f, false, 0.0f, 0.0f, 0.0f,
													 PL_CULL_MODE_NONE, PL_FRONT_FACE_COUNTER_CLOCKWISE),
			pl::pipelineColorBlendStateCreateInfo({pl::pipelineColorBlendAttachmentState(true)}),
			pl::pipelineDepthStencilStateCreateInfo(false, false, PL_COMPARE_OP_ALWAYS),
			pl::pipelineViewportStateCreateInfo(1, 1), pl::pipelineMultisampleStateCreateInfo(PL_SAMPLE_COUNT_1_BIT, 0),
			{PL_DYNAMIC_STATE_VIEWPORT, PL_DYNAMIC_STATE_SCISSOR},
			{pl::pushConstantRange(PL_STAGE_ALL, 0, sizeof(EquirectangularToCubeMapPC))});

		skyboxRenderGraph
			->AddRenderPass("EquirectangularToCubeMapPass", PL_STAGE_VERTEX | PL_STAGE_FRAGMENT, PL_RENDER_PASS_CUBE,
							glm::vec2(faceSize, faceSize), true)
			->AddInputTexture(1, 0, 1, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT,
							  PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0,
							  this->GetSharedTexture("EquirectangularTexture"))
			->AddOutputTexture(1, 0, 0, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
							   0, 0, this->GetSharedTexture("CubeMapTexture"))
			->AddPipeline(pipelineCreateInfo);

		pipelineCreateInfo.pipelineName = "BrdfGeneratorShaders";
		pipelineCreateInfo.renderMethod = PL_RENDER_PASS_FULL_SCREEN_QUAD;
		pipelineCreateInfo.shaderStages = {
			pl::pipelineShaderStageCreateInfo(
				PL_STAGE_VERTEX, FilesManager::sEngineFolder.string() + "/Shaders/Vulkan/skybox/brdfGenerator.vert",
				"main"),
			pl::pipelineShaderStageCreateInfo(
				PL_STAGE_FRAGMENT, FilesManager::sEngineFolder.string() + "/Shaders/Vulkan/skybox/brdfGenerator.frag",
				"main")};
		pipelineCreateInfo.pushConstants = {};
		skyboxRenderGraph
			->AddRenderPass("BrdfGeneratorPass", PL_STAGE_VERTEX | PL_STAGE_FRAGMENT, PL_RENDER_PASS_FULL_SCREEN_QUAD,
							glm::vec2(brdfSize, brdfSize), true)
			->AddOutputTexture(1, 0, 0, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
							   0, 0, this->GetSharedTexture("SamplerBRDFLUT"))
			->AddPipeline(pipelineCreateInfo);

		pipelineCreateInfo.pipelineName = "IrradianceGeneratorShaders";
		pipelineCreateInfo.renderMethod = PL_RENDER_PASS_CUBE;
		pipelineCreateInfo.shaderStages = {
			pl::pipelineShaderStageCreateInfo(
				PL_STAGE_VERTEX,
				FilesManager::sEngineFolder.string() + "/Shaders/Vulkan/skybox/equirectangularToCubemap.vert", "main"),
			pl::pipelineShaderStageCreateInfo(
				PL_STAGE_FRAGMENT,
				FilesManager::sEngineFolder.string() + "/Shaders/Vulkan/skybox/irradianceGenerator.frag", "main")};
		pipelineCreateInfo.pushConstants = {pl::pushConstantRange(PL_STAGE_ALL, 0, sizeof(EquirectangularToCubeMapPC))};
		skyboxRenderGraph
			->AddRenderPass("IrradianceGeneratorPass", PL_STAGE_VERTEX | PL_STAGE_FRAGMENT, PL_RENDER_PASS_CUBE,
							glm::vec2(irradianceSize, irradianceSize), false)
			->AddInputTexture(1, 0, 2, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT,
							  PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("CubeMapTexture"))
			->AddOutputTexture(1, 0, 0, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
							   0, 0, this->GetSharedTexture("IrradianceMap"))
			->AddPipeline(pipelineCreateInfo);

		pipelineCreateInfo.pipelineName = "PreFilteredGeneratorShaders";
		pipelineCreateInfo.renderMethod = PL_RENDER_PASS_CUBE;
		pipelineCreateInfo.shaderStages = {
			pl::pipelineShaderStageCreateInfo(
				PL_STAGE_VERTEX,
				FilesManager::sEngineFolder.string() + "/Shaders/Vulkan/skybox/equirectangularToCubemap.vert", "main"),
			pl::pipelineShaderStageCreateInfo(
				PL_STAGE_FRAGMENT,
				FilesManager::sEngineFolder.string() + "/Shaders/Vulkan/skybox/prefilterEnvGenerator.frag", "main")};
		pipelineCreateInfo.pushConstants = {pl::pushConstantRange(PL_STAGE_ALL, 0, sizeof(EquirectangularToCubeMapPC))};
		skyboxRenderGraph
			->AddRenderPass("PreFilteredGeneratorPass", PL_STAGE_VERTEX | PL_STAGE_FRAGMENT, PL_RENDER_PASS_CUBE,
							glm::vec2(faceSize, faceSize), false)
			->AddInputTexture(1, 0, 0, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT,
							  PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("CubeMapTexture"))
			->AddOutputTexture(1, 0, 0, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
							   0, 0, this->GetSharedTexture("PreFilterMap"))
			->AddPipeline(pipelineCreateInfo);

		return skyboxRenderGraph;
	}

	void VulkanRenderGraph::RunSkyboxRenderGraph(VulkanRenderGraph* renderGraph) {
		PL_CORE_INFO("Run SkyboxRenderGraph");
		renderGraph->mCompiledBindings = mCompiledBindings;
		renderGraph->Compile();

		const unsigned int faceSize = 512;
		const unsigned int irradianceSize = 64;
		const unsigned int numMips = this->GetSharedTexture("PreFilterMap")->mMipCount;
		PlTextureFormat skyboxFormat = PL_FORMAT_R32G32B32A32_SFLOAT;

		const std::vector<glm::mat4> matrices = {
			glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
						glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
						glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
		};

		const std::vector<glm::mat4> equirectangularToCubeMapMatrices = {
			glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
						glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
						glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
		};

		EquirectangularToCubeMapPC converterPushConstants{};
		converterPushConstants.first = true;

		VkCommandBuffer commandBuffer = VulkanRenderer::GetRenderer()->BeginSingleTimeCommands();

		renderGraph->GetRenderPass("EquirectangularToCubeMapPass")->mPipelines[0]->mPushConstants[0].mData =
			new EquirectangularToCubeMapPC();
		;
		for (uint32_t i = 0; i < 6; i++) {
			VkImageViewCreateInfo layerImageViewCreateInfo = {};
			layerImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			layerImageViewCreateInfo.image =
				this->GetTexture<VulkanTexture>("CubeMapTexture")->mImage; // mSkyboxTexture->mImage;
			layerImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			layerImageViewCreateInfo.format = PlImageFormatToVkFormat(skyboxFormat);
			layerImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			layerImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
			layerImageViewCreateInfo.subresourceRange.levelCount = 1;
			layerImageViewCreateInfo.subresourceRange.baseArrayLayer = i;
			layerImageViewCreateInfo.subresourceRange.layerCount = 1;

			VkImageView layerImageView;
			vkCreateImageView(VulkanRenderer::GetRenderer()->mDevice, &layerImageViewCreateInfo, nullptr,
							  &layerImageView);

			std::vector<VkImageView> frameBufferAttachments{layerImageView};

			VkFramebuffer framebuffer = VulkanRenderer::GetRenderer()->CreateFramebuffer(
				renderGraph->GetRenderPass("EquirectangularToCubeMapPass")->mRenderPass, glm::vec2(faceSize, faceSize),
				frameBufferAttachments.data(), frameBufferAttachments.size(), 1);

			converterPushConstants.mvp =
				glm::perspective((float)(glm::pi<double>() / 2.0), 1.0f, 0.1f, static_cast<float>(faceSize)) *
				equirectangularToCubeMapMatrices[i];
			renderGraph->GetRenderPass("EquirectangularToCubeMapPass")->mFrameBuffer = framebuffer;
			renderGraph->GetRenderPass("EquirectangularToCubeMapPass")->UpdateCommandBuffer(commandBuffer);
			renderGraph->GetRenderPass("EquirectangularToCubeMapPass")->mPipelines[0]->mPushConstants[0].mData =
				&converterPushConstants;
			memcpy(renderGraph->GetRenderPass("EquirectangularToCubeMapPass")->mPipelines[0]->mPushConstants[0].mData,
				   &converterPushConstants, sizeof(EquirectangularToCubeMapPC));
			renderGraph->GetRenderPass("EquirectangularToCubeMapPass")->Execute(nullptr, renderGraph);
		}

		VulkanRenderer::GetRenderer()->EndSingleTimeCommands(commandBuffer);

		commandBuffer = VulkanRenderer::GetRenderer()->BeginSingleTimeCommands();
		renderGraph->GetRenderPass("BrdfGeneratorPass")->UpdateCommandBuffer(commandBuffer);
		renderGraph->GetRenderPass("BrdfGeneratorPass")->Execute(nullptr, renderGraph);
		VulkanRenderer::GetRenderer()->EndSingleTimeCommands(commandBuffer);

		commandBuffer = VulkanRenderer::GetRenderer()->BeginSingleTimeCommands();
		renderGraph->GetRenderPass("IrradianceGeneratorPass")->mPipelines[0]->mPushConstants[0].mData =
			new EquirectangularToCubeMapPC();
		converterPushConstants.first = false;
		converterPushConstants.deltaPhi = (2.0f * float(3.14159265358979323846)) / 180.0f;
		converterPushConstants.deltaTheta = (0.5f * float(3.14159265358979323846)) / 64.0f;
		for (uint32_t i = 0; i < 6; i++) {
			VkImageViewCreateInfo layerImageViewCreateInfo = {};
			layerImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			layerImageViewCreateInfo.image =
				this->GetTexture<VulkanTexture>("IrradianceMap")->mImage; // mSkyboxTexture->mImage;
			layerImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			layerImageViewCreateInfo.format = PlImageFormatToVkFormat(skyboxFormat);
			layerImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			layerImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
			layerImageViewCreateInfo.subresourceRange.levelCount = 1;
			layerImageViewCreateInfo.subresourceRange.baseArrayLayer = i;
			layerImageViewCreateInfo.subresourceRange.layerCount = 1;

			VkImageView layerImageView;
			vkCreateImageView(VulkanRenderer::GetRenderer()->mDevice, &layerImageViewCreateInfo, nullptr,
							  &layerImageView);

			std::vector<VkImageView> frameBufferAttachments{layerImageView};

			VkFramebuffer framebuffer = VulkanRenderer::GetRenderer()->CreateFramebuffer(
				renderGraph->GetRenderPass("IrradianceGeneratorPass")->mRenderPass,
				glm::vec2(irradianceSize, irradianceSize), frameBufferAttachments.data(), frameBufferAttachments.size(),
				1);

			converterPushConstants.mvp =
				glm::perspective((float)(glm::pi<double>() / 2.0), 1.0f, 0.1f, static_cast<float>(faceSize)) *
				matrices[i];
			renderGraph->GetRenderPass("IrradianceGeneratorPass")->mFrameBuffer = framebuffer;
			renderGraph->GetRenderPass("IrradianceGeneratorPass")->UpdateCommandBuffer(commandBuffer);
			renderGraph->GetRenderPass("IrradianceGeneratorPass")->mPipelines[0]->mPushConstants[0].mData =
				&converterPushConstants;
			memcpy(renderGraph->GetRenderPass("IrradianceGeneratorPass")->mPipelines[0]->mPushConstants[0].mData,
				   &converterPushConstants, sizeof(EquirectangularToCubeMapPC));
			renderGraph->GetRenderPass("IrradianceGeneratorPass")->Execute(nullptr, renderGraph);
		}
		VulkanRenderer::GetRenderer()->EndSingleTimeCommands(commandBuffer);

		commandBuffer = VulkanRenderer::GetRenderer()->BeginSingleTimeCommands();
		renderGraph->GetRenderPass("PreFilteredGeneratorPass")->mPipelines[0]->mPushConstants[0].mData =
			new EquirectangularToCubeMapPC();
		converterPushConstants.first = false;
		converterPushConstants.deltaPhi = (2.0f * float(3.14159265358979323846)) / 180.0f;
		converterPushConstants.deltaTheta = (0.5f * float(3.14159265358979323846)) / 64.0f;
		for (uint32_t m = 0; m < numMips; m++) {
			converterPushConstants.roughness = (float)m / (float)(numMips - 1);
			for (uint32_t i = 0; i < 6; i++) {
				glm::vec2 currentSize = glm::vec2(faceSize * std::pow(0.5f, m), faceSize * std::pow(0.5f, m));
				VkImageViewCreateInfo layerImageViewCreateInfo = {};
				layerImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				layerImageViewCreateInfo.image =
					this->GetTexture<VulkanTexture>("PreFilterMap")->mImage; // mSkyboxTexture->mImage;
				layerImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				layerImageViewCreateInfo.format = PlImageFormatToVkFormat(skyboxFormat);
				layerImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				layerImageViewCreateInfo.subresourceRange.baseMipLevel = m;
				layerImageViewCreateInfo.subresourceRange.levelCount = 1;
				layerImageViewCreateInfo.subresourceRange.baseArrayLayer = i;
				layerImageViewCreateInfo.subresourceRange.layerCount = 1;

				VkImageView layerImageView;
				vkCreateImageView(VulkanRenderer::GetRenderer()->mDevice, &layerImageViewCreateInfo, nullptr,
								  &layerImageView);

				std::vector<VkImageView> frameBufferAttachments{layerImageView};

				VkFramebuffer framebuffer = VulkanRenderer::GetRenderer()->CreateFramebuffer(
					renderGraph->GetRenderPass("PreFilteredGeneratorPass")->mRenderPass, currentSize,
					frameBufferAttachments.data(), frameBufferAttachments.size(), 1);

				converterPushConstants.mvp =
					glm::perspective((float)(glm::pi<double>() / 2.0), 1.0f, 0.1f, static_cast<float>(faceSize)) *
					matrices[i];
				renderGraph->GetRenderPass("PreFilteredGeneratorPass")->mFrameBuffer = framebuffer;
				renderGraph->GetRenderPass("PreFilteredGeneratorPass")->UpdateCommandBuffer(commandBuffer);
				renderGraph->GetRenderPass("PreFilteredGeneratorPass")->mPipelines[0]->mPushConstants[0].mData =
					&converterPushConstants;
				memcpy(renderGraph->GetRenderPass("PreFilteredGeneratorPass")->mPipelines[0]->mPushConstants[0].mData,
					   &converterPushConstants, sizeof(EquirectangularToCubeMapPC));
				renderGraph->GetRenderPass("PreFilteredGeneratorPass")->mRenderSize = currentSize;
				renderGraph->GetRenderPass("PreFilteredGeneratorPass")->Execute(nullptr, renderGraph);
			}
		}
		VulkanRenderer::GetRenderer()->EndSingleTimeCommands(commandBuffer);
	}

#pragma region Shadows frustum
	void GetChildrendUuid(std::vector<uint64_t>& vector, Entity* entity, Scene* scene) {
		if (scene->GetComponent<MeshRenderer>(entity->uuid))
			vector.push_back(entity->uuid);
		for (uint64_t child : entity->childrenUuid) {
			if (vector.size() < 8192)
				GetChildrendUuid(vector, scene->GetEntity(child), scene);
		}
	}

	// Function to calculate the split distances based on the provided formula
	std::vector<float> calculateSplitDistances(float nearClip, float farClip, int numSplits, float lambda = 0.5f) {
		std::vector<float> splitDistances(numSplits + 1);
		splitDistances[0] = nearClip;
		// splitDistances[numSplits] = farClip;

		for (int i = 1; i < numSplits; ++i) {
			float i_normalized = static_cast<float>(i) / static_cast<float>(numSplits);
			float log_term = nearClip * std::pow(farClip / nearClip, i_normalized);
			float linear_term = nearClip + (farClip - nearClip) * i_normalized;
			// splitDistances[i] = lambda * log_term + (1.0f - lambda) * linear_term;
		}

		float mult = 1.0f;
		splitDistances[0] = Application::Get()->activeCamera->farPlane / (9000.0f * mult);
		splitDistances[1] = Application::Get()->activeCamera->farPlane / (3000.0f * mult);
		splitDistances[2] = Application::Get()->activeCamera->farPlane / (1000.0f * mult);
		splitDistances[3] = Application::Get()->activeCamera->farPlane / (500.0f * mult);
		splitDistances[4] = Application::Get()->activeCamera->farPlane / (100.0f * mult);
		splitDistances[5] = Application::Get()->activeCamera->farPlane / (35.0f * mult);
		splitDistances[6] = Application::Get()->activeCamera->farPlane / (10.0f * mult);
		splitDistances[7] = Application::Get()->activeCamera->farPlane / (2.0f * mult);
		splitDistances[8] = Application::Get()->activeCamera->farPlane / (1.0f * mult);

		return splitDistances;
	}

	glm::mat4 getShadowMapMatrix(const glm::vec3& lightDirection, float shadowMapResolution, float nearPlane,
								 float farPlane, const glm::mat4& viewMatrix, float ratio) {
		const auto proj =
			glm::perspective(glm::radians(Application::Get()->activeCamera->Zoom), ratio, nearPlane, farPlane);
		const auto corners = Application::Get()->activeCamera->getFrustumCornersWorldSpace(proj, viewMatrix);

		glm::vec3 center = glm::vec3(0, 0, 0);
		for (const auto& v : corners) {
			center += glm::vec3(v);
		}
		center /= corners.size();

		const float LARGE_CONSTANT = std::abs(std::numeric_limits<float>::min());
		auto lightView = glm::lookAt(center + lightDirection, center, glm::vec3(0.0f, 1.0f, 0.0f));
		float minX = std::numeric_limits<float>::max();
		float maxX = std::numeric_limits<float>::lowest();
		float minY = std::numeric_limits<float>::max();
		float maxY = std::numeric_limits<float>::lowest();
		float minZ = std::numeric_limits<float>::max();
		float maxZ = std::numeric_limits<float>::lowest();
		for (const auto& v : corners) {
			const auto trf = lightView * v;
			minX = std::min(minX, trf.x);
			maxX = std::max(maxX, trf.x);
			minY = std::min(minY, trf.y);
			maxY = std::max(maxY, trf.y);
			minZ = std::min(minZ, trf.z);
			maxZ = std::max(maxZ, trf.z);
		}

		// Tune this parameter according to the scene
		// lightView = glm::lookAt(center - lightDir * (minZ), center, glm::vec3(0.0f, 1.0f, 0.0f));

		constexpr float zMult = 22.0f;
		if (minZ < 0) {
			minZ *= zMult;
		}
		else {
			minZ /= zMult;
		}
		if (maxZ < 0) {
			maxZ /= zMult;
		}
		else {
			maxZ *= zMult;
		}

		// const glm::mat4 lightProjection = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.01f,
		// maxExtents.z - minExtents.z);
		const glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
		return lightProjection * lightView;
	}

	std::vector<glm::mat4> GetShadowMatrices(RendererSettings::LightingSettings& settings, const glm::mat4& camProj,
											 const glm::mat4& camView) {
		int numSplits = settings.mCascadeCount;
		glm::vec3 lightDir = (settings.mLightDirection);

		float nearClip = Application::Get()->activeCamera->nearPlane;
		float farClip = Application::Get()->activeCamera->farPlane;

		float fovY = glm::degrees(2.0f * atan(1.0f / camProj[1][1]));
		fovY = Application::Get()->activeCamera->Zoom;
		float aspect = camProj[1][1] / camProj[0][0];
		aspect = Application::Get()->appSizes->sceneSize.x / Application::Get()->appSizes->sceneSize.y;

		std::vector<float> splitDistances = calculateSplitDistances(nearClip, farClip, numSplits, settings.mLambda);
		for (int i = 0; i < splitDistances.size(); ++i)
			settings.shadowCascadeLevels[i] = splitDistances[i];

		std::vector<glm::mat4> shadowMatrices;
		for (int i = 0; i < numSplits + 1; ++i) {
			float nearSplit = 0.0f;
			if (i == 0)
				nearSplit = Application::Get()->activeCamera->nearPlane - 1.0f;
			else
				nearSplit = splitDistances[i - 1] - 1.0f;
			float farSplit = splitDistances[i];
			const auto proj = glm::perspective((Application::Get()->activeCamera->Zoom), aspect, nearSplit, farSplit);
			auto corners = Application::Get()->activeCamera->getFrustumCornersWorldSpace(
				proj, camView); // getFrustumSliceCornersWorldSpace(nearSplit, farSplit, aspect, fovY, camView);
			shadowMatrices.push_back(getShadowMapMatrix(lightDir, 2048, nearSplit, farSplit,
														Application::Get()->activeCamera->GetViewMatrix(), aspect));
		}

		return shadowMatrices;
	}
#pragma endregion
} // namespace Plaza
