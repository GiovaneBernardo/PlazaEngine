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

namespace Plaza {
	void GetChilrendUuid(std::vector<uint64_t>& vector, Entity* entity, Scene* scene) {
		if (scene->GetComponent<MeshRenderer>(entity->uuid))
			vector.push_back(entity->uuid);
		for (uint64_t child : entity->childrenUuid) {
			GetChilrendUuid(vector, scene->GetEntity(child), scene);
		}
	}

	void VulkanRenderGraph::BuildDefaultRenderGraph() {
		PlImageUsage inImageUsageFlags = static_cast<PlImageUsage>(PL_IMAGE_USAGE_COLOR_ATTACHMENT | PL_IMAGE_USAGE_SAMPLED | PL_IMAGE_USAGE_TRANSFER_DST | PL_IMAGE_USAGE_TRANSFER_SRC);
		PlImageUsage outImageUsageFlags = static_cast<PlImageUsage>(PL_IMAGE_USAGE_TRANSFER_DST | PL_IMAGE_USAGE_TRANSFER_SRC | PL_IMAGE_USAGE_SAMPLED | PL_IMAGE_USAGE_COLOR_ATTACHMENT);
		PlImageUsage depthTextureFlags = static_cast<PlImageUsage>(PL_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT | PL_IMAGE_USAGE_SAMPLED | PL_IMAGE_USAGE_TRANSFER_DST | PL_IMAGE_USAGE_TRANSFER_SRC);
		PlImageUsage equirectangularImageUsage = static_cast<PlImageUsage>(PL_IMAGE_USAGE_COLOR_ATTACHMENT | PL_IMAGE_USAGE_SAMPLED | PL_IMAGE_USAGE_TRANSFER_DST | PL_IMAGE_USAGE_TRANSFER_SRC);
		const unsigned int bufferCount = Application::Get()->mRenderer->mMaxFramesInFlight;
		const unsigned int shadowMapResolution = 2048;
		const unsigned int skyboxResolution = 512;
		const unsigned int irradianceSize = 64;
		const unsigned int brdfSize = 512;
		const glm::vec2 screenSize = Application::Get()->appSizes->sceneSize;
		const glm::vec2 deferredTileSize = glm::vec2(32, 32);
		const uint32_t clusterCount = glm::ceil(screenSize.x / deferredTileSize.x + 1) * glm::ceil(screenSize.y / deferredTileSize.y + 1);

		VulkanRenderGraph* graph = this;

		this->AddTexture(make_shared<VulkanTexture>(VulkanRenderer::GetRenderer()->mMaxBindlessTextures, inImageUsageFlags, PL_TYPE_2D, PL_VIEW_TYPE_2D, PL_FORMAT_R32G32B32A32_SFLOAT, glm::vec3(1, 1, 1), 1, 0, "TexturesBuffer"));

		this->AddTexture(make_shared<VulkanTexture>(1, equirectangularImageUsage, PL_TYPE_2D, PL_VIEW_TYPE_2D, PL_FORMAT_R8_UNORM, glm::vec3(STB_FONT_consolas_24_latin1_BITMAP_WIDTH, STB_FONT_consolas_24_latin1_BITMAP_HEIGHT, 1), 1, 1, "FontTexture"));
		this->AddTexture(make_shared<VulkanTexture>(1, equirectangularImageUsage, PL_TYPE_2D, PL_VIEW_TYPE_2D, PL_FORMAT_R32G32B32A32_SFLOAT, glm::vec3(skyboxResolution, skyboxResolution, 1), 1, 1, "EquirectangularTexture"));
		this->AddTexture(make_shared<VulkanTexture>(1, equirectangularImageUsage, PL_TYPE_2D, PL_VIEW_TYPE_CUBE, PL_FORMAT_R32G32B32A32_SFLOAT, glm::vec3(skyboxResolution, skyboxResolution, 1), 1, 6, "CubeMapTexture"));
		this->AddTexture(make_shared<VulkanTexture>(1, inImageUsageFlags, PL_TYPE_2D, PL_VIEW_TYPE_2D, PL_FORMAT_R16G16_SFLOAT, glm::vec3(brdfSize, brdfSize, 1), 1, 1, "SamplerBRDFLUT"));
		this->AddTexture(make_shared<VulkanTexture>(1, inImageUsageFlags, PL_TYPE_2D, PL_VIEW_TYPE_CUBE, PL_FORMAT_R32G32B32A32_SFLOAT, glm::vec3(skyboxResolution, skyboxResolution, 1), 0, 6, "PreFilterMap"));
		this->AddTexture(make_shared<VulkanTexture>(1, inImageUsageFlags, PL_TYPE_2D, PL_VIEW_TYPE_CUBE, PL_FORMAT_R32G32B32A32_SFLOAT, glm::vec3(irradianceSize, irradianceSize, 1), 1, 6, "IrradianceMap"));

		this->AddTexture(make_shared<VulkanTexture>(1, depthTextureFlags, PL_TYPE_2D, PL_VIEW_TYPE_2D_ARRAY, PL_FORMAT_D32_SFLOAT_S8_UINT, glm::vec3(shadowMapResolution, shadowMapResolution, 1), 1, VulkanRenderer::GetRenderer()->mShadows->mCascadeCount, "ShadowsDepthMap"));
		//this->AddTexture(make_shared<VulkanTexture>(1, outImageUsageFlags, PL_TYPE_2D, PL_VIEW_TYPE_2D, PL_FORMAT_R32G32B32A32_SFLOAT, glm::vec3(Application::Get()->appSizes->sceneSize, 1), 1, 1, "GPosition"));
		this->AddTexture(make_shared<VulkanTexture>(1, outImageUsageFlags, PL_TYPE_2D, PL_VIEW_TYPE_2D, PL_FORMAT_R32G32B32A32_SFLOAT, glm::vec3(Application::Get()->appSizes->sceneSize, 1), 1, 1, "GDiffuse"));
		this->AddTexture(make_shared<VulkanTexture>(1, outImageUsageFlags, PL_TYPE_2D, PL_VIEW_TYPE_2D, PL_FORMAT_R32G32B32A32_SFLOAT, glm::vec3(Application::Get()->appSizes->sceneSize, 1), 1, 1, "GNormal"));
		this->AddTexture(make_shared<VulkanTexture>(1, outImageUsageFlags, PL_TYPE_2D, PL_VIEW_TYPE_2D, PL_FORMAT_R32G32B32A32_SFLOAT, glm::vec3(Application::Get()->appSizes->sceneSize, 1), 1, 1, "GOthers"));
		this->AddTexture(make_shared<VulkanTexture>(1, depthTextureFlags, PL_TYPE_2D, PL_VIEW_TYPE_2D, PL_FORMAT_D32_SFLOAT_S8_UINT, glm::vec3(Application::Get()->appSizes->sceneSize, 1), 1, 1, "SceneDepth"));
		this->AddTexture(make_shared<VulkanTexture>(1, depthTextureFlags, PL_TYPE_2D, PL_VIEW_TYPE_2D, PL_FORMAT_D32_SFLOAT_S8_UINT, glm::vec3(Application::Get()->appSizes->sceneSize, 1), 1, 1, "OutlineStencil"));

		TextureInfo info{};
		info.mPath = "deferred";
		this->GetTexture<VulkanTexture>("GDiffuse")->CreateTextureInfo(info);
		this->GetTexture<VulkanTexture>("GNormal")->CreateTextureInfo(info);
		this->GetTexture<VulkanTexture>("GOthers")->CreateTextureInfo(info);
		this->GetTexture<VulkanTexture>("SceneDepth")->CreateTextureInfo(info);

		this->AddTexture(make_shared<VulkanTexture>(1, static_cast<PlImageUsage>(outImageUsageFlags | PL_IMAGE_USAGE_STORAGE), PL_TYPE_2D, PL_VIEW_TYPE_2D, PL_FORMAT_R32G32B32A32_SFLOAT, glm::vec3(Application::Get()->appSizes->sceneSize, 1), 1, 1, "SceneTexture"));
		this->AddTexture(make_shared<VulkanTexture>(1, outImageUsageFlags, PL_TYPE_2D, PL_VIEW_TYPE_2D, PL_FORMAT_R32G32B32A32_SFLOAT, glm::vec3(Application::Get()->appSizes->sceneSize, 1), 1, 1, "OutFinalPostProcessTexture"));
		this->AddTexture(make_shared<VulkanTexture>(1, outImageUsageFlags, PL_TYPE_2D, PL_VIEW_TYPE_2D, PL_FORMAT_R8G8B8A8_UNORM, glm::vec3(Application::Get()->appSizes->sceneSize, 1), 1, 1, "FinalTexture"));
		this->AddTexture(make_shared<VulkanTexture>(1, outImageUsageFlags, PL_TYPE_2D, PL_VIEW_TYPE_2D, PL_FORMAT_R8G8B8A8_UNORM, glm::vec3(Application::Get()->appSizes->sceneSize, 1), 1, 1, "PostProcessedTexture"));
		this->AddTexture(make_shared<VulkanTexture>(1, outImageUsageFlags, PL_TYPE_2D, PL_VIEW_TYPE_2D, PL_FORMAT_R8G8B8A8_UNORM, glm::vec3(Application::Get()->appSizes->sceneSize, 1), 1, 1, "OutlineTexture"));
		this->AddTexture(make_shared<VulkanTexture>(1, outImageUsageFlags, PL_TYPE_2D, PL_VIEW_TYPE_2D, PL_FORMAT_R8G8B8A8_UNORM, glm::vec3(Application::Get()->appSizes->sceneSize, 1), 1, 1, "OutlineBlurredTexture"));

		this->AddTexture(make_shared<VulkanTexture>(1, PlImageUsage(outImageUsageFlags | PL_IMAGE_USAGE_STORAGE), PL_TYPE_2D, PL_VIEW_TYPE_2D, PL_FORMAT_R32G32B32A32_SFLOAT, glm::vec3(Application::Get()->appSizes->sceneSize, 1), 0, 1, "BloomTexture"));
		TextureInfo bloomInfo = this->GetTexture<VulkanTexture>("BloomTexture")->GetTextureInfo();
		bloomInfo.mInitialLayout = PL_IMAGE_LAYOUT_GENERAL;
		bloomInfo.mSamplerAddressMode = PL_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		this->GetTexture<VulkanTexture>("BloomTexture")->SetTextureInfo(bloomInfo);

		this->AddTexture(make_shared<VulkanTexture>(1, outImageUsageFlags, PL_TYPE_2D, PL_VIEW_TYPE_2D, PL_FORMAT_R32G32B32A32_SFLOAT, glm::vec3(Application::Get()->appSizes->sceneSize, 1), 1, 1, "SSRTexture"));
		// this->AddTexture(make_shared<VulkanTexture>(1, PlImageUsage(outImageUsageFlags | PL_IMAGE_USAGE_STORAGE), PL_TYPE_2D, PL_VIEW_TYPE_2D, PL_FORMAT_R32G32B32A32_SFLOAT, glm::vec3(Application::Get()->appSizes->sceneSize, 1), 1, 1, "BloomFinalTexture"));

		std::string skyboxPath;
#ifdef EDITOR_MODE
		skyboxPath = Application::Get()->enginePath + "\\Editor\\DefaultAssets\\Skybox\\";
#else
		skyboxPath = Application::Get()->exeDirectory + "\\";
#endif

		TextureInfo equirectangularInfo = this->GetTexture<VulkanTexture>("EquirectangularTexture")->GetTextureInfo();
		equirectangularInfo.mPath = skyboxPath + "autumn_field_puresky_4k.hdr";
		equirectangularInfo.mIsHdr = true;
		this->GetTexture<Texture>("EquirectangularTexture")->SetTextureInfo(equirectangularInfo);

		struct ShadowPassUBO {
			glm::mat4 lightSpaceMatrices[32];
		};
		struct DeferredLightingPassUbo {
			glm::mat4 projection;                     // 64 bytes
			glm::mat4 view;                           // 64 bytes
			alignas(4) uint32_t showCascadeLevels;    // 4 bytes, must align to 4 bytes
			float farPlane;                           // 4 bytes
			float nearPlane;                          // 4 bytes
			float gamma;                              // 4 bytes
			float exposure;                              // 4 bytes
			int cascadeCount;                         // 4 bytes
			int lightCount;                           // 4 bytes
			alignas(16) glm::vec4 viewPos;            // 16 bytes
			glm::vec4 lightDirection;                 // 16 bytes
			glm::vec4 ambientLightColor;              // 16 bytes
			glm::vec4 directionalLightColor;          // 16 bytes
			glm::mat4 lightSpaceMatrices[16];         // 16 * 64 bytes = 1024 bytes
			glm::vec4 cascadePlaneDistances[16];      // 16 * 16 bytes = 256 bytes
		};

		this->AddBuffer(std::make_shared<PlVkBuffer>(PL_BUFFER_UNIFORM_BUFFER, 1, sizeof(ShadowPassUBO), bufferCount, PL_BUFFER_USAGE_UNIFORM_BUFFER, PL_MEMORY_USAGE_CPU_TO_GPU, "ShadowPassUBO"));
		this->AddBuffer(std::make_shared<PlVkBuffer>(PL_BUFFER_UNIFORM_BUFFER, 1, sizeof(VulkanRenderer::UniformBufferObject), bufferCount, PL_BUFFER_USAGE_UNIFORM_BUFFER, PL_MEMORY_USAGE_CPU_TO_GPU, "GPassUBO"));
		this->AddBuffer(std::make_shared<PlVkBuffer>(PL_BUFFER_UNIFORM_BUFFER, 1, sizeof(DeferredLightingPassUbo), bufferCount, PL_BUFFER_USAGE_UNIFORM_BUFFER, PL_MEMORY_USAGE_CPU_TO_GPU, "DeferredPassUBO"));
		this->AddBuffer(std::make_shared<PlVkBuffer>(PL_BUFFER_STORAGE_BUFFER, 1024 * 16, sizeof(glm::mat4), bufferCount, PL_BUFFER_USAGE_STORAGE_BUFFER, PL_MEMORY_USAGE_CPU_TO_GPU, "BoneMatricesBuffer"));
		this->AddBuffer(std::make_shared<PlVkBuffer>(PL_BUFFER_STORAGE_BUFFER, 1024 * 16, sizeof(VulkanRenderer::MaterialData), bufferCount, PL_BUFFER_USAGE_STORAGE_BUFFER, PL_MEMORY_USAGE_CPU_TO_GPU, "MaterialsBuffer"));
		this->AddBuffer(std::make_shared<PlVkBuffer>(PL_BUFFER_STORAGE_BUFFER, 1024 * 256 * 64, sizeof(unsigned int), bufferCount, PL_BUFFER_USAGE_STORAGE_BUFFER, PL_MEMORY_USAGE_CPU_TO_GPU, "RenderGroupOffsetsBuffer"));
		this->AddBuffer(std::make_shared<PlVkBuffer>(PL_BUFFER_STORAGE_BUFFER, 1024 * 256 * 64, sizeof(unsigned int), bufferCount, PL_BUFFER_USAGE_STORAGE_BUFFER, PL_MEMORY_USAGE_CPU_TO_GPU, "RenderGroupMaterialsOffsetsBuffer"));
		this->AddBuffer(std::make_shared<PlVkBuffer>(PL_BUFFER_STORAGE_BUFFER, 1024 * 32, sizeof(Lighting::LightStruct), bufferCount, static_cast<PlBufferUsage>(PL_BUFFER_USAGE_STORAGE_BUFFER | PL_BUFFER_USAGE_TRANSFER_DST), PL_MEMORY_USAGE_CPU_TO_GPU, "LightsBuffer"));
		this->AddBuffer(std::make_shared<PlVkBuffer>(PL_BUFFER_STORAGE_BUFFER, clusterCount, sizeof(Lighting::Tile), bufferCount, static_cast<PlBufferUsage>(PL_BUFFER_USAGE_STORAGE_BUFFER | PL_BUFFER_USAGE_TRANSFER_DST), PL_MEMORY_USAGE_CPU_TO_GPU, "ClustersBuffer"));
		this->AddBuffer(std::make_shared<PlVkBuffer>(PL_BUFFER_STORAGE_BUFFER, clusterCount, sizeof(glm::vec2), bufferCount, static_cast<PlBufferUsage>(PL_BUFFER_USAGE_STORAGE_BUFFER | PL_BUFFER_USAGE_TRANSFER_DST), PL_MEMORY_USAGE_CPU_TO_GPU, "TilesDepthBuffer"));
		this->AddBuffer(std::make_shared<PlVkBuffer>(PL_BUFFER_STORAGE_BUFFER, 1024, sizeof(glm::mat4), bufferCount, static_cast<PlBufferUsage>(PL_BUFFER_USAGE_VERTEX_BUFFER), PL_MEMORY_USAGE_CPU_TO_GPU, "RectanglesTransformBuffer"));
		this->AddBuffer(std::make_shared<PlVkBuffer>(PL_BUFFER_STORAGE_BUFFER, 1024 * 2, sizeof(glm::vec4), bufferCount, static_cast<PlBufferUsage>(PL_BUFFER_USAGE_VERTEX_BUFFER), PL_MEMORY_USAGE_CPU_TO_GPU, "GuiTextVerticesBuffer"));

		uint64_t a = alignof(Lighting::LightStruct);
		uint64_t b = sizeof(Lighting::LightStruct);

		this->AddRenderPass(std::make_shared<VulkanRenderPass>("Shadow Pass", PL_STAGE_VERTEX | PL_STAGE_FRAGMENT, PL_RENDER_PASS_INDIRECT_BUFFER_SHADOW_MAP, glm::vec2(shadowMapResolution, shadowMapResolution), true))
			->AddInputResource(std::make_shared<VulkanBufferBinding>(1, 0, PlBufferType::PL_BUFFER_UNIFORM_BUFFER, PL_STAGE_VERTEX, this->GetSharedBuffer("ShadowPassUBO")))
			->AddInputResource(std::make_shared<VulkanBufferBinding>(1, 1, PL_BUFFER_STORAGE_BUFFER, PL_STAGE_VERTEX, this->GetSharedBuffer("BoneMatricesBuffer")))
			->AddOutputResource(std::make_shared<VulkanTextureBinding>(1, 0, 0, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 0, 0, this->GetSharedTexture("ShadowsDepthMap")));

		this->GetRenderPass("Shadow Pass")->mMultiViewCount = VulkanRenderer::GetRenderer()->mShadows->mCascadeCount;
		this->GetRenderPass("Shadow Pass")->AddPipeline(pl::pipelineCreateInfo(
			"ShadowMapping",
			PL_RENDER_PASS_INDIRECT_BUFFER_SHADOW_MAP,
			{ pl::pipelineShaderStageCreateInfo(PL_STAGE_VERTEX, Application::Get()->enginePath + "\\Shaders\\shadows\\cascadedShadowDepthShaders.vert", "main"),
				pl::pipelineShaderStageCreateInfo(PL_STAGE_FRAGMENT, Application::Get()->enginePath + "\\Shaders\\shadows\\cascadedShadowDepthShaders.frag", "main") },
			{ pl::vertexInputBindingDescription(0, sizeof(Vertex), PL_VERTEX_INPUT_RATE_VERTEX), pl::vertexInputBindingDescription(1, sizeof(glm::vec4) * 4, PL_VERTEX_INPUT_RATE_INSTANCE) },
			{ pl::vertexInputAttributeDescription(0, 0, PL_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)),
			pl::vertexInputAttributeDescription(5, 1, PL_FORMAT_R32G32B32A32_SFLOAT, 0),
			pl::vertexInputAttributeDescription(6, 1, PL_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 4),
			pl::vertexInputAttributeDescription(7, 1, PL_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 8),
			pl::vertexInputAttributeDescription(8, 1, PL_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 12) },
			PL_TOPOLOGY_TRIANGLE_LIST,
			false,
			pl::pipelineRasterizationStateCreateInfo(false, false, PL_POLYGON_MODE_FILL, 1.0f, false, 0.0f, 0.0f, 0.0f, PL_CULL_MODE_BACK, PL_FRONT_FACE_COUNTER_CLOCKWISE),
			pl::pipelineColorBlendStateCreateInfo({ pl::pipelineColorBlendAttachmentState(true) }),
			pl::pipelineDepthStencilStateCreateInfo(true, true, PL_COMPARE_OP_LESS),
			pl::pipelineViewportStateCreateInfo(1, 1),
			pl::pipelineMultisampleStateCreateInfo(PL_SAMPLE_COUNT_1_BIT, 0),
			{ PL_DYNAMIC_STATE_VIEWPORT, PL_DYNAMIC_STATE_SCISSOR },
			{}
		));

		static ShadowPassUBO shadowPassUbo{};
		this->AddRenderPassCallback("Shadow Pass", [&](PlazaRenderGraph* plazaRenderGraph, PlazaRenderPass* plazaRenderPass) {
			std::vector<glm::mat4> mats = VulkanShadows::GetLightSpaceMatrices(VulkanRenderer::GetRenderer()->mShadows->shadowCascadeLevels, VulkanRenderer::GetRenderer()->mShadows->mLightDirection);
			for (int i = 0; i < VulkanRenderer::GetRenderer()->mShadows->mCascadeCount; ++i) {
				shadowPassUbo.lightSpaceMatrices[i] = mats[i];
			}
			for (unsigned int i = VulkanRenderer::GetRenderer()->mShadows->mCascadeCount; i < 32; ++i) {
				shadowPassUbo.lightSpaceMatrices[i] = glm::mat4(1.0f);
			}
			plazaRenderGraph->GetSharedBuffer("ShadowPassUBO")->UpdateData<ShadowPassUBO>(Application::Get()->mRenderer->mCurrentFrame, shadowPassUbo);
			});


		glm::vec2 gPassSize = Application::Get()->appSizes->sceneSize;
		this->AddRenderPass(std::make_shared<VulkanRenderPass>("Deferred Geometry Pass", PL_STAGE_VERTEX | PL_STAGE_FRAGMENT, PL_RENDER_PASS_INDIRECT_BUFFER, glm::vec2(gPassSize.x, gPassSize.y), true))
			->AddInputResource(std::make_shared<VulkanBufferBinding>(1, 0, PlBufferType::PL_BUFFER_UNIFORM_BUFFER, PL_STAGE_ALL, this->GetSharedBuffer("GPassUBO")))
			->AddInputResource(std::make_shared<VulkanTextureBinding>(VulkanRenderer::GetRenderer()->mMaxBindlessTextures, 0, 20, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("TexturesBuffer")))
			->AddInputResource(std::make_shared<VulkanBufferBinding>(1, 19, PL_BUFFER_STORAGE_BUFFER, PL_STAGE_VERTEX, this->GetSharedBuffer("MaterialsBuffer")))
			->AddInputResource(std::make_shared<VulkanBufferBinding>(1, 1, PL_BUFFER_STORAGE_BUFFER, PL_STAGE_VERTEX, this->GetSharedBuffer("BoneMatricesBuffer")))
			->AddInputResource(std::make_shared<VulkanBufferBinding>(1, 2, PL_BUFFER_STORAGE_BUFFER, PL_STAGE_VERTEX, this->GetSharedBuffer("RenderGroupOffsetsBuffer")))
			->AddInputResource(std::make_shared<VulkanBufferBinding>(1, 3, PL_BUFFER_STORAGE_BUFFER, PL_STAGE_VERTEX, this->GetSharedBuffer("RenderGroupMaterialsOffsetsBuffer")))
			->AddInputResource(std::make_shared<VulkanTextureBinding>(1, 0, 7, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("PreFilterMap")))
			->AddInputResource(std::make_shared<VulkanTextureBinding>(1, 0, 8, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("EquirectangularTexture")))
			//->AddOutputResource(std::make_shared<VulkanTextureBinding>(1, 0, 0, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0, 0, this->GetSharedTexture("GPosition")))
			->AddOutputResource(std::make_shared<VulkanTextureBinding>(1, 0, 0, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0, 0, this->GetSharedTexture("GNormal")))
			->AddOutputResource(std::make_shared<VulkanTextureBinding>(1, 1, 0, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0, 0, this->GetSharedTexture("GDiffuse")))
			->AddOutputResource(std::make_shared<VulkanTextureBinding>(1, 2, 0, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0, 0, this->GetSharedTexture("GOthers")))
			->AddOutputResource(std::make_shared<VulkanTextureBinding>(1, 3, 0, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 0, 0, this->GetSharedTexture("SceneDepth")));

		this->GetRenderPass("Deferred Geometry Pass")->AddPipeline(pl::pipelineCreateInfo(
			"MainShaders",
			PL_RENDER_PASS_INDIRECT_BUFFER,
			{ pl::pipelineShaderStageCreateInfo(PL_STAGE_VERTEX, Application::Get()->enginePath + "\\Shaders\\Vulkan\\deferred\\geometryPass.vert", "main"),
				pl::pipelineShaderStageCreateInfo(PL_STAGE_FRAGMENT, Application::Get()->enginePath + "\\Shaders\\Vulkan\\deferred\\geometryPass.frag", "main") },
			VertexGetBindingDescription(),
			VertexGetAttributeDescriptions(),
			PL_TOPOLOGY_TRIANGLE_LIST,
			false,
			pl::pipelineRasterizationStateCreateInfo(false, false, PL_POLYGON_MODE_FILL, 1.0f, false, 0.0f, 0.0f, 0.0f, PL_CULL_MODE_BACK, PL_FRONT_FACE_COUNTER_CLOCKWISE),
			pl::pipelineColorBlendStateCreateInfo({ pl::pipelineColorBlendAttachmentState(true) , pl::pipelineColorBlendAttachmentState(true)  ,pl::pipelineColorBlendAttachmentState(true) }),
			pl::pipelineDepthStencilStateCreateInfo(true, true, PL_COMPARE_OP_LESS_OR_EQUAL),
			pl::pipelineViewportStateCreateInfo(1, 1),
			pl::pipelineMultisampleStateCreateInfo(PL_SAMPLE_COUNT_1_BIT, 0),
			{ PL_DYNAMIC_STATE_VIEWPORT, PL_DYNAMIC_STATE_SCISSOR },
			{}
		));

		this->GetRenderPass("Deferred Geometry Pass")->AddPipeline(pl::pipelineCreateInfo(
			"Skinned",
			PL_RENDER_PASS_INDIRECT_BUFFER_SKINNED,
			{ pl::pipelineShaderStageCreateInfo(PL_STAGE_VERTEX, Application::Get()->enginePath + "\\Shaders\\Vulkan\\deferred\\geometrySkinnedPass.vert", "main"),
				pl::pipelineShaderStageCreateInfo(PL_STAGE_FRAGMENT, Application::Get()->enginePath + "\\Shaders\\Vulkan\\deferred\\geometryPass.frag", "main") },
			VertexGetBindingDescription(),
			SkinnedVertexGetAttributeDescriptions(),
			PL_TOPOLOGY_TRIANGLE_LIST,
			false,
			pl::pipelineRasterizationStateCreateInfo(false, false, PL_POLYGON_MODE_FILL, 1.0f, false, 0.0f, 0.0f, 0.0f, PL_CULL_MODE_BACK, PL_FRONT_FACE_COUNTER_CLOCKWISE),
			pl::pipelineColorBlendStateCreateInfo({ pl::pipelineColorBlendAttachmentState(true) , pl::pipelineColorBlendAttachmentState(true)  ,pl::pipelineColorBlendAttachmentState(true) }),
			pl::pipelineDepthStencilStateCreateInfo(true, true, PL_COMPARE_OP_LESS_OR_EQUAL),
			pl::pipelineViewportStateCreateInfo(1, 1),
			pl::pipelineMultisampleStateCreateInfo(PL_SAMPLE_COUNT_1_BIT, 0),
			{ PL_DYNAMIC_STATE_VIEWPORT, PL_DYNAMIC_STATE_SCISSOR },
			{}
		));

		struct DeferredGeometrySkyboxPC {
			glm::mat4 projection;
			glm::mat4 view;
			float skyboxIntensity;
			float gamma;
			float exposure;
			float useless;
		};

		uint64_t skyboxMeshUuid = 2;
		this->GetRenderPass("Deferred Geometry Pass")->AddPipeline(pl::pipelineCreateInfo(
			"Skybox",
			PL_RENDER_PASS_INDIRECT_BUFFER_SPECIFIC_MESH,
			{ pl::pipelineShaderStageCreateInfo(PL_STAGE_VERTEX, Application::Get()->enginePath + "\\Shaders\\Vulkan\\skybox\\skybox.vert", "main"),
				pl::pipelineShaderStageCreateInfo(PL_STAGE_FRAGMENT, Application::Get()->enginePath + "\\Shaders\\Vulkan\\skybox\\skybox.frag", "main") },
			VertexGetBindingDescription(),
			VertexGetAttributeDescriptions(),
			PL_TOPOLOGY_TRIANGLE_LIST,
			false,
			pl::pipelineRasterizationStateCreateInfo(false, false, PL_POLYGON_MODE_FILL, 1.0f, false, 0.0f, 0.0f, 0.0f, PL_CULL_MODE_NONE, PL_FRONT_FACE_CLOCKWISE),
			pl::pipelineColorBlendStateCreateInfo({ pl::pipelineColorBlendAttachmentState(true) , pl::pipelineColorBlendAttachmentState(true)  ,pl::pipelineColorBlendAttachmentState(true) }),
			pl::pipelineDepthStencilStateCreateInfo(true, false, PL_COMPARE_OP_LESS_OR_EQUAL),
			pl::pipelineViewportStateCreateInfo(1, 1),
			pl::pipelineMultisampleStateCreateInfo(PL_SAMPLE_COUNT_1_BIT, 0),
			{ PL_DYNAMIC_STATE_VIEWPORT, PL_DYNAMIC_STATE_SCISSOR },
			{ pl::pushConstantRange(PL_STAGE_ALL, 0, sizeof(DeferredGeometrySkyboxPC)) },
			{ skyboxMeshUuid }
		));

		this->AddRenderPassCallback("Deferred Geometry Pass", [&](PlazaRenderGraph* plazaRenderGraph, PlazaRenderPass* plazaRenderPass) {
			static VulkanRenderer::UniformBufferObject ubo{};
			ubo.projection = Application::Get()->activeCamera->GetProjectionMatrix();
			ubo.view = Application::Get()->activeCamera->GetViewMatrix();
			ubo.model = glm::mat4(1.0f);

			ubo.cascadeCount = 9;
			ubo.farPlane = 15000.0f;
			ubo.nearPlane = 0.01f;

			glm::vec3 lightDir = VulkanRenderer::GetRenderer()->mShadows->mLightDirection;
			glm::vec3 lightDistance = glm::vec3(100.0f, 400.0f, 0.0f);
			glm::vec3 lightPos;

			ubo.lightDirection = glm::vec4(lightDir, 1.0f);
			ubo.viewPos = glm::vec4(Application::Get()->activeCamera->Position, 1.0f);

			ubo.directionalLightColor = glm::vec4(VulkanRenderer::GetRenderer()->mLighting->directionalLightColor * VulkanRenderer::GetRenderer()->mLighting->directionalLightIntensity);
			ubo.directionalLightColor.a = 1.0f;
			ubo.ambientLightColor = glm::vec4(VulkanRenderer::GetRenderer()->mLighting->ambientLightColor * VulkanRenderer::GetRenderer()->mLighting->ambientLightIntensity);
			ubo.ambientLightColor.a = 1.0f;
			ubo.gamma = VulkanRenderer::GetRenderer()->gamma;

			for (int i = 0; i < VulkanRenderer::GetRenderer()->mShadows->mCascadeCount; ++i) {
				ubo.lightSpaceMatrices[i] = shadowPassUbo.lightSpaceMatrices[i];
				if (i <= 8)
					ubo.cascadePlaneDistances[i] = glm::vec4(VulkanRenderer::GetRenderer()->mShadows->shadowCascadeLevels[i], 1.0f, 1.0f, 1.0f);
				else
					ubo.cascadePlaneDistances[i] = glm::vec4(VulkanRenderer::GetRenderer()->mShadows->shadowCascadeLevels[8], 1.0f, 1.0f, 1.0f);
			}

			ubo.showCascadeLevels = Application::Get()->showCascadeLevels;
			plazaRenderGraph->GetSharedBuffer("GPassUBO")->UpdateData<VulkanRenderer::UniformBufferObject>(Application::Get()->mRenderer->mCurrentFrame, ubo);

			plazaRenderPass->mPipelines[2]->UpdatePushConstants<DeferredGeometrySkyboxPC>(0, DeferredGeometrySkyboxPC(Application::Get()->activeCamera->GetProjectionMatrix(), Application::Get()->activeCamera->GetViewMatrix(),
				VulkanRenderer::GetRenderer()->mSkyboxIntensity, VulkanRenderer::GetRenderer()->gamma, VulkanRenderer::GetRenderer()->exposure, 0.0f));
			});

		this->GetRenderPass("Deferred Geometry Pass")->GetInputResource<PlazaShadersBinding>("TexturesBuffer")->mMaxBindlessResources = VulkanRenderer::GetRenderer()->mMaxBindlessTextures;

		// Lights sorter
		struct LightSorterPC {
			glm::mat4 view;
			glm::mat4 projection;
			int lightCount;
			bool first;
			glm::vec2 screenSize;
			glm::vec2 clusterSize;
		};

		this->AddRenderPass(std::make_shared<VulkanRenderPass>("Light Sorter Pass", PL_STAGE_COMPUTE, PL_RENDER_PASS_COMPUTE, gPassSize, false))
			->AddInputResource(std::make_shared<VulkanTextureBinding>(1, 0, 3, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_COMPUTE, PL_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("SceneDepth")))
			->AddOutputResource(std::make_shared<VulkanBufferBinding>(1, 0, PL_BUFFER_STORAGE_BUFFER, PL_STAGE_COMPUTE, this->GetSharedBuffer("LightsBuffer")))
			->AddOutputResource(std::make_shared<VulkanBufferBinding>(1, 1, PL_BUFFER_STORAGE_BUFFER, PL_STAGE_COMPUTE, this->GetSharedBuffer("ClustersBuffer")))
			->AddOutputResource(std::make_shared<VulkanBufferBinding>(1, 8, PL_BUFFER_STORAGE_BUFFER, PL_STAGE_COMPUTE, this->GetSharedBuffer("TilesDepthBuffer")))
			->AddPipeline(pl::pipelineCreateInfo("LightSorter", PL_RENDER_PASS_COMPUTE,
				{ pl::pipelineShaderStageCreateInfo(PL_STAGE_COMPUTE, Application::Get()->enginePath + "\\Shaders\\Vulkan\\lighting\\lightSorter.comp", "main") },
				{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, { pl::pushConstantRange(PL_STAGE_COMPUTE, 0, sizeof(LightSorterPC)) }));

		this->AddRenderPassCallback("Light Sorter Pass", [&, gPassSize](PlazaRenderGraph* plazaRenderGraph, PlazaRenderPass* plazaRenderPass) {
			Application::Get()->mThreadsManager->mFrameRendererAfterGeometry->Update();
			glm::vec2 clusterSize = glm::vec2(32.0f);
			glm::vec2 clusterCount = glm::ceil(gPassSize / clusterSize);
			VulkanLighting* lig = VulkanRenderer::GetRenderer()->mLighting;
			plazaRenderPass->mPipelines[0]->UpdatePushConstants<LightSorterPC>(0, LightSorterPC(
				Application::Get()->activeCamera->GetViewMatrix(), Application::Get()->activeCamera->GetProjectionMatrix(), VulkanRenderer::GetRenderer()->mLighting->mLights.size(), true, gPassSize, clusterSize));
			plazaRenderPass->mDispatchSize = glm::vec3(clusterCount.x, clusterCount.y, 1);
			});

		// Deferred Lighting
		this->AddRenderPass(std::make_shared<VulkanRenderPass>("Deferred Lighting Pass", PL_STAGE_VERTEX | PL_STAGE_FRAGMENT, PL_RENDER_PASS_FULL_SCREEN_QUAD, gPassSize, false))
			->AddInputResource(std::make_shared<VulkanBufferBinding>(1, 15, PlBufferType::PL_BUFFER_UNIFORM_BUFFER, PL_STAGE_FRAGMENT, this->GetSharedBuffer("DeferredPassUBO")))
			->AddInputResource(std::make_shared<VulkanTextureBinding>(1, 0, 6, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("SamplerBRDFLUT")))
			->AddInputResource(std::make_shared<VulkanTextureBinding>(1, 0, 7, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("PreFilterMap")))
			->AddInputResource(std::make_shared<VulkanTextureBinding>(1, 0, 8, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("IrradianceMap")))
			->AddInputResource(std::make_shared<VulkanTextureBinding>(1, 0, 9, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("ShadowsDepthMap")))
			->AddInputResource(std::make_shared<VulkanTextureBinding>(1, 0, 10, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("EquirectangularTexture")))
			//->AddInputResource(std::make_shared<VulkanTextureBinding>(1, 0, 0, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("GPosition")))
			->AddInputResource(std::make_shared<VulkanTextureBinding>(1, 0, 0, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("GNormal")))
			->AddInputResource(std::make_shared<VulkanTextureBinding>(1, 0, 1, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("GDiffuse")))
			->AddInputResource(std::make_shared<VulkanTextureBinding>(1, 0, 2, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("GOthers")))
			->AddInputResource(std::make_shared<VulkanTextureBinding>(1, 0, 3, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("SceneDepth")))
			->AddInputResource(std::make_shared<VulkanBufferBinding>(1, 4, PL_BUFFER_STORAGE_BUFFER, PL_STAGE_FRAGMENT, this->GetSharedBuffer("LightsBuffer")))
			->AddInputResource(std::make_shared<VulkanBufferBinding>(1, 5, PL_BUFFER_STORAGE_BUFFER, PL_STAGE_FRAGMENT, this->GetSharedBuffer("ClustersBuffer")))
			->AddOutputResource(std::make_shared<VulkanTextureBinding>(1, 0, 0, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0, 0, this->GetSharedTexture("SceneTexture")));

		this->GetRenderPass("Deferred Lighting Pass")->AddPipeline(pl::pipelineCreateInfo(
			"LightingPassShaders",
			PL_RENDER_PASS_FULL_SCREEN_QUAD,
			{ pl::pipelineShaderStageCreateInfo(PL_STAGE_VERTEX, Application::Get()->enginePath + "\\Shaders\\Vulkan\\lighting\\deferredPass.vert", "main"),
				pl::pipelineShaderStageCreateInfo(PL_STAGE_FRAGMENT, Application::Get()->enginePath + "\\Shaders\\Vulkan\\lighting\\deferredPass.frag", "main") },
			VertexGetBindingDescription(),
			VertexGetAttributeDescriptions(),
			PL_TOPOLOGY_TRIANGLE_LIST,
			false,
			pl::pipelineRasterizationStateCreateInfo(false, false, PL_POLYGON_MODE_FILL, 1.0f, false, 0.0f, 0.0f, 0.0f, PL_CULL_MODE_NONE, PL_FRONT_FACE_COUNTER_CLOCKWISE),
			pl::pipelineColorBlendStateCreateInfo({ pl::pipelineColorBlendAttachmentState(true) }),
			pl::pipelineDepthStencilStateCreateInfo(false, false, PL_COMPARE_OP_ALWAYS),
			pl::pipelineViewportStateCreateInfo(1, 1),
			pl::pipelineMultisampleStateCreateInfo(PL_SAMPLE_COUNT_1_BIT, 0),
			{ PL_DYNAMIC_STATE_VIEWPORT, PL_DYNAMIC_STATE_SCISSOR },
			{  }
		));

		this->AddRenderPassCallback("Deferred Lighting Pass", [&](PlazaRenderGraph* plazaRenderGraph, PlazaRenderPass* plazaRenderPass) {
			static DeferredLightingPassUbo ubo{};
			ubo.projection = Application::Get()->activeCamera->GetProjectionMatrix();
			ubo.view = Application::Get()->activeCamera->GetViewMatrix();

			ubo.cascadeCount = 9;
			ubo.farPlane = 15000.0f;
			ubo.nearPlane = 0.01f;

			glm::vec3 lightDir = VulkanRenderer::GetRenderer()->mShadows->mLightDirection;
			glm::vec3 lightDistance = glm::vec3(100.0f, 400.0f, 0.0f);
			glm::vec3 lightPos;

			ubo.lightDirection = glm::vec4(lightDir, 1.0f);
			ubo.viewPos = glm::vec4(Application::Get()->activeCamera->Position, 1.0f);

			ubo.directionalLightColor = glm::vec4(VulkanRenderer::GetRenderer()->mLighting->directionalLightColor * VulkanRenderer::GetRenderer()->mLighting->directionalLightIntensity);
			ubo.directionalLightColor.a = 1.0f;
			ubo.ambientLightColor = glm::vec4(VulkanRenderer::GetRenderer()->mLighting->ambientLightColor * VulkanRenderer::GetRenderer()->mLighting->ambientLightIntensity);
			ubo.ambientLightColor.a = 1.0f;
			ubo.gamma = VulkanRenderer::GetRenderer()->gamma;
			ubo.exposure = VulkanRenderer::GetRenderer()->exposure;

			for (int i = 0; i < VulkanRenderer::GetRenderer()->mShadows->mCascadeCount; ++i) {
				ubo.lightSpaceMatrices[i] = shadowPassUbo.lightSpaceMatrices[i];
				if (i <= 8)
					ubo.cascadePlaneDistances[i] = glm::vec4(VulkanRenderer::GetRenderer()->mShadows->shadowCascadeLevels[i], 1.0f, 1.0f, 1.0f);
				else
					ubo.cascadePlaneDistances[i] = glm::vec4(VulkanRenderer::GetRenderer()->mShadows->shadowCascadeLevels[8], 1.0f, 1.0f, 1.0f);
			}

			ubo.showCascadeLevels = Application::Get()->showCascadeLevels;

			ubo.lightCount = VulkanRenderer::GetRenderer()->mLighting->mLights.size();
			//DeferredLightingPassConstants(Application::Get()->activeCamera->Position, 0.0f, Application::Get()->activeCamera->GetViewMatrix(),
			//Application::Get()->activeCamera->GetProjectionMatrix(), VulkanRenderer::GetRenderer()->mLighting->mLights.size(), VulkanRenderer::GetRenderer()->mLighting->ambientLightColor* VulkanRenderer::GetRenderer()->mLighting->ambientLightIntensity)
			//plazaRenderPass->mPipelines[0]->UpdatePushConstants<DeferredLightingPassConstants>(0, ubo);
			plazaRenderGraph->GetSharedBuffer("DeferredPassUBO")->UpdateData<DeferredLightingPassUbo>(Application::Get()->mRenderer->mCurrentFrame, ubo);
			});


		/* Bloom */
		struct BloomPassPC {
			glm::vec4 u_threshold;
			glm::vec2 u_texel_size;
			int u_mip_level;
			int u_use_threshold;
			float u_bloom_intensity;
		};
		this->AddRenderPass(std::make_shared<VulkanRenderPass>("Bloom Pass", PL_STAGE_COMPUTE, PL_RENDER_PASS_HOLDER, gPassSize, false))
			->AddInputResource(std::make_shared<VulkanTextureBinding>(1, 0, 0, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_COMPUTE, PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("BloomTexture")))
			->AddInputResource(std::make_shared<VulkanTextureBinding>(1, 0, 1, PL_BUFFER_STORAGE_IMAGE, PL_STAGE_COMPUTE, PL_IMAGE_LAYOUT_GENERAL, 0, 0, this->GetSharedTexture("BloomTexture")))
			->AddInputResource(std::make_shared<VulkanTextureBinding>(1, 0, 2, PL_BUFFER_STORAGE_IMAGE, PL_STAGE_COMPUTE, PL_IMAGE_LAYOUT_GENERAL, 0, 0, this->GetSharedTexture("SceneTexture")))
			->AddOutputResource(std::make_shared<VulkanTextureBinding>(1, 0, 0, PL_BUFFER_STORAGE_IMAGE, PL_STAGE_COMPUTE, PL_IMAGE_LAYOUT_GENERAL, 0, 0, this->GetSharedTexture("BloomTexture")))
			->SetRecordingCallback([&](PlazaRenderGraph* plazaRenderGraph, PlazaRenderPass* plazaRenderPass) {
			VulkanTexture* texture = this->GetTexture<VulkanTexture>("SceneTexture");

			VulkanRenderer::GetRenderer()->TransitionImageLayout(texture->mImage, PlImageFormatToVkFormat(texture->GetTextureInfo().mFormat), PlImageLayoutToVkImageLayout(PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
				PlImageLayoutToVkImageLayout(PL_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL), 1, 1, 1, false, *mCommandBuffer);

			VulkanRenderer::GetRenderer()->CopyTexture(this->GetTexture<VulkanTexture>("SceneTexture"), PlImageLayoutToVkImageLayout(PL_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL),
				this->GetTexture<VulkanTexture>("BloomTexture"), PlImageLayoutToVkImageLayout(PL_IMAGE_LAYOUT_GENERAL), *mCommandBuffer);

			VulkanRenderer::GetRenderer()->TransitionImageLayout(texture->mImage, PlImageFormatToVkFormat(texture->GetTextureInfo().mFormat), PlImageLayoutToVkImageLayout(PL_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL),
				PlImageLayoutToVkImageLayout(PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL), 1, 1, 1, false, *mCommandBuffer);

				});

		uint32_t downScaleLimit = 10;
		uint32_t width = gPassSize.x / 2;
		uint32_t height = gPassSize.y / 2;
		uint8_t  bloomMipCount = 1;
		for (uint8_t i = 0; i < 12; ++i)
		{
			width = width / 2;
			height = height / 2;
			if (width < downScaleLimit || height < downScaleLimit) break;
			++bloomMipCount;
		}
		bool pingPong = true;
		PlPipelineShaderStageCreateInfo downScaleShaders = pl::pipelineShaderStageCreateInfo(PL_STAGE_COMPUTE, VulkanShadersCompiler::Compile(Application::Get()->enginePath + "\\Shaders\\Vulkan\\bloom\\bloomDownScale.comp"), "main");
		PlPipelineShaderStageCreateInfo upScaleShaders = pl::pipelineShaderStageCreateInfo(PL_STAGE_COMPUTE, VulkanShadersCompiler::Compile(Application::Get()->enginePath + "\\Shaders\\Vulkan\\bloom\\bloomUpScale.comp"), "main");
		PlPipelineCreateInfo bloomPipelineCreateInfo = pl::pipelineCreateInfo("BloomShaders", PL_RENDER_PASS_COMPUTE, { downScaleShaders }, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, { pl::pushConstantRange(PL_STAGE_COMPUTE, 0, sizeof(BloomPassPC)) });
		std::shared_ptr<PlazaPipeline> bloomDownScalePipeline = this->GetRenderPass("Bloom Pass")->AddPipeline(bloomPipelineCreateInfo);

		glm::uvec2 mipSize = glm::uvec2(gPassSize.x / 2, gPassSize.y / 2);
		// DownScale
		for (unsigned int i = 0; i < bloomMipCount - 1; ++i) {
			this->GetRenderPass("Bloom Pass")->AddChildPass(std::make_shared<VulkanRenderPass>("Bloom Pass " + std::to_string(i) + " DownScale", PL_STAGE_COMPUTE, PL_RENDER_PASS_COMPUTE, gPassSize, false))
				->AddInputResource(std::make_shared<VulkanTextureBinding>(1, 0, 0, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_COMPUTE, PL_IMAGE_LAYOUT_GENERAL, i, 0, this->GetSharedTexture("BloomTexture")))
				->AddInputResource(std::make_shared<VulkanTextureBinding>(1, 0, 1, PL_BUFFER_STORAGE_IMAGE, PL_STAGE_COMPUTE, PL_IMAGE_LAYOUT_GENERAL, i + 1, 0, this->GetSharedTexture("BloomTexture")))
				->AddPipeline(bloomDownScalePipeline);
			this->GetRenderPass("Bloom Pass")->mChildPasses.back()->SetRecordingCallback([&, mipSize, i](PlazaRenderGraph* plazaRenderGraph, PlazaRenderPass* plazaRenderPass) {
				float threshold = VulkanRenderer::GetRenderer()->mBloom.mThreshold;
				float knee = VulkanRenderer::GetRenderer()->mBloom.mKnee;
				BloomPassPC constant{};
				constant.u_texel_size = 1.0f / glm::vec2(mipSize);
				constant.u_mip_level = i;
				constant.u_threshold = glm::vec4(threshold, threshold - knee, 2.0f * knee, 0.25f * knee);
				constant.u_use_threshold = i == 0 ? 1 : 0;
				constant.u_bloom_intensity = VulkanRenderer::GetRenderer()->mBloom.mBloomIntensity;
				plazaRenderPass->mDispatchSize = glm::vec3(glm::ceil(float(mipSize.x) / 8), glm::ceil(float(mipSize.y) / 8), 1);
				plazaRenderPass->mPipelines[0]->UpdatePushConstants<BloomPassPC>(0, BloomPassPC(constant.u_threshold, constant.u_texel_size, constant.u_mip_level, constant.u_use_threshold, constant.u_bloom_intensity));
				});

			mipSize = mipSize / 2u;
		}
		// UpScale
		bloomPipelineCreateInfo.shaderStages = { upScaleShaders };
		std::shared_ptr<PlazaPipeline> bloomUpScalePipeline = this->GetRenderPass("Bloom Pass")->AddPipeline(bloomPipelineCreateInfo);
		for (uint8_t i = bloomMipCount - 1; i >= 1; --i) {
			mipSize.x = glm::max(1.0, glm::floor(float(Application::Get()->appSizes->sceneSize.x) / glm::pow(2.0, i - 1)));
			mipSize.y = glm::max(1.0, glm::floor(float(Application::Get()->appSizes->sceneSize.y) / glm::pow(2.0, i - 1)));
			if (i == 0) {
				this->GetRenderPass("Bloom Pass")->AddChildPass(std::make_shared<VulkanRenderPass>("Bloom Pass " + std::to_string(i) + " UpScale", PL_STAGE_COMPUTE, PL_RENDER_PASS_COMPUTE, gPassSize, false))
					->AddInputResource(std::make_shared<VulkanTextureBinding>(1, 0, 0, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_COMPUTE, PL_IMAGE_LAYOUT_GENERAL, 1, 0, this->GetSharedTexture("BloomTexture")))
					->AddInputResource(std::make_shared<VulkanTextureBinding>(1, 0, 1, PL_BUFFER_STORAGE_IMAGE, PL_STAGE_COMPUTE, PL_IMAGE_LAYOUT_GENERAL, i, 0, this->GetSharedTexture("BloomTexture")))
					->AddPipeline(bloomUpScalePipeline);
			}
			else {
				this->GetRenderPass("Bloom Pass")->AddChildPass(std::make_shared<VulkanRenderPass>("Bloom Pass " + std::to_string(i) + " UpScale", PL_STAGE_COMPUTE, PL_RENDER_PASS_COMPUTE, gPassSize, false))
					->AddInputResource(std::make_shared<VulkanTextureBinding>(1, 0, 0, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_COMPUTE, PL_IMAGE_LAYOUT_GENERAL, i, 0, this->GetSharedTexture("BloomTexture")))
					->AddInputResource(std::make_shared<VulkanTextureBinding>(1, 0, 1, PL_BUFFER_STORAGE_IMAGE, PL_STAGE_COMPUTE, PL_IMAGE_LAYOUT_GENERAL, i - 1, 0, this->GetSharedTexture("BloomTexture")))
					->AddPipeline(bloomUpScalePipeline);
			}
			this->GetRenderPass("Bloom Pass")->mChildPasses.back()->SetRecordingCallback([&, mipSize, i](PlazaRenderGraph* plazaRenderGraph, PlazaRenderPass* plazaRenderPass) {
				float threshold = VulkanRenderer::GetRenderer()->mBloom.mThreshold;
				float knee = VulkanRenderer::GetRenderer()->mBloom.mKnee;
				BloomPassPC constant{};
				constant.u_texel_size = 1.0f / glm::vec2(mipSize);
				constant.u_mip_level = i;
				constant.u_threshold = glm::vec4(threshold, threshold - knee, 2.0f * knee, 0.25f * knee);
				constant.u_use_threshold = i == 0 ? 1 : 0;
				constant.u_bloom_intensity = VulkanRenderer::GetRenderer()->mBloom.mBloomIntensity;
				plazaRenderPass->mDispatchSize = glm::vec3(glm::ceil(float(mipSize.x) / 8), glm::ceil(float(mipSize.y) / 8), 1);
				plazaRenderPass->mPipelines[0]->UpdatePushConstants<BloomPassPC>(0, BloomPassPC(constant.u_threshold, constant.u_texel_size, constant.u_mip_level, constant.u_use_threshold, constant.u_bloom_intensity));
				});
		}

		// Screen Space reflections
		this->AddRenderPass(std::make_shared<VulkanRenderPass>("Screen Space Reflections Pass", PL_STAGE_VERTEX | PL_STAGE_FRAGMENT, PL_RENDER_PASS_FULL_SCREEN_QUAD, gPassSize, false))
			->AddInputResource(std::make_shared<VulkanTextureBinding>(1, 0, 0, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("GNormal")))
			->AddInputResource(std::make_shared<VulkanTextureBinding>(1, 0, 1, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_GENERAL, 0, 0, this->GetSharedTexture("BloomTexture")))
			->AddInputResource(std::make_shared<VulkanTextureBinding>(1, 0, 2, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("GOthers")))
			->AddInputResource(std::make_shared<VulkanTextureBinding>(1, 0, 3, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("SceneDepth")))
			->AddOutputResource(std::make_shared<VulkanTextureBinding>(1, 0, 0, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0, 0, this->GetSharedTexture("SSRTexture")));

		struct SSRPushConstants {
			glm::vec4 screenSize;
			glm::vec4 cameraPos;
			glm::mat4 projection;
			glm::mat4 view;
			glm::mat4 lensProjection;
		};
		this->GetRenderPass("Screen Space Reflections Pass")->AddPipeline(pl::pipelineCreateInfo(
			"ScreenSpaceReflectionsShaders",
			PL_RENDER_PASS_FULL_SCREEN_QUAD,
			{ pl::pipelineShaderStageCreateInfo(PL_STAGE_VERTEX, Application::Get()->enginePath + "\\Shaders\\Vulkan\\ssr\\ssr.vert", "main"),
				pl::pipelineShaderStageCreateInfo(PL_STAGE_FRAGMENT, Application::Get()->enginePath + "\\Shaders\\Vulkan\\ssr\\ssr.frag", "main") },
			VertexGetBindingDescription(),
			VertexGetAttributeDescriptions(),
			PL_TOPOLOGY_TRIANGLE_LIST,
			false,
			pl::pipelineRasterizationStateCreateInfo(false, false, PL_POLYGON_MODE_FILL, 1.0f, false, 0.0f, 0.0f, 0.0f, PL_CULL_MODE_NONE, PL_FRONT_FACE_COUNTER_CLOCKWISE),
			pl::pipelineColorBlendStateCreateInfo({ pl::pipelineColorBlendAttachmentState(true) }),
			pl::pipelineDepthStencilStateCreateInfo(false, false, PL_COMPARE_OP_ALWAYS),
			pl::pipelineViewportStateCreateInfo(1, 1),
			pl::pipelineMultisampleStateCreateInfo(PL_SAMPLE_COUNT_1_BIT, 0),
			{ PL_DYNAMIC_STATE_VIEWPORT, PL_DYNAMIC_STATE_SCISSOR },
			{ pl::pushConstantRange(PL_STAGE_FRAGMENT, 0, sizeof(SSRPushConstants)) },
			{  }
		));
		this->AddRenderPassCallback("Screen Space Reflections Pass", [&](PlazaRenderGraph* plazaRenderGraph, PlazaRenderPass* plazaRenderPass) {
			plazaRenderPass->mPipelines[0]->UpdatePushConstants<SSRPushConstants>(0, SSRPushConstants(glm::vec4(Application::Get()->appSizes->sceneSize, 0.0f, 1.0f), glm::vec4(Application::Get()->activeCamera->Position, 1.0f), Application::Get()->activeCamera->GetProjectionMatrix(), Application::Get()->activeCamera->GetViewMatrix(), Application::Get()->activeCamera->GetProjectionMatrix()));
			});
		/*
layout (binding = 0) uniform sampler2D sceneTexture;
layout (binding = 1) uniform sampler2D normalTexture;
layout (binding = 2) uniform sampler2D depthTexture;

layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 fragColor;

layout(push_constant) uniform PushConstants {
	vec2 screenSize;
	mat4 projection;
	mat4 view;
	mat4 lensProjection;
} pushConstants;
		*/

		// Final Post Processing
		this->AddRenderPass(std::make_shared<VulkanRenderPass>("Final Post Processing Pass", PL_STAGE_VERTEX | PL_STAGE_FRAGMENT, PL_RENDER_PASS_FULL_SCREEN_QUAD, gPassSize, false))
			->AddInputResource(std::make_shared<VulkanTextureBinding>(1, 0, 0, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("SSRTexture")))
			->AddInputResource(std::make_shared<VulkanTextureBinding>(1, 0, 1, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("FontTexture")))
			->AddOutputResource(std::make_shared<VulkanTextureBinding>(1, 0, 0, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0, 0, this->GetSharedTexture("PostProcessedTexture")));

		struct FinalPostProcessingPC {
			float exposure;
			float gamma;
		};
		this->GetRenderPass("Final Post Processing Pass")->AddPipeline(pl::pipelineCreateInfo(
			"FinalShaders",
			PL_RENDER_PASS_FULL_SCREEN_QUAD,
			{ pl::pipelineShaderStageCreateInfo(PL_STAGE_VERTEX, Application::Get()->enginePath + "\\Shaders\\Vulkan\\swapchainDraw.vert", "main"),
				pl::pipelineShaderStageCreateInfo(PL_STAGE_FRAGMENT, Application::Get()->enginePath + "\\Shaders\\Vulkan\\swapchainDraw.frag", "main") },
			VertexGetBindingDescription(),
			VertexGetAttributeDescriptions(),
			PL_TOPOLOGY_TRIANGLE_LIST,
			false,
			pl::pipelineRasterizationStateCreateInfo(false, false, PL_POLYGON_MODE_FILL, 1.0f, false, 0.0f, 0.0f, 0.0f, PL_CULL_MODE_NONE, PL_FRONT_FACE_COUNTER_CLOCKWISE),
			pl::pipelineColorBlendStateCreateInfo({ pl::pipelineColorBlendAttachmentState(true) }),
			pl::pipelineDepthStencilStateCreateInfo(false, false, PL_COMPARE_OP_ALWAYS),
			pl::pipelineViewportStateCreateInfo(1, 1),
			pl::pipelineMultisampleStateCreateInfo(PL_SAMPLE_COUNT_1_BIT, 0),
			{ PL_DYNAMIC_STATE_VIEWPORT, PL_DYNAMIC_STATE_SCISSOR },
			{ pl::pushConstantRange(PL_STAGE_FRAGMENT, 0, sizeof(FinalPostProcessingPC)) }
		));

		// Gui
		struct GuiShadersPC {
			glm::mat4 matrix;
		};
		PlPipelineCreateInfo guiPipelineInfo = pl::pipelineCreateInfo(
			"GuiShaders",
			PL_RENDER_PASS_GUI_RECTANGLE,
			{ pl::pipelineShaderStageCreateInfo(PL_STAGE_VERTEX, Application::Get()->enginePath + "\\Shaders\\Vulkan\\gui\\rectangle.vert", "main"),
				pl::pipelineShaderStageCreateInfo(PL_STAGE_FRAGMENT, Application::Get()->enginePath + "\\Shaders\\Vulkan\\gui\\rectangle.frag", "main") },
			VertexGetBindingDescription(),
			VertexGetAttributeDescriptions(),
			PL_TOPOLOGY_TRIANGLE_LIST,
			false,
			pl::pipelineRasterizationStateCreateInfo(false, false, PL_POLYGON_MODE_FILL, 1.0f, false, 0.0f, 0.0f, 0.0f, PL_CULL_MODE_NONE, PL_FRONT_FACE_COUNTER_CLOCKWISE),
			pl::pipelineColorBlendStateCreateInfo({ pl::pipelineColorBlendAttachmentState(false, PL_BLEND_FACTOR_SRC_ALPHA, PL_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, PL_BLEND_OP_ADD, PL_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, PL_BLEND_FACTOR_ONE) }),
			pl::pipelineDepthStencilStateCreateInfo(false, false, PL_COMPARE_OP_ALWAYS),
			pl::pipelineViewportStateCreateInfo(1, 1),
			pl::pipelineMultisampleStateCreateInfo(PL_SAMPLE_COUNT_1_BIT, 0),
			{ PL_DYNAMIC_STATE_VIEWPORT, PL_DYNAMIC_STATE_SCISSOR },
			{ pl::pushConstantRange(PL_STAGE_ALL, 0, sizeof(GuiShadersPC)) },
			{ 1 }
		);
		this->GetRenderPass("Final Post Processing Pass")->AddPipeline(guiPipelineInfo);
		guiPipelineInfo.renderMethod = PL_RENDER_PASS_GUI_BUTTON;
		guiPipelineInfo.shaderStages = { pl::pipelineShaderStageCreateInfo(PL_STAGE_VERTEX, Application::Get()->enginePath + "\\Shaders\\Vulkan\\gui\\button.vert", "main"),
				pl::pipelineShaderStageCreateInfo(PL_STAGE_FRAGMENT, Application::Get()->enginePath + "\\Shaders\\Vulkan\\gui\\button.frag", "main") };
		this->GetRenderPass("Final Post Processing Pass")->AddPipeline(guiPipelineInfo);

		guiPipelineInfo.depthStencilState = pl::pipelineDepthStencilStateCreateInfo(false, false, PL_COMPARE_OP_ALWAYS);
		guiPipelineInfo.topology = PL_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		guiPipelineInfo.vertexBindingDescriptions = { pl::vertexInputBindingDescription(0, sizeof(glm::vec4), PL_VERTEX_INPUT_RATE_VERTEX), pl::vertexInputBindingDescription(1, sizeof(glm::vec4), PL_VERTEX_INPUT_RATE_VERTEX) };
		guiPipelineInfo.vertexAttributeDescriptions = { pl::vertexInputAttributeDescription(0, 0, PL_FORMAT_R32G32_SFLOAT, 0), pl::vertexInputAttributeDescription(1, 1
			, PL_FORMAT_R32G32_SFLOAT, sizeof(glm::vec2)) };
		guiPipelineInfo.renderMethod = PL_RENDER_PASS_GUI_TEXT;
		guiPipelineInfo.shaderStages = { pl::pipelineShaderStageCreateInfo(PL_STAGE_VERTEX, Application::Get()->enginePath + "\\Shaders\\Vulkan\\gui\\text.vert", "main"),
		pl::pipelineShaderStageCreateInfo(PL_STAGE_FRAGMENT, Application::Get()->enginePath + "\\Shaders\\Vulkan\\gui\\text.frag", "main") };
		this->GetRenderPass("Final Post Processing Pass")->AddPipeline(guiPipelineInfo);

		this->AddRenderPassCallback("Final Post Processing Pass", [&](PlazaRenderGraph* plazaRenderGraph, PlazaRenderPass* plazaRenderPass) {
			plazaRenderPass->mPipelines[0]->UpdatePushConstants<FinalPostProcessingPC>(0, FinalPostProcessingPC(VulkanRenderer::GetRenderer()->exposure, VulkanRenderer::GetRenderer()->gamma));
			plazaRenderPass->mPipelines[1]->UpdatePushConstants<GuiShadersPC>(0, GuiShadersPC(Application::Get()->activeCamera->GetOrthogonalMatrix()));
			plazaRenderPass->mPipelines[2]->UpdatePushConstants<GuiShadersPC>(0, GuiShadersPC(Application::Get()->activeCamera->GetOrthogonalMatrix()));
			plazaRenderPass->mPipelines[3]->UpdatePushConstants<GuiShadersPC>(0, GuiShadersPC(Application::Get()->activeCamera->GetOrthogonalMatrix()));
			});

		// Outline
		this->AddRenderPass(std::make_shared<VulkanRenderPass>("OutlineDrawPass", PL_STAGE_VERTEX | PL_STAGE_FRAGMENT, PL_RENDER_PASS_INDIRECT_BUFFER_SPECIFIC_ENTITY, gPassSize, true))
			->AddOutputResource(std::make_shared<VulkanTextureBinding>(1, 0, 0, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0, 0, this->GetSharedTexture("OutlineTexture")))
			->AddOutputResource(std::make_shared<VulkanTextureBinding>(1, 1, 0, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 0, 0, this->GetSharedTexture("OutlineStencil"), PL_ATTACHMENT_OP_CLEAR, true));

		struct OutlinePC {
			glm::mat4 projection;
			glm::mat4 view;
		};

		this->AddRenderPassCallback("OutlineDrawPass", [&](PlazaRenderGraph* plazaRenderGraph, PlazaRenderPass* plazaRenderPass) {
			plazaRenderPass->mPipelines[0]->UpdatePushConstants<OutlinePC>(0, OutlinePC(Application::Get()->activeCamera->GetProjectionMatrix(), Application::Get()->activeCamera->GetViewMatrix()));


			if (!plazaRenderPass->mPipelines[0]->mIndirectBuffer) {
				std::shared_ptr<PlVkBuffer> buffer = std::make_shared<PlVkBuffer>();
				buffer->CreateBuffer(sizeof(VkDrawIndexedIndirectCommand) * 1024 * 16, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, 0, Application::Get()->mRenderer->mMaxFramesInFlight);
				plazaRenderPass->mPipelines[0]->mIndirectBuffer = buffer;
			}

			plazaRenderPass->mPipelines[0]->mCreateInfo.specificUuids.clear();
			if (Editor::selectedGameObject && Scene::GetActiveScene()->GetEntity(Editor::selectedGameObject->uuid)) {
				GetChilrendUuid(plazaRenderPass->mPipelines[0]->mCreateInfo.specificUuids, Scene::GetActiveScene()->GetEntity(Editor::selectedGameObject->uuid), Scene::GetActiveScene());
			}

			std::vector<VkDrawIndexedIndirectCommand> indirectCommands;
			int totalInstances = 0;
			for (const auto& uuid : plazaRenderPass->mPipelines[0]->mCreateInfo.specificUuids) {
				auto& value = *Scene::GetActiveScene()->GetComponent<MeshRenderer>(uuid)->renderGroup;
				const size_t& instanceCount = value.instanceModelMatrices.size();

				VkDrawIndexedIndirectCommand indirectCommand{};
				indirectCommand.firstIndex = value.mesh->indicesOffset;
				indirectCommand.vertexOffset = value.mesh->verticesOffset;
				indirectCommand.firstInstance = totalInstances;
				indirectCommand.indexCount = value.mesh->indicesCount;
				indirectCommand.instanceCount = 1;
				indirectCommands.push_back(indirectCommand);
			}

			plazaRenderPass->mPipelines[0]->mIndirectBuffer->mCurrentItemCount = indirectCommands.size();
			if (indirectCommands.size() > 0)
				plazaRenderPass->mPipelines[0]->mIndirectBuffer->UpdateData<VkDrawIndexedIndirectCommand>(Application::Get()->mRenderer->mCurrentFrame, indirectCommands.data());

			});

		PlStencilOpState stencilOpState = PlStencilOpState(PL_STENCIL_OP_KEEP, PL_STENCIL_OP_REPLACE, PL_STENCIL_OP_KEEP, PL_COMPARE_OP_ALWAYS, 0xFF, 0xFF, 1);

		PlPipelineCreateInfo outlinePipelineInfo = pl::pipelineCreateInfo(
			"Outline",
			PL_RENDER_PASS_INDIRECT_BUFFER_SPECIFIC_ENTITY,
			{ pl::pipelineShaderStageCreateInfo(PL_STAGE_VERTEX, Application::Get()->enginePath + "\\Shaders\\Vulkan\\outline\\outline.vert", "main"),
				pl::pipelineShaderStageCreateInfo(PL_STAGE_FRAGMENT, Application::Get()->enginePath + "\\Shaders\\Vulkan\\outline\\outline.frag", "main") },
			VertexGetBindingDescription(),
			VertexGetAttributeDescriptions(),
			PL_TOPOLOGY_TRIANGLE_LIST,
			false,
			pl::pipelineRasterizationStateCreateInfo(false, false, PL_POLYGON_MODE_FILL, 1.0f, false, 0.0f, 0.0f, 0.0f, PL_CULL_MODE_NONE, PL_FRONT_FACE_COUNTER_CLOCKWISE),
			pl::pipelineColorBlendStateCreateInfo({ pl::pipelineColorBlendAttachmentState(true) }),
			pl::pipelineDepthStencilStateCreateInfo(false, true, PL_COMPARE_OP_LESS_OR_EQUAL, false, false, stencilOpState, stencilOpState),
			pl::pipelineViewportStateCreateInfo(1, 1),
			pl::pipelineMultisampleStateCreateInfo(PL_SAMPLE_COUNT_1_BIT, 0),
			{ PL_DYNAMIC_STATE_VIEWPORT, PL_DYNAMIC_STATE_SCISSOR },
			{ pl::pushConstantRange(PL_STAGE_VERTEX, 0, sizeof(OutlinePC)) },
			{  }
		);

		std::vector<PlVertexInputBindingDescription> bindingDescriptions{};
		bindingDescriptions.push_back(pl::vertexInputBindingDescription(0, sizeof(Vertex), PL_VERTEX_INPUT_RATE_VERTEX));
		bindingDescriptions.push_back(pl::vertexInputBindingDescription(1, sizeof(glm::vec4) * 4, PL_VERTEX_INPUT_RATE_INSTANCE));
		outlinePipelineInfo.vertexBindingDescriptions = bindingDescriptions;
		std::vector<PlVertexInputAttributeDescription> attributeDescriptions{};
		attributeDescriptions.push_back(pl::vertexInputAttributeDescription(0, 0, PL_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)));
		attributeDescriptions.push_back(pl::vertexInputAttributeDescription(1, 1, PL_FORMAT_R32G32B32A32_SFLOAT, 0));
		attributeDescriptions.push_back(pl::vertexInputAttributeDescription(2, 1, PL_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 4));
		attributeDescriptions.push_back(pl::vertexInputAttributeDescription(3, 1, PL_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 8));
		attributeDescriptions.push_back(pl::vertexInputAttributeDescription(4, 1, PL_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 12));
		outlinePipelineInfo.vertexAttributeDescriptions = attributeDescriptions;

		this->GetRenderPass("OutlineDrawPass")->AddPipeline(outlinePipelineInfo);

		outlinePipelineInfo.pushConstants = {};
		outlinePipelineInfo.pipelineName = "OutlineBlurPass";
		outlinePipelineInfo.renderMethod = PL_RENDER_PASS_FULL_SCREEN_QUAD;
		outlinePipelineInfo.shaderStages = {
			pl::pipelineShaderStageCreateInfo(PL_STAGE_VERTEX, Application::Get()->enginePath + "\\Shaders\\Vulkan\\fullScreenQuad.vert", "main"),
			pl::pipelineShaderStageCreateInfo(PL_STAGE_FRAGMENT, Application::Get()->enginePath + "\\Shaders\\Vulkan\\outline\\outlineBlur.frag", "main") };

		this->AddRenderPass(std::make_shared<VulkanRenderPass>("OutlineBlurPass", PL_STAGE_VERTEX | PL_STAGE_FRAGMENT, PL_RENDER_PASS_FULL_SCREEN_QUAD, gPassSize, false))
			->AddInputResource(std::make_shared<VulkanTextureBinding>(1, 0, 0, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("OutlineTexture")))
			->AddOutputResource(std::make_shared<VulkanTextureBinding>(1, 0, 0, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0, 0, this->GetSharedTexture("OutlineBlurredTexture")));
		this->GetRenderPass("OutlineBlurPass")->AddPipeline(outlinePipelineInfo);

		outlinePipelineInfo.depthStencilState = pl::pipelineDepthStencilStateCreateInfo(true, false, PL_COMPARE_OP_LESS_OR_EQUAL, false, false);
		outlinePipelineInfo.shaderStages = {
			pl::pipelineShaderStageCreateInfo(PL_STAGE_VERTEX, Application::Get()->enginePath + "\\Shaders\\Vulkan\\fullScreenQuad.vert", "main"),
			pl::pipelineShaderStageCreateInfo(PL_STAGE_FRAGMENT, Application::Get()->enginePath + "\\Shaders\\Vulkan\\outline\\outlineSceneMerge.frag", "main") };

		outlinePipelineInfo.pipelineName = "OutlineSceneMergePass";
		this->AddRenderPass(std::make_shared<VulkanRenderPass>("OutlineSceneMergePass", PL_STAGE_VERTEX | PL_STAGE_FRAGMENT, PL_RENDER_PASS_FULL_SCREEN_QUAD, gPassSize, false))
			->AddInputResource(std::make_shared<VulkanTextureBinding>(1, 0, 0, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("PostProcessedTexture")))
			->AddInputResource(std::make_shared<VulkanTextureBinding>(1, 0, 1, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("OutlineBlurredTexture")))
			->AddOutputResource(std::make_shared<VulkanTextureBinding>(1, 0, 0, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0, 0, this->GetSharedTexture("FinalTexture")));
		//->AddOutputResource(std::make_shared<VulkanTextureBinding>(1, 1, 0, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 0, 0, this->GetSharedTexture("OutlineStencil"), PL_ATTACHMENT_OP_LOAD, true));
		stencilOpState = PlStencilOpState(PL_STENCIL_OP_KEEP, PL_STENCIL_OP_KEEP, PL_STENCIL_OP_KEEP, PL_COMPARE_OP_EQUAL, 0xFF, 0xFF, 1);
		outlinePipelineInfo.depthStencilState = pl::pipelineDepthStencilStateCreateInfo(false, false, PL_COMPARE_OP_ALWAYS, false, true, stencilOpState, stencilOpState);
		this->GetRenderPass("OutlineSceneMergePass")->AddPipeline(outlinePipelineInfo);

		this->OrderPasses();
		this->UpdateUsedTexturesInfo();
	}

	VulkanRenderGraph* VulkanRenderGraph::BuildSkyboxRenderGraph() {
		VulkanRenderGraph* skyboxRenderGraph = new VulkanRenderGraph();

		const unsigned int faceSize = 512;
		const unsigned int brdfSize = 512;
		const unsigned int irradianceSize = 64;

		static EquirectangularToCubeMapPC pushConstants{};

		PlPipelineCreateInfo pipelineCreateInfo = pl::pipelineCreateInfo(
			"EquirectangularToCubeMapShaders",
			PL_RENDER_PASS_CUBE,
			{ pl::pipelineShaderStageCreateInfo(PL_STAGE_VERTEX, Application::Get()->enginePath + "\\Shaders\\Vulkan\\skybox\\equirectangularToCubemap.vert", "main"),
				pl::pipelineShaderStageCreateInfo(PL_STAGE_FRAGMENT, Application::Get()->enginePath + "\\Shaders\\Vulkan\\skybox\\equirectangularToCubemap.frag", "main") },
			{ },
			{ },
			PL_TOPOLOGY_TRIANGLE_LIST,
			false,
			pl::pipelineRasterizationStateCreateInfo(false, false, PL_POLYGON_MODE_FILL, 1.0f, false, 0.0f, 0.0f, 0.0f, PL_CULL_MODE_NONE, PL_FRONT_FACE_COUNTER_CLOCKWISE),
			pl::pipelineColorBlendStateCreateInfo({ pl::pipelineColorBlendAttachmentState(true) }),
			pl::pipelineDepthStencilStateCreateInfo(false, false, PL_COMPARE_OP_ALWAYS),
			pl::pipelineViewportStateCreateInfo(1, 1),
			pl::pipelineMultisampleStateCreateInfo(PL_SAMPLE_COUNT_1_BIT, 0),
			{ PL_DYNAMIC_STATE_VIEWPORT, PL_DYNAMIC_STATE_SCISSOR },
			{ pl::pushConstantRange(PL_STAGE_ALL, 0, sizeof(EquirectangularToCubeMapPC)) }
		);

		skyboxRenderGraph->AddRenderPass(std::make_shared<VulkanRenderPass>("EquirectangularToCubeMapPass", PL_STAGE_VERTEX | PL_STAGE_FRAGMENT, PL_RENDER_PASS_CUBE, glm::vec2(faceSize, faceSize), true))
			->AddInputResource(std::make_shared<VulkanTextureBinding>(1, 0, 1, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("EquirectangularTexture")))
			->AddOutputResource(std::make_shared<VulkanTextureBinding>(1, 0, 0, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0, 0, this->GetSharedTexture("CubeMapTexture")))
			->AddPipeline(pipelineCreateInfo);

		pipelineCreateInfo.pipelineName = "BrdfGeneratorShaders";
		pipelineCreateInfo.renderMethod = PL_RENDER_PASS_FULL_SCREEN_QUAD;
		pipelineCreateInfo.shaderStages = {
		pl::pipelineShaderStageCreateInfo(PL_STAGE_VERTEX, Application::Get()->enginePath + "\\Shaders\\Vulkan\\skybox\\brdfGenerator.vert", "main"),
		pl::pipelineShaderStageCreateInfo(PL_STAGE_FRAGMENT, Application::Get()->enginePath + "\\Shaders\\Vulkan\\skybox\\brdfGenerator.frag", "main") };
		pipelineCreateInfo.pushConstants = {};
		skyboxRenderGraph->AddRenderPass(std::make_shared<VulkanRenderPass>("BrdfGeneratorPass", PL_STAGE_VERTEX | PL_STAGE_FRAGMENT, PL_RENDER_PASS_FULL_SCREEN_QUAD, glm::vec2(brdfSize, brdfSize), true))
			->AddOutputResource(std::make_shared<VulkanTextureBinding>(1, 0, 0, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0, 0, this->GetSharedTexture("SamplerBRDFLUT")))
			->AddPipeline(pipelineCreateInfo);

		pipelineCreateInfo.pipelineName = "IrradianceGeneratorShaders";
		pipelineCreateInfo.renderMethod = PL_RENDER_PASS_CUBE;
		pipelineCreateInfo.shaderStages = {
		pl::pipelineShaderStageCreateInfo(PL_STAGE_VERTEX, Application::Get()->enginePath + "\\Shaders\\Vulkan\\skybox\\equirectangularToCubemap.vert", "main"),
		pl::pipelineShaderStageCreateInfo(PL_STAGE_FRAGMENT, Application::Get()->enginePath + "\\Shaders\\Vulkan\\skybox\\irradianceGenerator.frag", "main") };
		pipelineCreateInfo.pushConstants = { pl::pushConstantRange(PL_STAGE_ALL, 0, sizeof(EquirectangularToCubeMapPC)) };
		skyboxRenderGraph->AddRenderPass(std::make_shared<VulkanRenderPass>("IrradianceGeneratorPass", PL_STAGE_VERTEX | PL_STAGE_FRAGMENT, PL_RENDER_PASS_CUBE, glm::vec2(irradianceSize, irradianceSize), false))
			->AddInputResource(std::make_shared<VulkanTextureBinding>(1, 0, 2, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("CubeMapTexture")))
			->AddOutputResource(std::make_shared<VulkanTextureBinding>(1, 0, 0, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0, 0, this->GetSharedTexture("IrradianceMap")))
			->AddPipeline(pipelineCreateInfo);

		pipelineCreateInfo.pipelineName = "PreFilteredGeneratorShaders";
		pipelineCreateInfo.renderMethod = PL_RENDER_PASS_CUBE;
		pipelineCreateInfo.shaderStages = {
		pl::pipelineShaderStageCreateInfo(PL_STAGE_VERTEX, Application::Get()->enginePath + "\\Shaders\\Vulkan\\skybox\\equirectangularToCubemap.vert", "main"),
		pl::pipelineShaderStageCreateInfo(PL_STAGE_FRAGMENT, Application::Get()->enginePath + "\\Shaders\\Vulkan\\skybox\\prefilterEnvGenerator.frag", "main") };
		pipelineCreateInfo.pushConstants = { pl::pushConstantRange(PL_STAGE_ALL, 0, sizeof(EquirectangularToCubeMapPC)) };
		skyboxRenderGraph->AddRenderPass(std::make_shared<VulkanRenderPass>("PreFilteredGeneratorPass", PL_STAGE_VERTEX | PL_STAGE_FRAGMENT, PL_RENDER_PASS_CUBE, glm::vec2(faceSize, faceSize), false))
			->AddInputResource(std::make_shared<VulkanTextureBinding>(1, 0, 0, PL_BUFFER_COMBINED_IMAGE_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, this->GetSharedTexture("CubeMapTexture")))
			->AddOutputResource(std::make_shared<VulkanTextureBinding>(1, 0, 0, PL_BUFFER_SAMPLER, PL_STAGE_FRAGMENT, PL_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0, 0, this->GetSharedTexture("PreFilterMap")))
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
			glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
		};

		const std::vector<glm::mat4> equirectangularToCubeMapMatrices = {
			glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
		};

		EquirectangularToCubeMapPC converterPushConstants{};
		converterPushConstants.first = true;

		VkCommandBuffer commandBuffer = VulkanRenderer::GetRenderer()->BeginSingleTimeCommands();

		renderGraph->GetRenderPass("EquirectangularToCubeMapPass")->mPipelines[0]->mPushConstants[0].mData = new EquirectangularToCubeMapPC();;
		for (uint32_t i = 0; i < 6; i++) {
			VkImageViewCreateInfo layerImageViewCreateInfo = {};
			layerImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			layerImageViewCreateInfo.image = this->GetTexture<VulkanTexture>("CubeMapTexture")->mImage;//mSkyboxTexture->mImage;
			layerImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			layerImageViewCreateInfo.format = PlImageFormatToVkFormat(skyboxFormat);
			layerImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			layerImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
			layerImageViewCreateInfo.subresourceRange.levelCount = 1;
			layerImageViewCreateInfo.subresourceRange.baseArrayLayer = i;
			layerImageViewCreateInfo.subresourceRange.layerCount = 1;

			VkImageView layerImageView;
			vkCreateImageView(VulkanRenderer::GetRenderer()->mDevice, &layerImageViewCreateInfo, nullptr, &layerImageView);

			std::vector<VkImageView> frameBufferAttachments{ layerImageView };

			VkFramebuffer framebuffer = VulkanRenderer::GetRenderer()->CreateFramebuffer(renderGraph->GetRenderPass("EquirectangularToCubeMapPass")->mRenderPass,
				glm::vec2(faceSize, faceSize), frameBufferAttachments.data(), frameBufferAttachments.size(), 1);

			converterPushConstants.mvp = glm::perspective((float)(glm::pi<double>() / 2.0), 1.0f, 0.1f, static_cast<float>(faceSize)) * equirectangularToCubeMapMatrices[i];
			renderGraph->GetRenderPass("EquirectangularToCubeMapPass")->mFrameBuffer = framebuffer;
			renderGraph->GetRenderPass("EquirectangularToCubeMapPass")->UpdateCommandBuffer(commandBuffer);
			renderGraph->GetRenderPass("EquirectangularToCubeMapPass")->mPipelines[0]->mPushConstants[0].mData = &converterPushConstants;
			memcpy(renderGraph->GetRenderPass("EquirectangularToCubeMapPass")->mPipelines[0]->mPushConstants[0].mData, &converterPushConstants, sizeof(EquirectangularToCubeMapPC));
			renderGraph->GetRenderPass("EquirectangularToCubeMapPass")->Execute(Scene::GetActiveScene(), renderGraph);
		}

		VulkanRenderer::GetRenderer()->EndSingleTimeCommands(commandBuffer);

		commandBuffer = VulkanRenderer::GetRenderer()->BeginSingleTimeCommands();
		renderGraph->GetRenderPass("BrdfGeneratorPass")->UpdateCommandBuffer(commandBuffer);
		renderGraph->GetRenderPass("BrdfGeneratorPass")->Execute(Scene::GetActiveScene(), renderGraph);
		VulkanRenderer::GetRenderer()->EndSingleTimeCommands(commandBuffer);

		commandBuffer = VulkanRenderer::GetRenderer()->BeginSingleTimeCommands();
		renderGraph->GetRenderPass("IrradianceGeneratorPass")->mPipelines[0]->mPushConstants[0].mData = new EquirectangularToCubeMapPC();
		converterPushConstants.first = false;
		converterPushConstants.deltaPhi = (2.0f * float(3.14159265358979323846)) / 180.0f;
		converterPushConstants.deltaTheta = (0.5f * float(3.14159265358979323846)) / 64.0f;
		for (uint32_t i = 0; i < 6; i++) {
			VkImageViewCreateInfo layerImageViewCreateInfo = {};
			layerImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			layerImageViewCreateInfo.image = this->GetTexture<VulkanTexture>("IrradianceMap")->mImage;//mSkyboxTexture->mImage;
			layerImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			layerImageViewCreateInfo.format = PlImageFormatToVkFormat(skyboxFormat);
			layerImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			layerImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
			layerImageViewCreateInfo.subresourceRange.levelCount = 1;
			layerImageViewCreateInfo.subresourceRange.baseArrayLayer = i;
			layerImageViewCreateInfo.subresourceRange.layerCount = 1;

			VkImageView layerImageView;
			vkCreateImageView(VulkanRenderer::GetRenderer()->mDevice, &layerImageViewCreateInfo, nullptr, &layerImageView);

			std::vector<VkImageView> frameBufferAttachments{ layerImageView };

			VkFramebuffer framebuffer = VulkanRenderer::GetRenderer()->CreateFramebuffer(renderGraph->GetRenderPass("IrradianceGeneratorPass")->mRenderPass,
				glm::vec2(irradianceSize, irradianceSize), frameBufferAttachments.data(), frameBufferAttachments.size(), 1);

			converterPushConstants.mvp = glm::perspective((float)(glm::pi<double>() / 2.0), 1.0f, 0.1f, static_cast<float>(faceSize)) * matrices[i];
			renderGraph->GetRenderPass("IrradianceGeneratorPass")->mFrameBuffer = framebuffer;
			renderGraph->GetRenderPass("IrradianceGeneratorPass")->UpdateCommandBuffer(commandBuffer);
			renderGraph->GetRenderPass("IrradianceGeneratorPass")->mPipelines[0]->mPushConstants[0].mData = &converterPushConstants;
			memcpy(renderGraph->GetRenderPass("IrradianceGeneratorPass")->mPipelines[0]->mPushConstants[0].mData, &converterPushConstants, sizeof(EquirectangularToCubeMapPC));
			renderGraph->GetRenderPass("IrradianceGeneratorPass")->Execute(Scene::GetActiveScene(), renderGraph);
		}
		VulkanRenderer::GetRenderer()->EndSingleTimeCommands(commandBuffer);

		commandBuffer = VulkanRenderer::GetRenderer()->BeginSingleTimeCommands();
		renderGraph->GetRenderPass("PreFilteredGeneratorPass")->mPipelines[0]->mPushConstants[0].mData = new EquirectangularToCubeMapPC();
		converterPushConstants.first = false;
		converterPushConstants.deltaPhi = (2.0f * float(3.14159265358979323846)) / 180.0f;
		converterPushConstants.deltaTheta = (0.5f * float(3.14159265358979323846)) / 64.0f;
		for (uint32_t m = 0; m < numMips; m++) {
			converterPushConstants.roughness = (float)m / (float)(numMips - 1);
			for (uint32_t i = 0; i < 6; i++) {
				glm::vec2 currentSize = glm::vec2(faceSize * std::pow(0.5f, m), faceSize * std::pow(0.5f, m));
				VkImageViewCreateInfo layerImageViewCreateInfo = {};
				layerImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				layerImageViewCreateInfo.image = this->GetTexture<VulkanTexture>("PreFilterMap")->mImage;//mSkyboxTexture->mImage;
				layerImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				layerImageViewCreateInfo.format = PlImageFormatToVkFormat(skyboxFormat);
				layerImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				layerImageViewCreateInfo.subresourceRange.baseMipLevel = m;
				layerImageViewCreateInfo.subresourceRange.levelCount = 1;
				layerImageViewCreateInfo.subresourceRange.baseArrayLayer = i;
				layerImageViewCreateInfo.subresourceRange.layerCount = 1;

				VkImageView layerImageView;
				vkCreateImageView(VulkanRenderer::GetRenderer()->mDevice, &layerImageViewCreateInfo, nullptr, &layerImageView);

				std::vector<VkImageView> frameBufferAttachments{ layerImageView };

				VkFramebuffer framebuffer = VulkanRenderer::GetRenderer()->CreateFramebuffer(renderGraph->GetRenderPass("PreFilteredGeneratorPass")->mRenderPass,
					currentSize, frameBufferAttachments.data(), frameBufferAttachments.size(), 1);

				converterPushConstants.mvp = glm::perspective((float)(glm::pi<double>() / 2.0), 1.0f, 0.1f, static_cast<float>(faceSize)) * matrices[i];
				renderGraph->GetRenderPass("PreFilteredGeneratorPass")->mFrameBuffer = framebuffer;
				renderGraph->GetRenderPass("PreFilteredGeneratorPass")->UpdateCommandBuffer(commandBuffer);
				renderGraph->GetRenderPass("PreFilteredGeneratorPass")->mPipelines[0]->mPushConstants[0].mData = &converterPushConstants;
				memcpy(renderGraph->GetRenderPass("PreFilteredGeneratorPass")->mPipelines[0]->mPushConstants[0].mData, &converterPushConstants, sizeof(EquirectangularToCubeMapPC));
				renderGraph->GetRenderPass("PreFilteredGeneratorPass")->mRenderSize = currentSize;
				renderGraph->GetRenderPass("PreFilteredGeneratorPass")->Execute(Scene::GetActiveScene(), renderGraph);
			}
		}
		VulkanRenderer::GetRenderer()->EndSingleTimeCommands(commandBuffer);
	}

	void VulkanRenderPass::CompilePipeline(std::shared_ptr<PlazaPipeline> plazaPipeline) {
		plazaPipeline->mCompiled = true;
		VulkanPlazaPipeline* pipeline = static_cast<VulkanPlazaPipeline*>(plazaPipeline.get());
		PlPipelineCreateInfo createInfo = pipeline->mCreateInfo;

		std::vector<VkPushConstantRange> pushConstants = std::vector<VkPushConstantRange>();
		if (pipeline->mPushConstants.size() == 0) {
			for (PlPushConstantRange range : createInfo.pushConstants) {
				pushConstants.push_back(plvk::pushConstantRange(PlRenderStageToVkShaderStage(range.stageFlags), range.offset, range.size));
				pipeline->mPushConstants.push_back(PlPushConstants(range.stageFlags, range.offset, range.size));
			}
		}

		bool isComputeShaders = createInfo.renderMethod == PL_RENDER_PASS_COMPUTE && createInfo.shaderStages.size() > 0;
		if (isComputeShaders) {
			pipeline->mComputeShaders->mComputeDescriptorSetLayout = mDescriptorSetLayout;
			pipeline->mComputeShaders->Init(createInfo.shaderStages[0].shadersPath, pushConstants);
			return;
		}

		pipeline->mRenderPass = this->mRenderPass;
		pipeline->mFramebuffer = this->mFrameBuffer;
		pipeline->mShaders->mDescriptorSetLayout = mDescriptorSetLayout;
		pipeline->mShaders->mDescriptorSets = mDescriptorSets;


		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = plvk::pipelineLayoutCreateInfo(1, &pipeline->mShaders->mDescriptorSetLayout, pushConstants.size(), pushConstants.data());

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages{};
		for (size_t i = 0; i < createInfo.shaderStages.size(); ++i) {
			const PlPipelineShaderStageCreateInfo& stage = createInfo.shaderStages[i];

			VkPipelineShaderStageCreateInfo shaderStage = {};
			shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStage.stage = static_cast<VkShaderStageFlagBits>(PlRenderStageToVkShaderStage(stage.stage));
			shaderStage.pName = createInfo.shaderStages[i].name.c_str();

			std::string shadersPath = stage.shadersPath;
			if (!shadersPath.ends_with(".spv"))
				shadersPath = VulkanShadersCompiler::Compile(shadersPath);
			auto shaderCode = VulkanShaders::ReadFile(shadersPath.c_str());
			shaderStage.module = VulkanShaders::CreateShaderModule(shaderCode, VulkanRenderer::GetRenderer()->mDevice);

			shaderStages.push_back(shaderStage);
		}

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
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = plvk::pipelineVertexInputStateCreateInfo(bindings, attributes);
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = plvk::pipelineInputAssemblyStateCreateInfo(PlTopologyToVkTopology(createInfo.topology), createInfo.primitiveRestartEnable);
		VkPipelineRasterizationStateCreateInfo rasterizationState = plvk::pipelineRasterizationStateCreateInfo(createInfo.rasterization.depthClampEnable, createInfo.rasterization.rasterizerDiscardEnable, PlPolygonModeToVkPolygonMode(createInfo.rasterization.polygonMode), createInfo.rasterization.lineWidth, createInfo.rasterization.depthBiasEnable, createInfo.rasterization.depthBiasConstantFactor, createInfo.rasterization.depthBiasClamp, createInfo.rasterization.depthBiasSlopeFactor, PlCullModeToVkCullMode(createInfo.rasterization.cullMode), PlFrontFaceToVkFrontFace(createInfo.rasterization.frontFace));
		std::vector<VkPipelineColorBlendAttachmentState> blendAttachments{};
		for (const PlPipelineColorBlendAttachmentState& attachment : createInfo.colorBlendState.attachments) {
			blendAttachments.push_back(plvk::pipelineColorBlendAttachmentState(attachment.blendEnable,
				PlBlendFactorToVkBlendFactor(attachment.srcColorBlendFactor),
				PlBlendFactorToVkBlendFactor(attachment.dstColorBlendFactor),
				PlBlendOpToVkBlendOp(attachment.colorBlendOp),
				PlBlendFactorToVkBlendFactor(attachment.srcAlphaBlendFactor),
				PlBlendFactorToVkBlendFactor(attachment.dstAlphaBlendFactor),
				PlBlendOpToVkBlendOp(attachment.alphaBlendOp),
				attachment.colorWriteMask));
		}
		VkPipelineColorBlendStateCreateInfo colorBlendState = plvk::pipelineColorBlendStateCreateInfo(blendAttachments.size(), blendAttachments.data(), createInfo.colorBlendState.logicOpEnable, PlLogicOpToVkLogicOp(createInfo.colorBlendState.logicOp), std::vector<float>(std::begin(createInfo.colorBlendState.blendConstants), std::end(createInfo.colorBlendState.blendConstants)));
		VkPipelineDepthStencilStateCreateInfo depthStencilState = plvk::pipelineDepthStencilStateCreateInfo(createInfo.depthStencilState.depthTestEnable, createInfo.depthStencilState.depthWriteEnable, PlCompareOpToVkCompareOp(createInfo.depthStencilState.depthCompareOp), createInfo.depthStencilState.depthBoundsTestEnable, createInfo.depthStencilState.stencilTestEnable);
		VkPipelineViewportStateCreateInfo viewportState = plvk::pipelineViewportStateCreateInfo(createInfo.viewPortState.viewportCount, createInfo.viewPortState.scissorsCount);
		VkPipelineMultisampleStateCreateInfo multisampleState = plvk::pipelineMultisampleStateCreateInfo(PlSampleCountToVkSampleCount(createInfo.multiSampleState.rasterizationSamples), createInfo.multiSampleState.sampleShadingEnable);
		std::vector<VkDynamicState> dynamicStates{};
		for (PlDynamicState state : createInfo.dynamicStates) {
			dynamicStates.push_back(PlDynamicStateToVkDynamicState(state));
		}
		VkPipelineDynamicStateCreateInfo dynamicState = plvk::pipelineDynamicStateCreateInfo(dynamicStates);
		pipeline->mShaders->InitializeFull(
			VulkanRenderer::GetRenderer()->mDevice,
			pipelineLayoutCreateInfo,
			createInfo.vertexAttributeDescriptions.size() > 0 || createInfo.vertexBindingDescriptions.size() > 0 ? true : false,
			mRenderSize.x,
			mRenderSize.y,
			shaderStages,
			vertexInputInfo,
			inputAssemblyState,
			viewportState,
			rasterizationState,
			multisampleState,
			colorBlendState,
			dynamicState,
			mRenderPass,
			depthStencilState
		);
		//mPipelines.push_back(plazaPipeline);
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

	void VulkanRenderPass::ReCompileShaders(bool resetCompiledBool) {
		VulkanRenderer::GetRenderer()->WaitRendererHere();
		if (resetCompiledBool)
			ResetPipelineCompiledBool();
		for (auto& pipeline : mPipelines) {
			if (this->mRenderMethod == PL_RENDER_PASS_HOLDER)
				continue;

			if (!pipeline->mCompiled) {
				PlPipelineCreateInfo createInfo = pipeline->mCreateInfo;
				this->TerminatePipeline(pipeline);
				for (auto& shadersStage : pipeline->mCreateInfo.shaderStages) {
					VulkanShadersCompiler::Compile(shadersStage.shadersPath);
				}
				pipeline->mPushConstants.clear();
				this->CompilePipeline(pipeline);
			}
			else {
				VulkanPlazaPipeline* vkPipeline = static_cast<VulkanPlazaPipeline*>(pipeline.get());
				mDescriptorSetLayout = vkPipeline->mShaders->mDescriptorSetLayout != VK_NULL_HANDLE ? vkPipeline->mShaders->mDescriptorSetLayout : vkPipeline->mComputeShaders->mComputeDescriptorSetLayout;
			}

		}
		for (auto& child : mChildPasses) {
			child->ReCompileShaders(false);
		}
	}


	void VulkanRenderGraph::CreatePipeline(PlPipelineCreateInfo createInfo) {}

	void VulkanRenderGraph::AddPipeline() {}

	bool compareRenderPasses(const std::shared_ptr<PlazaRenderPass>& a, const std::shared_ptr<PlazaRenderPass>& b) {
		return a->mExecutionIndex < b->mExecutionIndex;
	}

	void OrderPassDependencies(int16_t  currentIndex, std::shared_ptr<PlazaRenderPass> pass, std::set<std::string>& alreadyOrderedPasses) {
		alreadyOrderedPasses.emplace(pass->mName);
		pass->mExecutionIndex = currentIndex;//pass->mExecutionIndex > currentIndex ? pass->mExecutionIndex : currentIndex;
		for (const auto& dependent : pass->mDependents) {
			if (dependent.second != pass)
				OrderPassDependencies(currentIndex + 1, dependent.second, alreadyOrderedPasses);
			//dependency.second->mExecutionIndex = currentIndex;
		}
	}

	void VulkanRenderGraph::OrderPasses() {
		std::map<std::string, BindingModifiers> bindingsModifiers = std::map<std::string, BindingModifiers>();

		//mOrderedPasses.clear();
		for (const auto& [key, pass] : mPasses) {
			//mOrderedPasses.push_back(pass);
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
			if (alreadyOrderedPasses.find(mOrderedPasses[i]->mName) != alreadyOrderedPasses.end() || mOrderedPasses[i]->mDependents.size() == 0)
				continue;
			mOrderedPasses[i]->mExecutionIndex = 0;
			OrderPassDependencies(0, mOrderedPasses[mOrderedPasses[i]->mExecutionIndex], alreadyOrderedPasses);
		}

		std::sort(mOrderedPasses.begin(), mOrderedPasses.end(), compareRenderPasses);
		for (unsigned int i = 0; i < mOrderedPasses.size(); ++i) {
			mOrderedPasses[i]->mExecutionIndex = i;
		}
	}

	void VulkanBufferBinding::Compile(std::set<std::string>& compiledBindings) {
		//mBuffer = std::make_shared<PlVkBuffer>();
		if (compiledBindings.find(mName) != compiledBindings.end())
			return;
		mBuffer->CreateBuffer(mBuffer->mMaxItems * mBuffer->mStride, PlBufferUsageToVkBufferUsage(mBuffer->mBufferUsage), PlMemoryUsageToVmaMemoryUsage(mBuffer->mMemoryUsage), 0, mBuffer->mBufferCount);
		mBuffer->CreateMemory(0, mBuffer->mBufferCount);
	}
	void VulkanBufferBinding::Destroy() {

	}

	void VulkanTextureBinding::Compile(std::set<std::string>& compiledBindings) {
		if (mMaxBindlessResources > 0)
			return;

		bool viewMustBeNonDefault = mBaseMipLevel != 0 || mBaseLayerLevel != 0;

		VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
		if (this->GetTextureInfo().mImageUsage & PL_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT) {
			aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
			if (VulkanRenderer::IsFormatStencil(PlImageFormatToVkFormat(this->GetTextureInfo().mFormat)))
				aspect = VK_IMAGE_ASPECT_STENCIL_BIT;
		};


		if (viewMustBeNonDefault) {
			mNonDefaultView = VulkanRenderer::GetRenderer()->CreateImageView(this->GetTexture()->mImage, this->GetTexture()->GetFormat(), aspect, PlViewTypeToVkImageViewType(GetTextureInfo().mViewType),
				GetTextureInfo().mLayersCount, 1, mBaseMipLevel);
			return;
		}

		if (compiledBindings.find(mName) != compiledBindings.end())
			return;

		VkImageUsageFlags flags = 0;
		if (GetTextureInfo().mViewType == PL_VIEW_TYPE_CUBE)
			flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		if (GetTextureInfo().mPath == "" || GetTextureInfo().mPath == "deferred") {
			this->GetTexture()->CreateTextureImage(
				VulkanRenderer::GetRenderer()->mDevice,
				PlImageFormatToVkFormat(GetTextureInfo().mFormat),
				mTexture->mResolution.x,
				mTexture->mResolution.y,
				mTexture->mMipCount == 0 ? true : false,
				PlImageUsageToVkImageUsage(GetTextureInfo().mImageUsage),
				PlTextureTypeToVkImageType(GetTextureInfo().mTextureType),
				GetTextureInfo().mPath == "deferred" ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED,
				GetTextureInfo().mLayersCount,
				flags,
				true,
				VK_SHARING_MODE_EXCLUSIVE);
			VulkanRenderer::GetRenderer()->TransitionTextureLayout(*this->GetTexture(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VulkanRenderer::GetFormatAspectMask(PlImageFormatToVkFormat(mTexture->GetTextureInfo().mFormat)), this->GetTextureInfo().mLayersCount, this->mTexture->mMipCount);
		}
		else {
			this->GetTexture()->CreateTextureImage(VulkanRenderer::GetRenderer()->mDevice, GetTextureInfo().mPath, PlImageFormatToVkFormat(GetTextureInfo().mFormat), true, GetTextureInfo().mIsHdr, PlImageUsageToVkImageUsage(GetTextureInfo().mImageUsage));
		}

		if (GetTextureInfo().mInitialLayout != PL_IMAGE_LAYOUT_UNDEFINED) {
			VulkanRenderer::GetRenderer()->TransitionTextureLayout(*this->GetTexture(), PlImageLayoutToVkImageLayout(GetTextureInfo().mInitialLayout), aspect, this->GetTexture()->GetTextureInfo().mLayersCount, this->mTexture->mMipCount);
		}

		this->GetTexture()->CreateTextureSampler(PlAddressModeToVkSamplerAddressMode(GetTextureInfo().mSamplerAddressMode));
		this->GetTexture()->CreateImageView(PlImageFormatToVkFormat(GetTextureInfo().mFormat), aspect, PlViewTypeToVkImageViewType(GetTextureInfo().mViewType), GetTextureInfo().mLayersCount, 0);

		VulkanRenderer::GetRenderer()->AddTrackerToImage(this->GetTexture()->mImage, mTexture->mAssetName, this->GetTexture()->mSampler, this->GetTexture()->GetTextureInfo(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}
	void VulkanTextureBinding::Destroy() {

	}

	void VulkanRenderPass::CompileGraphics(PlazaRenderGraph* renderGraph) {
		glm::vec2 biggestSize = this->mRenderSize;//glm::vec2(0.0f);
		std::vector<VkImageView> frameBufferAttachments{};
		std::vector<VkSubpassDescription> subPasses{};
		std::vector<VkSubpassDependency> dependencies{};
		std::vector<VkAttachmentDescription> attachmentDescs{};
		std::vector<uint32_t> locations{};
		std::vector<VkAttachmentReference> colorReferences;
		VkAttachmentReference depthReference = {};

		for (unsigned int i = 0; i < glm::max(mOutputBindings.size(), mInputBindings.size()); ++i) {
			std::shared_ptr<PlazaShadersBinding> value;
			//for (const auto& value : mOutputBindings) {
			if (mInputBindings.size() > i && mInputBindings[i]->mUseAsDepthStencilAttachment)
				value = mInputBindings[i];
			else if (mOutputBindings.size() > i)
				value = mOutputBindings[i];
			else
				continue;

			// Get next layout of each output binding of this pass
			int nextPassToUseThisBinding = -1;
			for (unsigned int i = mExecutionIndex; i < renderGraph->mOrderedPasses.size(); ++i) {
				auto& pass = renderGraph->mOrderedPasses.at(i);
				if (pass->GetInputResource<PlazaShadersBinding>(value->mName) != nullptr) {
					nextPassToUseThisBinding = i;
					break;
				}
			}

			VulkanTextureBinding* binding = static_cast<VulkanTextureBinding*>(value.get());
			biggestSize = glm::max(biggestSize, glm::vec2(binding->mTexture->mResolution));

			VkImageLayout finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			if (binding->mInitialLayout == PL_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
				finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			else if (binding->mInitialLayout == PL_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
				finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			//if (value->mName == "SceneDepth")
			//	finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; //VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL

			// Find the first pass to use the resource used in this binding
			int firstPassToUseThisResource = mExecutionIndex;
			while (firstPassToUseThisResource < renderGraph->mOrderedPasses.size() && firstPassToUseThisResource > 0) {
				if (renderGraph->mOrderedPasses[firstPassToUseThisResource]->GetInputResource<VulkanTextureBinding>(value->mName))
					break;
				firstPassToUseThisResource--;
			}
			if (firstPassToUseThisResource == 0)
				firstPassToUseThisResource = mExecutionIndex;
			if (firstPassToUseThisResource != renderGraph->mOrderedPasses.size() && firstPassToUseThisResource > 0) {
				firstPassToUseThisResource = mExecutionIndex;
				while (firstPassToUseThisResource < renderGraph->mOrderedPasses.size()) {
					if (renderGraph->mOrderedPasses[firstPassToUseThisResource]->GetOutputResource<VulkanTextureBinding>(value->mName))
						break;
					firstPassToUseThisResource--;
				}
			}

			//static_cast<VulkanTextureBinding*>(value.get());//renderGraph->mOrderedPasses[glm::max(mExecutionIndex - 1, 0)]->GetInputResource<VulkanTextureBinding>(binding->GetTexture()->mAssetName);
			VulkanTextureBinding* currentPassBinding = nullptr;
			if (firstPassToUseThisResource != renderGraph->mOrderedPasses.size() && firstPassToUseThisResource != mExecutionIndex)
				currentPassBinding = renderGraph->mOrderedPasses[glm::max(firstPassToUseThisResource, 0)]->GetInputResource<VulkanTextureBinding>(binding->GetTexture()->mAssetName);
			//currentPassBinding = static_cast<VulkanTextureBinding*>(value.get());//
		//currentPassBinding = renderGraph->mOrderedPasses[glm::max(mExecutionIndex - 1, 0)]->GetInputResource<VulkanTextureBinding>(binding->GetTexture()->mAssetName);
		//currentPassBinding = renderGraph->mOrderedPasses[glm::max(firstPassToUseThisResource, 0)]->GetInputResource<VulkanTextureBinding>(binding->GetTexture()->mAssetName);

			VkImageLayout currentLayout = mExecutionIndex == 0 || currentPassBinding == nullptr ? VK_IMAGE_LAYOUT_UNDEFINED : PlImageLayoutToVkImageLayout(currentPassBinding->mInitialLayout); //PlImageLayoutToVkImageLayout(binding->GetTexture()->mFutureLayouts[mExecutionIndex - 1]);
			VulkanTextureBinding* nextPassBinding = nextPassToUseThisBinding >= 0 ? renderGraph->mOrderedPasses[nextPassToUseThisBinding]->GetInputResource<VulkanTextureBinding>(binding->GetTexture()->mAssetName) : nullptr;//mExecutionIndex + 1 < renderGraph->mOrderedPasses.size() ? renderGraph->mOrderedPasses[mExecutionIndex + 1]->GetInputResource<VulkanTextureBinding>(binding->GetTexture()->mAssetName) : nullptr;
			VkImageLayout nextLayout = nextPassToUseThisBinding == -1 ? finalLayout : PlImageLayoutToVkImageLayout(nextPassBinding->mInitialLayout); //PlImageLayoutToVkImageLayout(binding->GetTexture()->mFutureLayouts[mExecutionIndex]);

			VkAttachmentLoadOp loadOp = currentLayout == VK_IMAGE_LAYOUT_UNDEFINED ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
			VkAttachmentStoreOp storeOp = nextLayout == VK_IMAGE_LAYOUT_UNDEFINED ? VK_ATTACHMENT_STORE_OP_DONT_CARE : VK_ATTACHMENT_STORE_OP_STORE;
			VkAttachmentLoadOp loadStencilOp = currentLayout == VK_IMAGE_LAYOUT_UNDEFINED ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
			VkAttachmentStoreOp storeStencilOp = nextLayout == VK_IMAGE_LAYOUT_UNDEFINED ? VK_ATTACHMENT_STORE_OP_DONT_CARE : VK_ATTACHMENT_STORE_OP_STORE;

			if (binding->mAttachmentOp == PL_ATTACHMENT_OP_LOAD) {
				loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
				loadStencilOp = VK_ATTACHMENT_LOAD_OP_LOAD;
				nextLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				currentLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			}
			else if (binding->mAttachmentOp == PL_ATTACHMENT_OP_CLEAR) {
				loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				loadStencilOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			}
			else if (binding->mAttachmentOp == PL_ATTACHMENT_OP_STORE) {
				storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				storeStencilOp = VK_ATTACHMENT_STORE_OP_STORE;
			}

			if (currentLayout == VK_IMAGE_LAYOUT_UNDEFINED && binding && binding->GetTexture()->GetFormat() == VK_FORMAT_D32_SFLOAT_S8_UINT && !binding->mUseAsDepthStencilAttachment) {
				nextLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
				finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			}

			//if (currentLayout == PL_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			//	currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDescs.push_back(
				plvk::attachmentDescription(PlImageFormatToVkFormat(binding->GetTextureInfo().mFormat),
					VK_SAMPLE_COUNT_1_BIT,
					loadOp,
					storeOp,
					loadStencilOp,
					storeStencilOp,
					currentLayout,
					nextLayout));

			VulkanTexture* texture = binding->GetTexture();
			frameBufferAttachments.push_back(binding->GetTexture()->mImageView);
			locations.push_back(binding->mLocation);

			VkClearValue clearValue{};
			if (binding->mInitialLayout == PL_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL || binding->mInitialLayout == PL_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
				clearValue.depthStencil = { 1.0f, 0 };
			else
				clearValue.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
			mClearValues.push_back(clearValue);

			// Set depth or color references based on usage
			if (binding->GetTextureInfo().mImageUsage & PL_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT) {
				depthReference.attachment = binding->mLocation;
				depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				continue;
			}
			colorReferences.push_back({ binding->mLocation, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

		}

		// Sort the attachments to make location 0 image view be in index 0
		std::vector<VkAttachmentDescription> temporaryAttachmentDescs = attachmentDescs;
		std::vector<VkImageView> temporaryFrameBufferAttachments = frameBufferAttachments;
		for (unsigned int i = 0; i < locations.size(); ++i) {
			attachmentDescs[i] = temporaryAttachmentDescs[locations[i]];
			frameBufferAttachments[i] = temporaryFrameBufferAttachments[locations[i]];
		}

		// Define the subpass description
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.pColorAttachments = colorReferences.data();
		subpass.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
		if (depthReference.layout != VK_IMAGE_LAYOUT_UNDEFINED)
			subpass.pDepthStencilAttachment = &depthReference;

		// Add external dependencies to ensure synchronization outside the render pass
		dependencies.push_back(
			plvk::subpassDependency(VK_SUBPASS_EXTERNAL, 0,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT));

		// Add subpass dependencies for other stages if needed...
		subPasses.push_back(subpass);

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

		mRenderPass = VulkanRenderer::GetRenderer()->CreateRenderPass(attachmentDescs.data(), attachmentDescs.size(), subPasses.data(), subPasses.size(), dependencies.data(), dependencies.size(), next);

		// Frame buffer
		mFrameBuffer = VulkanRenderer::GetRenderer()->CreateFramebuffer(mRenderPass, biggestSize, frameBufferAttachments.data(), frameBufferAttachments.size(), 1);
	}

	void VulkanRenderPass::Compile(PlazaRenderGraph* renderGraph) {
		for (auto& binding : mInputBindings) {
			binding->Compile(renderGraph->mCompiledBindings);
			renderGraph->mCompiledBindings.insert(binding->mName);
		}

		for (auto& binding : mOutputBindings) {
			binding->Compile(renderGraph->mCompiledBindings);
			renderGraph->mCompiledBindings.insert(binding->mName);
		}


		if (mRenderMethod != PL_RENDER_PASS_COMPUTE && mRenderMethod != PL_RENDER_PASS_HOLDER)
			CompileGraphics(renderGraph);

		// Descriptor sets
		std::vector<VkDescriptorSetLayoutBinding> descriptorSets{};
		std::vector<VkDescriptorBindingFlagsEXT> bindingFlags = std::vector<VkDescriptorBindingFlagsEXT>();
		for (const auto& value : mInputBindings) {
			this->GetBindingDescriptorSet(value, descriptorSets, bindingFlags);
		}
		if (mRenderMethod == PL_RENDER_PASS_COMPUTE) {
			for (const auto& value : mOutputBindings) {
				this->GetBindingDescriptorSet(value, descriptorSets, bindingFlags);
			}
		}

		VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extendedInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT,	nullptr };
		extendedInfo.pBindingFlags = bindingFlags.data();
		extendedInfo.bindingCount = static_cast<uint32_t>(descriptorSets.size());

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = plvk::descriptorSetLayoutCreateInfo(descriptorSets, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT, &extendedInfo);

		if (mDescriptorSetLayout == VK_NULL_HANDLE)
			vkCreateDescriptorSetLayout(VulkanRenderer::GetRenderer()->mDevice, &descriptorSetLayoutCreateInfo, nullptr, &mDescriptorSetLayout);

		std::vector<VkDescriptorSetLayout> layouts(Application::Get()->mRenderer->mMaxFramesInFlight, mDescriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO
		};
		allocInfo.descriptorPool = VulkanRenderer::GetRenderer()->mDescriptorPool;
		allocInfo.descriptorSetCount = layouts.size();
		allocInfo.pSetLayouts = layouts.data();

		VkDescriptorSetVariableDescriptorCountAllocateInfoEXT countInfo{
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT
		};
		static const uint32_t maxBindlessResources = 16536;
		std::vector<uint32_t> maxBinding(Application::Get()->mRenderer->mMaxFramesInFlight, Application::Get()->mRenderer->mMaxBindlessTextures - 1);
		countInfo.descriptorSetCount = Application::Get()->mRenderer->mMaxFramesInFlight;
		countInfo.pDescriptorCounts = maxBinding.data();
		//allocInfo.pNext = &countInfo;
		mDescriptorSets.resize(Application::Get()->mRenderer->mMaxFramesInFlight);
		VkResult res = vkAllocateDescriptorSets(VulkanRenderer::GetRenderer()->mDevice, &allocInfo, mDescriptorSets.data());
		if (res != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}
		for (unsigned int i = 0; i < Application::Get()->mRenderer->mMaxFramesInFlight; ++i) {
			std::vector<VkWriteDescriptorSet> descriptorWrites{};

			std::vector<VkDescriptorBufferInfo*> bufferInfos{};
			std::vector<VkDescriptorImageInfo*> imageInfos{};
			for (const auto& value : mInputBindings) {
				if (value->mUseAsDepthStencilAttachment)
					continue;
				GetBindingWriteInfo(value, i, descriptorWrites, bufferInfos, imageInfos);
			}

			if (mRenderMethod == PL_RENDER_PASS_COMPUTE) {
				for (const auto& value : mOutputBindings) {
					GetBindingWriteInfo(value, i, descriptorWrites, bufferInfos, imageInfos);
				}
			}

			vkUpdateDescriptorSets(VulkanRenderer::GetRenderer()->mDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}

		for (auto& pipeline : mPipelines) {
			if (!pipeline->mCompiled) {
				this->CompilePipeline(pipeline);
			}
			else {
				VulkanPlazaPipeline* vkPipeline = static_cast<VulkanPlazaPipeline*>(pipeline.get());
				mDescriptorSetLayout = vkPipeline->mShaders->mDescriptorSetLayout != VK_NULL_HANDLE ? vkPipeline->mShaders->mDescriptorSetLayout : vkPipeline->mComputeShaders->mComputeDescriptorSetLayout;
			}
		}

		for (auto& pass : mChildPasses) {
			static_cast<VulkanRenderPass*>(pass.get())->mDescriptorSetLayout = mDescriptorSetLayout;
			pass->Compile(renderGraph);
		}

	}


	void VulkanRenderPass::GetBindingDescriptorSet(const shared_ptr<PlazaShadersBinding>& binding, std::vector<VkDescriptorSetLayoutBinding>& descriptorSets, std::vector<VkDescriptorBindingFlagsEXT>& bindingFlags) {
		VkDescriptorBindingFlags bindlessFlags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;
		switch (binding->mBindingType) {
		case PL_BINDING_BUFFER:
			descriptorSets.push_back(static_cast<VulkanBufferBinding*>(binding.get())->GetDescriptorLayout());
			bindingFlags.push_back(binding->mMaxBindlessResources > 0 ? bindlessFlags : 0);
			break;
		case PL_BINDING_TEXTURE:
			descriptorSets.push_back(static_cast<VulkanTextureBinding*>(binding.get())->GetDescriptorLayout());
			bindingFlags.push_back(binding->mMaxBindlessResources > 0 ? bindlessFlags : 0);
			break;
		}
	}
	void VulkanRenderPass::GetBindingWriteInfo(const shared_ptr<PlazaShadersBinding>& binding, unsigned int i, std::vector<VkWriteDescriptorSet>& descriptorWrites, std::vector<VkDescriptorBufferInfo*>& bufferInfos, std::vector<VkDescriptorImageInfo*>& imageInfos) {
		switch (binding->mBindingType) {
		case PL_BINDING_BUFFER: {
			VulkanBufferBinding* bufferBinding = static_cast<VulkanBufferBinding*>(binding.get());
			VkDescriptorBufferInfo bufferInfo = bufferBinding->GetBufferInfo(i);
			bufferInfos.push_back(new VkDescriptorBufferInfo(bufferInfo));
			descriptorWrites.push_back(bufferBinding->GetDescriptorWrite(mDescriptorSets[i], bufferInfos[bufferInfos.size() - 1]));
			break;
		}
		case PL_BINDING_TEXTURE: {
			VulkanTextureBinding* textureBinding = static_cast<VulkanTextureBinding*>(binding.get());
			VkDescriptorImageInfo imageInfo = textureBinding->GetImageInfo();
			if (binding->mMaxBindlessResources > 0) {
				imageInfo = plvk::descriptorImageInfo(VK_IMAGE_LAYOUT_UNDEFINED, VK_NULL_HANDLE, VK_NULL_HANDLE);
				break;
			}
			imageInfo.imageLayout = PlImageLayoutToVkImageLayout(textureBinding->mInitialLayout);
			imageInfos.push_back(new VkDescriptorImageInfo(imageInfo));
			descriptorWrites.push_back(textureBinding->GetDescriptorWrite(mDescriptorSets[i], imageInfos[imageInfos.size() - 1]));
			break;
		}
		}
	}

	void VulkanRenderPass::BindMainBuffers() {
		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(mCommandBuffer, 0, 1, mRenderMethod == PL_RENDER_PASS_INDIRECT_BUFFER_SKINNED ?
			&VulkanRenderer::GetRenderer()->mSkinnedVertexBuffer->GetBuffer() : &VulkanRenderer::GetRenderer()->mMainVertexBuffer->GetBuffer(), offsets);
		vkCmdBindVertexBuffers(mCommandBuffer, 1, 1, &VulkanRenderer::GetRenderer()->mMainInstanceMatrixBuffers[VulkanRenderer::GetRenderer()->mCurrentFrame], offsets);
		vkCmdBindIndexBuffer(mCommandBuffer, VulkanRenderer::GetRenderer()->mMainIndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
	}

	void VulkanRenderPass::BindRenderPass() {
		VkRenderPassBeginInfo renderPassInfo = plvk::renderPassBeginInfo(this->mRenderPass, this->mFrameBuffer,
			mRenderSize.x, mRenderSize.y, 0, 0, mClearValues.size(), mClearValues.data());
		vkCmdBeginRenderPass(mCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = plvk::viewport(0.0f, mFlipViewPort ? mRenderSize.y : 0.0f, mRenderSize.x, mFlipViewPort ? -static_cast<float>(mRenderSize.y) : mRenderSize.y);
		vkCmdSetViewport(mCommandBuffer, 0, 1, &viewport);
		VkRect2D scissor = plvk::rect2D(0, 0, mRenderSize.x, mRenderSize.y);
		vkCmdSetScissor(mCommandBuffer, 0, 1, &scissor);
	}

	void VulkanRenderPass::EndRenderPass() {
		vkCmdEndRenderPass(mCommandBuffer);
	}

	bool VulkanRenderGraph::BindPass(std::string passName) {
		if (mPasses.find(passName) == mPasses.end())
			return false;
		VulkanRenderPass* renderPass = this->GetRenderPass(passName);

		renderPass->BindRenderPass();
	}

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
		//VulkanRenderer::GetRenderer()->TransitionImageLayout(VulkanRenderer::GetRenderer()->mSwapChainImages[currentFrame], VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

		for (unsigned int i = 0; i < mOrderedPasses.size(); ++i) {
			this->GetRenderPass(mOrderedPasses[i]->mName)->UpdateCommandBuffer(commandBuffer);
			for (const auto& child : mOrderedPasses[i]->mChildPasses) {
				static_cast<VulkanRenderPass*>(child.get())->UpdateCommandBuffer(commandBuffer);
			}
#ifdef GAME_MODE
			if (i == mOrderedPasses.size() - 1) {
				static_cast<VulkanRenderPass*>(mOrderedPasses[i].get())->mFrameBuffer = VulkanRenderer::GetRenderer()->mSwapChainFramebuffers[imageIndex];
				static_cast<VulkanRenderPass*>(mOrderedPasses[i].get())->mRenderPass = VulkanRenderer::GetRenderer()->mSwapchainRenderPass;
			}
#endif
			this->GetRenderPass(mOrderedPasses[i]->mName)->Execute(scene, this);
		}

		/* Render ImGui if in Editor build */
#ifdef EDITOR_MODE
		std::array<VkClearValue, 1> clearValues{};
		clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
		VkRenderPassBeginInfo renderPassInfo = plvk::renderPassBeginInfo(VulkanRenderer::GetRenderer()->mSwapchainRenderPass, VulkanRenderer::GetRenderer()->mSwapChainFramebuffers[imageIndex],
			VulkanRenderer::GetRenderer()->mSwapChainExtent.width, VulkanRenderer::GetRenderer()->mSwapChainExtent.height, 0, 0, 1, clearValues.data());

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		//vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, VulkanRenderer::GetRenderer()->mGraphicsPipeline);

		VkViewport viewport = plvk::viewport(0.0f, 0.0f, VulkanRenderer::GetRenderer()->mSwapChainExtent.width, VulkanRenderer::GetRenderer()->mSwapChainExtent.height);
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		VkRect2D scissor = plvk::rect2D(0, 0, VulkanRenderer::GetRenderer()->mSwapChainExtent.width, VulkanRenderer::GetRenderer()->mSwapChainExtent.height);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		//vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, VulkanRenderer::GetRenderer()->mPipelineLayout, 0, 1, &VulkanRenderer::GetRenderer()->mFinalSceneDescriptorSet, 0, nullptr);

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
		vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->mShaders->mPipelineLayout, 0, 1, &mDescriptorSets[VulkanRenderer::GetRenderer()->mCurrentFrame], 0, nullptr);
		for (const PlPushConstants& pushConstant : vulkanPipeline->mPushConstants) {
			vkCmdPushConstants(mCommandBuffer, vulkanPipeline->mShaders->mPipelineLayout, PlRenderStageToVkShaderStage(pushConstant.mStage), pushConstant.mOffset, pushConstant.mStride, pushConstant.mData);
		}
		vkCmdDrawIndexedIndirect(mCommandBuffer, VulkanRenderer::GetRenderer()->mIndirectBuffers[VulkanRenderer::GetRenderer()->mCurrentFrame], pipeline->mIndirectBufferOffset, VulkanRenderer::GetRenderer()->mIndirectDrawCount, sizeof(VkDrawIndexedIndirectCommand));
	}

	void VulkanRenderPass::RenderIndirectBufferShadowMap(PlazaPipeline* pipeline) {
		VulkanPlazaPipeline* vulkanPipeline = static_cast<VulkanPlazaPipeline*>(pipeline);
		vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->mShaders->mPipeline);
		vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->mShaders->mPipelineLayout, 0, 1, &mDescriptorSets[VulkanRenderer::GetRenderer()->mCurrentFrame], 0, nullptr);
		for (const PlPushConstants& pushConstant : vulkanPipeline->mPushConstants) {
			vkCmdPushConstants(mCommandBuffer, vulkanPipeline->mShaders->mPipelineLayout, PlRenderStageToVkShaderStage(pushConstant.mStage), pushConstant.mOffset, pushConstant.mStride, pushConstant.mData);
		}
		vkCmdDrawIndexedIndirect(mCommandBuffer, VulkanRenderer::GetRenderer()->mIndirectBuffers[VulkanRenderer::GetRenderer()->mCurrentFrame], pipeline->mIndirectBufferOffset, VulkanRenderer::GetRenderer()->mIndirectDrawCount, sizeof(VkDrawIndexedIndirectCommand));
	}

	void VulkanRenderPass::RenderIndirectBufferSpecificEntity(PlazaPipeline* pipeline) {
		VulkanPlazaPipeline* vulkanPipeline = static_cast<VulkanPlazaPipeline*>(pipeline);
		vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->mShaders->mPipeline);
		vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->mShaders->mPipelineLayout, 0, 1, &mDescriptorSets[VulkanRenderer::GetRenderer()->mCurrentFrame], 0, nullptr);
		for (const PlPushConstants& pushConstant : pipeline->mPushConstants) {
			vkCmdPushConstants(mCommandBuffer, vulkanPipeline->mShaders->mPipelineLayout, PlRenderStageToVkShaderStage(pushConstant.mStage), pushConstant.mOffset, pushConstant.mStride, pushConstant.mData);
		}

		if (pipeline->mIndirectBuffer && pipeline->mIndirectBuffer->mCurrentItemCount > 0) {
			vkCmdDrawIndexedIndirect(mCommandBuffer, static_cast<PlVkBuffer*>(pipeline->mIndirectBuffer.get())->GetBuffer(VulkanRenderer::GetRenderer()->mCurrentFrame),
				pipeline->mIndirectBufferOffset, pipeline->mIndirectBuffer->mCurrentItemCount, sizeof(VkDrawIndexedIndirectCommand));
		}
	}

	void VulkanRenderPass::RenderIndirectBufferSpecificMesh(PlazaPipeline* pipeline) {
		VulkanPlazaPipeline* vulkanPipeline = static_cast<VulkanPlazaPipeline*>(pipeline);
		vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->mShaders->mPipeline);
		vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->mShaders->mPipelineLayout, 0, 1, &mDescriptorSets[VulkanRenderer::GetRenderer()->mCurrentFrame], 0, nullptr);
		for (const PlPushConstants& pushConstant : pipeline->mPushConstants) {
			vkCmdPushConstants(mCommandBuffer, vulkanPipeline->mShaders->mPipelineLayout, PlRenderStageToVkShaderStage(pushConstant.mStage), pushConstant.mOffset, pushConstant.mStride, pushConstant.mData);
		}
		for (auto& meshUuid : pipeline->mCreateInfo.specificUuids) {
			Mesh* mesh = AssetsManager::GetMesh(meshUuid);
			if (!mesh) continue;
			vkCmdDrawIndexed(mCommandBuffer, static_cast<uint32_t>(mesh->indices.size()), 1, mesh->indicesOffset, mesh->verticesOffset, mesh->instanceOffset);
		}
	}

	void VulkanRenderPass::RenderIndirectBufferSkinned(PlazaPipeline* pipeline) {
		//VulkanPlazaPipeline* vulkanPipeline = static_cast<VulkanPlazaPipeline*>(pipeline);
		//vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->mShaders->mPipeline);
		//vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->mShaders->mPipelineLayout, 0, 1, &mDescriptorSets[VulkanRenderer::GetRenderer()->mCurrentFrame], 0, nullptr);
		//for (const PlPushConstants& pushConstant : vulkanPipeline->mPushConstants) {
		//	vkCmdPushConstants(mCommandBuffer, vulkanPipeline->mShaders->mPipelineLayout, PlRenderStageToVkShaderStage(pushConstant.mStage), pushConstant.mOffset, pushConstant.mStride, pushConstant.mData);
		//}
		//vkCmdDrawIndexedIndirect(mCommandBuffer, VulkanRenderer::GetRenderer()->mIndirectBuffers[VulkanRenderer::GetRenderer()->mCurrentFrame], pipeline->mIndirectBufferOffset, VulkanRenderer::GetRenderer()->mIndirectDrawCount, sizeof(VkDrawIndexedIndirectCommand));
	}

	void VulkanRenderPass::RenderFullScreenQuad(PlazaPipeline* pipeline) {
		VulkanPlazaPipeline* vulkanPipeline = static_cast<VulkanPlazaPipeline*>(pipeline);
		vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->mShaders->mPipeline);
		vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->mShaders->mPipelineLayout, 0, 1, &mDescriptorSets[VulkanRenderer::GetRenderer()->mCurrentFrame], 0, nullptr);
		for (const PlPushConstants& pushConstant : vulkanPipeline->mPushConstants) {
			vkCmdPushConstants(mCommandBuffer, vulkanPipeline->mShaders->mPipelineLayout, PlRenderStageToVkShaderStage(pushConstant.mStage), pushConstant.mOffset, pushConstant.mStride, pushConstant.mData);
		}
		vkCmdDraw(mCommandBuffer, 3, 1, 0, 0);
	}

	void VulkanRenderPass::RenderCube(PlazaPipeline* pipeline) {
		VulkanPlazaPipeline* vulkanPipeline = static_cast<VulkanPlazaPipeline*>(pipeline);
		vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->mShaders->mPipeline);
		vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->mShaders->mPipelineLayout, 0, 1, &mDescriptorSets[VulkanRenderer::GetRenderer()->mCurrentFrame], 0, nullptr);
		for (const PlPushConstants& pushConstant : vulkanPipeline->mPushConstants) {
			vkCmdPushConstants(mCommandBuffer, vulkanPipeline->mShaders->mPipelineLayout, PlRenderStageToVkShaderStage(pushConstant.mStage), pushConstant.mOffset, pushConstant.mStride, pushConstant.mData);
		}
		vkCmdDraw(mCommandBuffer, 36, 1, 0, 0);
	}

	void VulkanRenderPass::RunCompute(PlazaPipeline* pipeline) {
		VulkanPlazaPipeline* vulkanPipeline = static_cast<VulkanPlazaPipeline*>(pipeline);
		VkPipelineLayout pipelineLayout = vulkanPipeline->mComputeShaders->mComputePipelineLayout;

		//VkCommandBuffer commandBuffer = VulkanRenderer::GetRenderer()->BeginSingleTimeCommands();
		vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, vulkanPipeline->mComputeShaders->mComputePipeline);
		vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, vulkanPipeline->mComputeShaders->mComputePipelineLayout, 0, 1, &mDescriptorSets[VulkanRenderer::GetRenderer()->mCurrentFrame], 0, nullptr);
		for (const PlPushConstants& pushConstant : vulkanPipeline->mPushConstants) {
			vkCmdPushConstants(mCommandBuffer, vulkanPipeline->mComputeShaders->mComputePipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, pushConstant.mOffset, pushConstant.mStride, pushConstant.mData);
		}
		vkCmdDispatch(mCommandBuffer, mDispatchSize.x, mDispatchSize.y, mDispatchSize.z);

		if (this->GetInputResource<VulkanTextureBinding>("BloomTexture")) {
			VkImageMemoryBarrier imageMemoryBarrier = {};
			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
			imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
			imageMemoryBarrier.image = this->GetInputResource<VulkanTextureBinding>("BloomTexture")->GetTexture()->mImage;
			imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
			imageMemoryBarrier.subresourceRange.levelCount = this->GetInputResource<VulkanTextureBinding>("BloomTexture")->mTexture->mMipCount;
			imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
			imageMemoryBarrier.subresourceRange.layerCount = 1;

			imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			vkCmdPipelineBarrier(mCommandBuffer,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &imageMemoryBarrier);
		}
		//VulkanRenderer::GetRenderer()->EndSingleTimeCommands(commandBuffer);
	}

	void VulkanRenderPass::RenderGui(Scene* scene, PlazaPipeline* pipeline) {
		VulkanPlazaPipeline* vulkanPipeline = static_cast<VulkanPlazaPipeline*>(pipeline);
		vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->mShaders->mPipeline);
		vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->mShaders->mPipelineLayout, 0, 1, &mDescriptorSets[VulkanRenderer::GetRenderer()->mCurrentFrame], 0, nullptr);
		for (const PlPushConstants& pushConstant : pipeline->mPushConstants) {
			vkCmdPushConstants(mCommandBuffer, vulkanPipeline->mShaders->mPipelineLayout, PlRenderStageToVkShaderStage(pushConstant.mStage), pushConstant.mOffset, pushConstant.mStride, pushConstant.mData);
		}
		std::vector<glm::mat4> rectanglesTransform = std::vector<glm::mat4>();
		std::vector<glm::mat4> buttonsTransform = std::vector<glm::mat4>();
		std::vector<glm::mat4> textsTransform = std::vector<glm::mat4>();
		std::vector<glm::mat4>* vector;
		for (const uint64_t& uuid : SceneView<GuiComponent>(scene)) {
			GuiComponent& component = *scene->GetComponent<GuiComponent>(uuid);
			for (auto& [itemUuid, item] : component.mGuiItems) {
				switch (item->mGuiType) {
				case GuiType::PL_GUI_RECTANGLE: vector = &rectanglesTransform; break;
				case GuiType::PL_GUI_BUTTON: vector = &buttonsTransform; break;
				case GuiType::PL_GUI_TEXT: vector = &buttonsTransform; break;
				}
				vector->push_back(item->mTransform);
			}
		}

		PlVkBuffer* buffer = VulkanRenderer::GetRenderer()->mRenderGraph->GetBuffer<PlVkBuffer>("RectanglesTransformBuffer");
		Mesh* meshe = AssetsManager::GetMesh(1);
		VulkanRenderer::GetRenderer()->mInstanceModelMatrices.resize(VulkanRenderer::GetRenderer()->mInstanceModelMatrices.size() + rectanglesTransform.size());
		if (rectanglesTransform.size() > 0) {
			for (unsigned int i = 0; i < rectanglesTransform.size(); ++i) {
				VulkanRenderer::GetRenderer()->mInstanceModelMatrices[meshe->instanceOffset + i] = rectanglesTransform[i];
			}
		}
		void* data;
		size_t bufferSize = sizeof(glm::mat4) * VulkanRenderer::GetRenderer()->mInstanceModelMatrices.size();
		vmaMapMemory(VulkanRenderer::GetRenderer()->mVmaAllocator, buffer->GetAllocation(VulkanRenderer::GetRenderer()->mCurrentFrame), &data);
		memcpy(data, VulkanRenderer::GetRenderer()->mInstanceModelMatrices.data(), bufferSize);
		vmaUnmapMemory(VulkanRenderer::GetRenderer()->mVmaAllocator, buffer->GetAllocation(VulkanRenderer::GetRenderer()->mCurrentFrame));

		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(mCommandBuffer, 1, 1, &buffer->GetBuffer(Application::Get()->mRenderer->mCurrentFrame), offsets);
		for (auto& meshUuid : pipeline->mCreateInfo.specificUuids) {
			Mesh* mesh = AssetsManager::GetMesh(meshUuid);
			if (!mesh) continue;
			vkCmdDrawIndexed(mCommandBuffer, static_cast<uint32_t>(mesh->indices.size()), glm::max<int>(rectanglesTransform.size(), 1), mesh->indicesOffset, mesh->verticesOffset, mesh->instanceOffset);
		}
	}

	void VulkanRenderPass::RenderGuiRectangle(Scene* scene, PlazaPipeline* pipeline) {
		VulkanPlazaPipeline* vulkanPipeline = static_cast<VulkanPlazaPipeline*>(pipeline);
		vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->mShaders->mPipeline);
		vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->mShaders->mPipelineLayout, 0, 1, &mDescriptorSets[VulkanRenderer::GetRenderer()->mCurrentFrame], 0, nullptr);
		for (const PlPushConstants& pushConstant : pipeline->mPushConstants) {
			vkCmdPushConstants(mCommandBuffer, vulkanPipeline->mShaders->mPipelineLayout, PlRenderStageToVkShaderStage(pushConstant.mStage), pushConstant.mOffset, pushConstant.mStride, pushConstant.mData);
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
				case GuiType::PL_GUI_RECTANGLE: vector = &rectanglesTransform; break;
				case GuiType::PL_GUI_BUTTON: vector = &buttonsTransform; break;
				case GuiType::PL_GUI_TEXT: vector = &buttonsTransform; break;
				}
				vector->push_back(item->mTransform);
			}
		}

		PlVkBuffer* buffer = VulkanRenderer::GetRenderer()->mRenderGraph->GetBuffer<PlVkBuffer>("RectanglesTransformBuffer");
		Mesh* meshe = AssetsManager::GetMesh(1);
		VulkanRenderer::GetRenderer()->mInstanceModelMatrices.resize(VulkanRenderer::GetRenderer()->mInstanceModelMatrices.size() + rectanglesTransform.size());
		if (rectanglesTransform.size() > 0) {
			for (unsigned int i = 0; i < rectanglesTransform.size(); ++i) {
				VulkanRenderer::GetRenderer()->mInstanceModelMatrices[meshe->instanceOffset + i] = rectanglesTransform[i];
			}
		}
		void* data;
		size_t bufferSize = sizeof(glm::mat4) * VulkanRenderer::GetRenderer()->mInstanceModelMatrices.size();
		vmaMapMemory(VulkanRenderer::GetRenderer()->mVmaAllocator, buffer->GetAllocation(VulkanRenderer::GetRenderer()->mCurrentFrame), &data);
		memcpy(data, VulkanRenderer::GetRenderer()->mInstanceModelMatrices.data(), bufferSize);
		vmaUnmapMemory(VulkanRenderer::GetRenderer()->mVmaAllocator, buffer->GetAllocation(VulkanRenderer::GetRenderer()->mCurrentFrame));

		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(mCommandBuffer, 1, 1, &buffer->GetBuffer(Application::Get()->mRenderer->mCurrentFrame), offsets);
		for (auto& meshUuid : pipeline->mCreateInfo.specificUuids) {
			Mesh* mesh = AssetsManager::GetMesh(meshUuid);
			if (!mesh) mesh = AssetsManager::GetMesh(1);
			vkCmdDrawIndexed(mCommandBuffer, static_cast<uint32_t>(mesh->indices.size()), glm::max<int>(rectanglesTransform.size(), 1), mesh->indicesOffset, mesh->verticesOffset, mesh->instanceOffset);
		}
	}

	void VulkanRenderPass::RenderGuiButton(Scene* scene, PlazaPipeline* pipeline) {
		VulkanPlazaPipeline* vulkanPipeline = static_cast<VulkanPlazaPipeline*>(pipeline);
		vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->mShaders->mPipeline);
		vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->mShaders->mPipelineLayout, 0, 1, &mDescriptorSets[VulkanRenderer::GetRenderer()->mCurrentFrame], 0, nullptr);

		//for (auto& [key, value] : Scene::GetActiveScene()->guiComponents) {
			//for (auto& [itemUuid, item] : value.mGuiItems) {
				//if (item->mGuiType != GuiType::PL_GUI_BUTTON)
				//	continue;
				//
				//PlVkBuffer* textsBuffer = VulkanRenderer::GetRenderer()->mRenderGraph->GetBuffer<PlVkBuffer>("GuiTextVerticesBuffer");
				//glm::vec4* mapped;
				//VkCommandBuffer cmdBuffer = mCommandBuffer;
				//vkMapMemory(VulkanRenderer::GetRenderer()->mDevice, textsBuffer->GetMemory(VulkanRenderer::GetRenderer()->mCurrentFrame), 0, VK_WHOLE_SIZE, 0, (void**)&mapped);
				//int numLetters = 0;
				//
				//GuiButton* button = static_cast<GuiButton*>(item.get());
				////for (auto& [key, value] : Scene::GetActiveScene()->UITextRendererComponents) {
				//static_cast<VulkanGuiRenderer*>(VulkanRenderer::GetRenderer()->mGuiRenderer)->AddText(button->mText, button->GetPosition().x, button->GetPosition().y, button->mTextScale, VulkanGuiRenderer::TextAlign::alignLeft, mapped, &numLetters);
				////}
				//
				//vkUnmapMemory(VulkanRenderer::GetRenderer()->mDevice, textsBuffer->GetMemory(VulkanRenderer::GetRenderer()->mCurrentFrame));
				//mapped = nullptr;
				//
				//VkDeviceSize offsets = 0;
				//vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &textsBuffer->GetBuffer(VulkanRenderer::GetRenderer()->mCurrentFrame), &offsets);
				//vkCmdBindVertexBuffers(cmdBuffer, 1, 1, &textsBuffer->GetBuffer(VulkanRenderer::GetRenderer()->mCurrentFrame), &offsets);
				//
				//// One draw command for every character. This is okay for a debug overlay, but not optimal
				//// In a real-world application one would try to batch draw commands
				//for (uint32_t j = 0; j < numLetters; j++) {
				//	vkCmdDraw(cmdBuffer, 4, 1, j * 4, 0);
				//}
			//}
		//}
	}

	void VulkanRenderPass::RenderGuiText(Scene* scene, PlazaPipeline* pipeline) {
		VulkanPlazaPipeline* vulkanPipeline = static_cast<VulkanPlazaPipeline*>(pipeline);
		vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->mShaders->mPipeline);
		vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->mShaders->mPipelineLayout, 0, 1, &mDescriptorSets[VulkanRenderer::GetRenderer()->mCurrentFrame], 0, nullptr);

		PlVkBuffer* textsBuffer = VulkanRenderer::GetRenderer()->mRenderGraph->GetBuffer<PlVkBuffer>("GuiTextVerticesBuffer");

		glm::vec4* mapped = nullptr;
		std::vector<glm::vec4> data = std::vector<glm::vec4>();
		int count = 0;
		int numLetters = 0;
		vmaMapMemory(VulkanRenderer::GetRenderer()->mVmaAllocator, textsBuffer->GetAllocation(VulkanRenderer::GetRenderer()->mCurrentFrame), (void**)&mapped);
		for (const uint64_t& uuid : SceneView<GuiComponent>(scene)) {
			GuiComponent& value = *scene->GetComponent<GuiComponent>(uuid);
			if (!value.mEnabled)
				continue;

			for (auto& [itemUuid, item] : value.mGuiItems) {
				if (item->mGuiType != GuiType::PL_GUI_BUTTON)
					continue;

				GuiButton* button = static_cast<GuiButton*>(item.get());
				VulkanGuiRenderer::AddText(button->mText, button->GetWorldPosition().x, button->GetWorldPosition().y, button->mTextScale, VulkanGuiRenderer::TextAlign::alignCenter, mapped, numLetters);
				count++;

			}
		}
		vmaUnmapMemory(VulkanRenderer::GetRenderer()->mVmaAllocator, textsBuffer->GetAllocation(VulkanRenderer::GetRenderer()->mCurrentFrame));
		mapped = nullptr;
		VkDeviceSize offsets = 0;
		vkCmdBindVertexBuffers(mCommandBuffer, 0, 1, &textsBuffer->GetBuffer(VulkanRenderer::GetRenderer()->mCurrentFrame), &offsets);
		vkCmdBindVertexBuffers(mCommandBuffer, 1, 1, &textsBuffer->GetBuffer(VulkanRenderer::GetRenderer()->mCurrentFrame), &offsets);

		// One draw command for every character. This is okay for a debug overlay, but not optimal
		// In a real-world application one would try to batch draw commands
		for (uint32_t j = 0; j < numLetters; j++) {
			vkCmdDraw(mCommandBuffer, 4, 1, j * 4, 0);
		}
	}
}