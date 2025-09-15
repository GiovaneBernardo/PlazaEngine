#include "Engine/Core/PreCompiledHeaders.h"
#include "RenderGraph.h"

#include "Renderer.h"
#include "stb_font_consolas_24_latin1.inl"
#include "Engine/Core/Scene.h"
#include "Engine/Core/Renderer/Mesh.h"

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
						 PL_FORMAT_R32G32B32A32_SFLOAT, glm::vec3(1, 1, 1), 1, 1, "TexturesBuffer");

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

		this->AddSampler("MaterialsSampler");

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
		this->AddBuffer(PL_BUFFER_UNIFORM_BUFFER, 1, sizeof(ShadowPassUBO), bufferCount, PL_BUFFER_USAGE_UNIFORM_BUFFER,
						PL_MEMORY_USAGE_CPU_TO_GPU, "ShadowPassUBO");
		this->AddBuffer(PL_BUFFER_UNIFORM_BUFFER, 1, sizeof(UniformBufferObject), bufferCount,
						PL_BUFFER_USAGE_UNIFORM_BUFFER, PL_MEMORY_USAGE_CPU_TO_GPU, "GPassUBO");
		this->AddBuffer(PL_BUFFER_UNIFORM_BUFFER, 1, sizeof(DeferredLightingPassUbo), bufferCount,
						PL_BUFFER_USAGE_UNIFORM_BUFFER, PL_MEMORY_USAGE_CPU_TO_GPU, "LightingPassUBO");
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

		PlPipelineCreateInfo shadowPassPipelineCreateInfo = pl::pipelineCreateInfo(
			"ShadowMapping", PL_RENDER_PASS_INDIRECT_BUFFER_SHADOW_MAP,
			{pl::pipelineShaderStageCreateInfo(
				 PL_STAGE_VERTEX,
				 FilesManager::sEngineFolder.string() + "/Shaders/shadows/cascadedShadowDepthShaders.vert", "mainVS"),
			 pl::pipelineShaderStageCreateInfo(
				 PL_STAGE_FRAGMENT,
				 FilesManager::sEngineFolder.string() + "/Shaders/shadows/cascadedShadowDepthShaders.frag", "mainPS")},
			{pl::vertexInputBindingDescription(0, sizeof(Vertex), PL_VERTEX_INPUT_RATE_VERTEX),
			 pl::vertexInputBindingDescription(1, sizeof(glm::vec4) * 4, PL_VERTEX_INPUT_RATE_INSTANCE)},
			{pl::vertexInputAttributeDescription(0, 0, PL_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)),
			 pl::vertexInputAttributeDescription(1, 1, PL_FORMAT_R32G32B32A32_SFLOAT, 0),
			 pl::vertexInputAttributeDescription(2, 1, PL_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 4),
			 pl::vertexInputAttributeDescription(3, 1, PL_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 8),
			 pl::vertexInputAttributeDescription(4, 1, PL_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 12)},
			PL_TOPOLOGY_TRIANGLE_LIST, false,
			pl::pipelineRasterizationStateCreateInfo(false, false, PL_POLYGON_MODE_FILL, 1.0f, false, 0.0f, 0.0f, 0.0f,
													 PL_CULL_MODE_BACK, PL_FRONT_FACE_COUNTER_CLOCKWISE),
			pl::pipelineColorBlendStateCreateInfo({pl::pipelineColorBlendAttachmentState(true)}),
			pl::pipelineDepthStencilStateCreateInfo(true, true, PL_COMPARE_OP_GREATER),
			pl::pipelineViewportStateCreateInfo(1, 1), pl::pipelineMultisampleStateCreateInfo(PL_SAMPLE_COUNT_1_BIT, 0),
			{PL_DYNAMIC_STATE_VIEWPORT, PL_DYNAMIC_STATE_SCISSOR}, {});

		this->AddRenderPass("ShadowPass", PL_STAGE_VERTEX | PL_STAGE_FRAGMENT,
							PL_RENDER_PASS_INDIRECT_BUFFER_SHADOW_MAP,
							glm::vec2(shadowMapResolution, shadowMapResolution), true)
			->SetBuffer("ShadowsUBO", this->GetSharedBuffer("ShadowPassUBO"))
			->SetBuffer("BoneMatrices", this->GetSharedBuffer("BoneMatricesBuffer"))
			->AddRenderTarget(this->GetSharedTexture("ShadowsDepthMap"))
			->SetShader("Shadows.hlsl")
			->AddPipeline(shadowPassPipelineCreateInfo);

		static ShadowPassUBO shadowPassUbo{};
		this->AddRenderPassCallback(
			"ShadowPass", [&](PlazaRenderGraph* plazaRenderGraph, PlazaRenderPass* plazaRenderPass, Scene* scene) {
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

		// Geometry
		PlPipelineCreateInfo geometryPassPipelineCreateInfo = pl::pipelineCreateInfo(
			"GeometryPass", PL_RENDER_PASS_INDIRECT_BUFFER, {},
			{pl::vertexInputBindingDescription(0, sizeof(Vertex), PL_VERTEX_INPUT_RATE_VERTEX),
			 pl::vertexInputBindingDescription(1, sizeof(glm::vec4) * 4, PL_VERTEX_INPUT_RATE_INSTANCE)},
			{pl::vertexInputAttributeDescription(0, 0, PL_FORMAT_R32G32B32_SFLOAT,
												 offsetof(Vertex, position)), // inPosition
			 pl::vertexInputAttributeDescription(1, 0, PL_FORMAT_R32G32B32_SFLOAT,
												 offsetof(Vertex, normal)), // inNormal
			 pl::vertexInputAttributeDescription(2, 0, PL_FORMAT_R32G32_SFLOAT,
												 offsetof(Vertex, texCoords)), // inTexCoord
			 pl::vertexInputAttributeDescription(3, 0, PL_FORMAT_R32G32B32_SFLOAT,
												 offsetof(Vertex, tangent)),			  // inTangent
			 pl::vertexInputAttributeDescription(4, 1, PL_FORMAT_R32G32B32A32_SFLOAT, 0), // instanceMatrix0
			 pl::vertexInputAttributeDescription(5, 1, PL_FORMAT_R32G32B32A32_SFLOAT,
												 sizeof(float) * 4), // instanceMatrix1
			 pl::vertexInputAttributeDescription(6, 1, PL_FORMAT_R32G32B32A32_SFLOAT,
												 sizeof(float) * 8), // instanceMatrix2
			 pl::vertexInputAttributeDescription(7, 1, PL_FORMAT_R32G32B32A32_SFLOAT,
												 sizeof(float) * 12), // instanceMatrix3
			 pl::vertexInputAttributeDescription(8, 1, PL_FORMAT_R32_UINT, sizeof(unsigned int))},

			PL_TOPOLOGY_TRIANGLE_LIST, false,
			pl::pipelineRasterizationStateCreateInfo(false, false, PL_POLYGON_MODE_FILL, 1.0f, false, 0.0f, 0.0f, 0.0f,
													 PL_CULL_MODE_BACK, PL_FRONT_FACE_COUNTER_CLOCKWISE),
			pl::pipelineColorBlendStateCreateInfo({3, pl::pipelineColorBlendAttachmentState(true)}),
			pl::pipelineDepthStencilStateCreateInfo(true, true, PL_COMPARE_OP_LESS_OR_EQUAL),
			pl::pipelineViewportStateCreateInfo(1, 1), pl::pipelineMultisampleStateCreateInfo(PL_SAMPLE_COUNT_1_BIT, 0),
			{PL_DYNAMIC_STATE_VIEWPORT, PL_DYNAMIC_STATE_SCISSOR}, {});

		this->AddRenderPass("DeferredGeometryPass", PL_STAGE_VERTEX | PL_STAGE_FRAGMENT, PL_RENDER_PASS_INDIRECT_BUFFER,
							screenSize, true)
			->SetBuffer("UBO", this->GetSharedBuffer("GPassUBO"))
			->SetBuffer("MaterialsSSBO", this->GetSharedBuffer("MaterialsBuffer"))
			->SetBuffer("RenderGroupMaterialsOffsets", this->GetSharedBuffer("RenderGroupMaterialsOffsetsBuffer"))
			->SetBuffer("RenderGroupOffsets", this->GetSharedBuffer("RenderGroupOffsetsBuffer"))
			->SetTexture("textures", this->GetSharedTexture("TexturesBuffer"))
			->SetSampler("texSampler", this->GetSharedTextureSampler("MaterialsSampler"))
			->AddRenderTarget(this->GetSharedTexture("GNormal"))
			->AddRenderTarget(this->GetSharedTexture("GDiffuse"))
			->AddRenderTarget(this->GetSharedTexture("GOthers"))
			->AddRenderTarget(this->GetSharedTexture("SceneDepth"))
			->SetShader("DeferredGeometry.hlsl")
			->AddPipeline(geometryPassPipelineCreateInfo);

		static UniformBufferObject geometryUbo{};
		this->AddRenderPassCallback("DeferredGeometryPass", [&](PlazaRenderGraph* plazaRenderGraph,
																PlazaRenderPass* plazaRenderPass, Scene* scene) {
			// TODO: MOVE THE THREAD THAT UPDATES THE TERRAIN GENERATED BY THE TERRAIN TOOL TO SOMEWHERE ELSE
			Application::Get()->mThreadsManager->mFrameRendererAfterGeometry->Update();

			geometryUbo.projection = Application::Get()->activeCamera->GetProjectionMatrix();
			geometryUbo.view = Application::Get()->activeCamera->GetViewMatrix();
			geometryUbo.model = glm::mat4(1.0f);

			geometryUbo.cascadeCount = 9;
			geometryUbo.farPlane = 15000.0f;
			geometryUbo.nearPlane = 0.01f;

			glm::vec3 lightDir = mRenderer->mRendererSettings.mLightingSettings.mLightDirection;
			glm::vec3 lightDistance = glm::vec3(100.0f, 400.0f, 0.0f);
			glm::vec3 lightPos;

			geometryUbo.lightDirection = glm::vec4(lightDir, 1.0f);
			geometryUbo.viewPos = glm::vec4(Application::Get()->activeCamera->Position, 1.0f);

			geometryUbo.directionalLightColor =
				glm::vec4(mRenderer->mRendererSettings.mLightingSettings.directionalLightColor *
						  mRenderer->mRendererSettings.mLightingSettings.directionalLightIntensity);
			geometryUbo.directionalLightColor.w =
				mRenderer->mRendererSettings.mLightingSettings.directionalLightIntensity;
			geometryUbo.ambientLightColor =
				glm::vec4(mRenderer->mRendererSettings.mLightingSettings.ambientLightColor *
						  mRenderer->mRendererSettings.mLightingSettings.ambientLightIntensity);
			geometryUbo.gamma = mRenderer->gamma;

			for (int i = 0; i < mRenderer->mRendererSettings.mLightingSettings.mCascadeCount; ++i) {
				geometryUbo.lightSpaceMatrices[i] = shadowPassUbo.lightSpaceMatrices[i];
				geometryUbo.cascadePlaneDistances[i] =
					glm::vec4(mRenderer->mRendererSettings.mLightingSettings.shadowCascadeLevels[i], 1.0f, 1.0f, 1.0f);
			}

			geometryUbo.showCascadeLevels = Application::Get()->showCascadeLevels;

			plazaRenderGraph->GetSharedBuffer("GPassUBO")
				->UpdateData<UniformBufferObject>(Application::Get()->mRenderer->mCurrentFrame, geometryUbo);
		});

		this->AddRenderPass("DeferredLightingPass", PL_STAGE_VERTEX | PL_STAGE_FRAGMENT,
							PL_RENDER_PASS_FULL_SCREEN_QUAD, screenSize, true)
			->SetBuffer("UBO", this->GetSharedBuffer("LightingPassUBO"))
			->SetBuffer("LightsSSBO", this->GetSharedBuffer("LightsBuffer"))
			->SetBuffer("ClustersSSBO", this->GetSharedBuffer("ClustersBuffer"))
			->SetTexture("samplerBRDFLUT", this->GetSharedTexture("SamplerBRDFLUT"))
			->SetTexture("prefilterMap", this->GetSharedTexture("PreFilterMap"))
			->SetTexture("irradianceMap", this->GetSharedTexture("IrradianceMap"))
			->SetTexture("shadowsDepthMap", this->GetSharedTexture("ShadowsDepthMap"))
			->SetTexture("equirectangularMap", this->GetSharedTexture("EquirectangularTexture"))
			->SetTexture("gNormal", this->GetSharedTexture("GNormal"))
			->SetTexture("gDiffuse", this->GetSharedTexture("GDiffuse"))
			->SetTexture("gOthers", this->GetSharedTexture("GOthers"))
			->SetTexture("gSceneDepth", this->GetSharedTexture("SceneDepth"))
			->SetSampler("linearSampler", this->GetSharedTextureSampler("MaterialsSampler"))
			->AddRenderTarget(this->GetSharedTexture("SceneTexture"))
			->SetShader("DeferredLighting.hlsl");
			//->AddPipeline(geometryPassPipelineCreateInfo);'

		/*
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


		 */

		this->GetRenderPass("DeferredLightingPass")
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

		this->AddRenderPassCallback("DeferredLightingPass", [&](PlazaRenderGraph* plazaRenderGraph,
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

			ubo.screenSize = Application::Get()->appSizes->sceneSize;
			ubo.clusterSize = mRenderer->mRendererSettings.mLightingSettings.clusterSize;
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
			plazaRenderGraph->GetSharedBuffer("LightingPassUBO")
				->UpdateData<DeferredLightingPassUbo>(Application::Get()->mRenderer->mCurrentFrame, ubo);
		});

		this->OrderPasses();
		this->UpdateUsedTexturesInfo();
	}

	void VulkanRenderGraph::DebugRendererNodes(const PlViewport& viewport, const std::string& textureToDraw) {
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
