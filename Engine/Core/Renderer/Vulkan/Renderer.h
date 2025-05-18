#pragma once
#include "Engine/Core/Renderer/Renderer.h"
#include "Engine/Components/Core/Entity.h"
#include "Engine/Core/Renderer/Vulkan/ShadersCompiler.h"

#include "Mesh.h"
#include "Engine/Core/Renderer/Mesh.h"

#include "VulkanRenderGraph.h"
#include "VulkanTexture.h"
#include "VulkanPicking.h"
#include "VulkanComputeShaders.h"

#include "ThirdParty/include/VulkanMemoryAllocator/vk_mem_alloc.h"
#include "VulkanPlazaWrapper.h"
#include "Engine/Core/Debugging/Log.h"
#include <stb_image.h>

#define PLVK_CHECK_RESULT(x)                                                                                           \
	{                                                                                                                  \
		VkResult res = (x);                                                                                            \
		if (res != VK_SUCCESS) {                                                                                       \
			assert(x == VK_SUCCESS);                                                                                   \
		}                                                                                                              \
	}

namespace Plaza {
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct PLAZA_API VulkanTrackedImage : public TrackedImage {
	  public:
		VulkanTrackedImage(const std::string& newName, VkImage image, const TextureInfo& textureInfo,
						   VkSampler textureSampler, VkImageLayout layout)
			: mImage(image), mSampler(textureSampler), mLayout(layout), TrackedImage(newName, textureInfo) {}
		VulkanTrackedImage(){};
		VkImage mImage = VK_NULL_HANDLE;
		VkSampler mSampler = VK_NULL_HANDLE;
		VkImageLayout mLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
		VkImageView mImageView = VK_NULL_HANDLE;
	};

	class VulkanRenderGraph;
	class PLAZA_API VulkanRenderer : public Renderer {
	  public:
		VulkanRenderer(){};
		std::array<int, MAX_BONE_INFLUENCE> GetBoneIds(const std::vector<uint64_t>& bones);

		std::map<uint64_t, Bone> mBones = std::map<uint64_t, Bone>();
		VkSemaphore semaphore;

		bool mShowWireframe = false;

		struct PushConstants {
			glm::vec4 color = glm::vec4(1.0f);
			float intensity = 1.0f;
			int diffuseIndex = -1;
			int normalIndex = -1;
			int roughnessIndex = -1;
			int metalnessIndex = -1;
			float roughnessFloat = 0.5f;
			float metalnessFloat = 0.5f;
		};
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

		void Init() override;
		void InitializeRenderGraph(PlazaRenderGraph* renderGraph) override;
		void UpdateProjectManager() override;
		void Render(Scene* scene) override;
		void RenderImGuiFrame(std::vector<ImDrawData*> drawDatas) override;
		void RecordImGuiFrame(std::vector<ImDrawData*> drawDatas) override;
		void UpdateMainProgressBar(float percentage) override;
		void Destroy() override;
		void CopyLastFramebufferToFinalDrawBuffer() override;
		void InitGUI() override;
		void InitVulkanImGui();
		void NewFrameGUI() override;
		void UpdateGUI() override;
		void UpdateImGuiDisplayTexture(Texture* texture) override;
		ImTextureID GetFrameImage() override;

		void UploadBindlessTexture(VulkanTexture* texture, int index = -1);
		void UpdateInstancesData(Scene* scene);

		VkRenderPass CreateRenderPass(VkAttachmentDescription* attachmentDescs, uint32_t attachmentsCount,
									  VkSubpassDescription* subpasses, uint32_t subpassesCount,
									  VkSubpassDependency* dependencies, uint32_t dependenciesCount,
									  void* next = nullptr);
		VkFramebuffer CreateFramebuffer(VkRenderPass& renderPass, glm::vec2 size, VkImageView* pAttachmentsData,
										uint32_t attachmentsCount, uint32_t layers);

		void AddTrackerToImage(VkImage image, const std::string& name = "", VkSampler textureSampler = VK_NULL_HANDLE,
							   const TextureInfo& textureInfo = TextureInfo(),
							   VkImageLayout layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		ImTextureID GetTrackedImageID(TrackedImage* tracked) override;

		Mesh* CreateNewMesh(const std::vector<glm::vec3>& vertices, const std::vector<glm::vec3>& normals,
							const std::vector<glm::vec2>& uvs, const std::vector<glm::vec3>& tangent,
							const std::vector<unsigned int>& indices, const std::vector<unsigned int>& materialsIndices,
							bool usingNormal, const std::vector<BonesHolder>& bonesHolder = vector<BonesHolder>(),
							const std::vector<Bone>& uniqueBonesInfo = vector<Bone>()) override;
		void UpdateMeshVertices(Mesh& mesh);
		void DeleteMesh(Mesh& mesh) override;
		Mesh* RestartMesh(Mesh* mesh);
		void DrawRenderGroupInstanced(RenderGroup* renderGroup);
		void DrawRenderGroupShadowDepthMapInstanced(RenderGroup* renderGroup, unsigned int cascade);

		Texture* LoadTexture(std::string path, uint64_t uuid = 0) override;
		Texture* LoadImGuiTexture(std::string path) override;

		bool mFramebufferResized = false;
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkDeviceSize allocationSize = 0);
		VkFormat FindDepthFormat();

		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
						  VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
								   VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
								   unsigned int layerCount = 1, unsigned int mipCount = 1,
								   bool forceSynchronization = true, VkCommandBuffer commandBuffer = VK_NULL_HANDLE,
								   bool createOwnCommandPool = false);
		void TransitionTextureLayout(VulkanTexture& texture, VkImageLayout newLayout,
									 VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
									 unsigned int layerCount = 1, unsigned int mipCount = 1,
									 bool forceSynchronization = true);

		void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
						 VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image,
						 VkDeviceMemory& imageMemory);
		// VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
		VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
									VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D, unsigned int layerCount = 1,
									unsigned int mipCount = 1, unsigned int baseMipLevel = 0);

		VkCommandBuffer BeginSingleTimeCommands(bool createOwnCommandPool = false,
												VkCommandPool* commandPool = nullptr);
		void EndSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool commandPool = VK_NULL_HANDLE);
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t mipLevel = 0,
							   unsigned int arrayLayerCount = 1, bool createOwnCommandPool = false);
		VkCommandBuffer* mActiveCommandBuffer;

		VulkanRenderGraph* mRenderGraph = nullptr;

		static bool IsFormatDepth(VkFormat format);
		static bool IsFormatStencil(VkFormat format);
		static bool IsFormatDepthStencil(VkFormat format);
		static VkImageAspectFlags GetFormatAspectMask(VkFormat format);

		VkFormat mFinalDeferredFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
		VkFormat mSwapChainImageFormat;
		std::vector<VkDescriptorSet> mSwapchainDescriptorSets = std::vector<VkDescriptorSet>();
		VkImage mFinalSceneImage;
		VkImageView mFinalSceneImageView;
		VkImage mDeferredFinalImage;
		VkImageView mDeferredFinalImageView;

		VkPipelineCache mPipelineCache = VK_NULL_HANDLE;
		void LoadPipelineCache();
		void SavePipelineCache();
		const VkPipelineCache& GetPipelineCache() {
			return mPipelineCache;
		}

		VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
		VkFramebuffer mFinalSceneFramebuffer;
		VkFramebuffer mDeferredFramebuffer;
		VkExtent2D mSwapChainExtent;
		VkQueue mGraphicsQueue = VK_NULL_HANDLE;
		VkQueue mComputeQueue = VK_NULL_HANDLE;
		std::vector<VkFence> mInFlightFences;
		VkImageView mDepthImageView;

		void UpdateMaterials();

		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkDeviceSize offset = 0,
						bool createOwnCommandPool = false);
		void CopyImage(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkDeviceSize offset = 0);
		void CopyTexture(VulkanTexture* srcTexture, VkImageLayout srcLayout, VulkanTexture* dstTexture,
						 VkImageLayout dstLayout, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
		void CopyDifferentSizeTexture(VulkanTexture* srcTexture, VkImageLayout srcLayout, VulkanTexture* dstTexture,
						 VkImageLayout dstLayout, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
		VkSampler mImGuiTextureSampler;
		VkSampler mTextureSampler;
		std::vector<VkFence> mComputeInFlightFences;

		static VulkanRenderer* GetRenderer();

		std::vector<VkImage> mSwapChainImages;
		std::vector<VkImageView> mSwapChainImageViews;
		std::vector<VkFramebuffer> mSwapChainFramebuffers;

		RendererAPI api = RendererAPI::Vulkan;
		VkDevice mDevice = VK_NULL_HANDLE;
		VkDescriptorPool mDescriptorPool;
		VkRenderPass mRenderPass;
		VkRenderPass mDeferredRenderPass;
		VkRenderPass mSwapchainRenderPass;
		VulkanPlazaPipeline mSwapchainRenderer;

		VulkanPlazaPipeline mGeometryPassRenderer;

		VulkanTexture mDeferredPositionTexture;
		VulkanTexture mDeferredNormalTexture;
		VulkanTexture mDeferredDiffuseTexture;
		VulkanTexture mDeferredOthersTexture;

		VmaAllocator mVmaAllocator;

		VkDescriptorSet mFinalSceneDescriptorSet;
		VkPipelineLayout mPipelineLayout;
		VkPipeline mGraphicsPipeline;

		VkBuffer mMainIndirectCommandsBuffer = VK_NULL_HANDLE;
		VkDeviceMemory mMainIndirectCommandsBufferMemory = VK_NULL_HANDLE;
		PlVkBuffer* mMainVertexBuffer = new PlVkBuffer();
		PlVkBuffer* mSkinnedVertexBuffer = new PlVkBuffer();
		PlVkBuffer* mMainIndexBuffer = new PlVkBuffer();

		std::atomic<uint64_t> mBufferTotalVertices = 0;
		std::atomic<uint64_t> mBufferTotalIndices = 0;
		std::atomic<uint64_t> mIndirectDrawCount = 0;
		uint64_t mTotalInstances = 0;
		std::atomic<uint64_t> mTotalInstancesNewMesh = 0;
		std::vector<VkDrawIndexedIndirectCommand> mIndirectCommands = std::vector<VkDrawIndexedIndirectCommand>();

		std::vector<VkBuffer> mIndirectBuffers = std::vector<VkBuffer>();
		std::vector<VkDeviceMemory> mIndirectBufferMemories = std::vector<VkDeviceMemory>();

		std::vector<VkBuffer> mMainInstanceMatrixBuffers = std::vector<VkBuffer>();
		std::vector<VkDeviceMemory> mMainInstanceMatrixBufferMemories = std::vector<VkDeviceMemory>();

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

		VkInstance mVulkanInstance = VK_NULL_HANDLE;

		std::vector<glm::mat4> mInstanceModelMatrices = std::vector<glm::mat4>();

		void WaitRendererHere();

	  private:
		struct SwapChainPushConstant {
			float exposure = 1.2f;
			float gamma = 2.6f;
		};

		VkDescriptorSetLayout mSwapchainDescriptorSetLayout = VK_NULL_HANDLE;

		const std::string MODEL_PATH = "C:/Users/Giovane/Desktop/Workspace/viking_room.obj";
		const std::string TEXTURE_PATH = "C:/Users/Giovane/Desktop/Workspace/viking_room.png";

		uint32_t mCurrentImage;
		const int MAX_FRAMES_IN_FLIGHT = 2;

		bool mEnableValidationLayers = false;
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
											  const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
											  const VkAllocationCallbacks* pAllocator,
											  VkDebugUtilsMessengerEXT* pDebugMessenger);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
		VkExtent2D ChooseSwapExtent(VkSurfaceCapabilitiesKHR& capabilities);
		bool isDeviceSuitable(VkPhysicalDevice device);
		void InitVulkan();
		void SetupDebugMessenger();
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void InitSurface();
		void InitSwapChain();
		void CreateImageViews(VkImageLayout initialLayout);
		void InitCommands();
		void InitSyncStructures();
		void CreateRenderPass();
		void CreateSwapchainRenderPass();
		void CreateGraphicsPipeline();
		void CreateFramebuffers();
		void CreateCommandPool();
		void CreateCommandBuffers();
		void CreateImGuiTextureSampler();

		void UpdatePreRenderData(Scene* scene);
		void UpdatePreRecord();
		void UpdateAfterRecord();

		void CleanupSwapChain();
		void RecreateSwapChain();

		float time = 0.0f;
		bool increasing = true;
		void CalculateBonesParentship(Bone* bone, glm::mat4 parentTransform, float time, uint64_t boneId);
		void EarlyAnimationController();
		void CreateVertexBuffer(vector<Vertex> vertices, VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory,
								VkDeviceSize bufferSize = -1);
		void CreateIndexBuffer(vector<uint32_t> indices, VkBuffer& indicesBuffer, VkDeviceMemory& indicesMemoryBuffer,
							   VkDeviceSize bufferSize = -1);
		void CreateUniformBuffers();
		void CreateDescriptorPool();
		void CreateDescriptorSets();
		void CreateDescriptorSetLayout();
		void InitializeGeometryPassRenderer();

		void CreateTextureImage();
		VkDescriptorSet GetGeometryPassDescriptorSet(unsigned int frame);

		VkCommandBuffer CreateCommandBuffer();

		void CreateTextureImageView();
		void CreateTextureSampler();
		VkImage mTextureImage;
		VkDeviceMemory mTextureImageMemory;
		VkImageView mTextureImageView;

		VkDebugUtilsMessengerEXT mDebugMessenger = VK_NULL_HANDLE;
		VkQueue mPresentQueue = VK_NULL_HANDLE;
		VkSurfaceKHR mSurface = VK_NULL_HANDLE;
		VkSwapchainKHR mSwapChain;

		VkCommandPool mCommandPool;
		std::vector<VkCommandBuffer> mCommandBuffers;
		std::vector<VkCommandBuffer> mComputeCommandBuffers;

		std::vector<VkSemaphore> mImageAvailableSemaphores;
		std::vector<VkSemaphore> mRenderFinishedSemaphores;
		std::vector<VkSemaphore> mComputeFinishedSemaphores;

		void InitComputeCommandBuffers();
		void InitComputeSemaphores();
		void InitComputeInFlightFences();

		// VkBuffer mVertexBuffer;
		// VkDeviceMemory mVertexBufferMemory;
		// VkBuffer mIndexBuffer;
		// VkDeviceMemory mIndexBufferMemory;
		VkDescriptorSetLayout mDescriptorSetLayout;
		std::vector<VkDescriptorSet> mDescriptorSets;
		std::vector<VkDescriptorSet> mSwapPassDescriptorSets;

		std::vector<VkBuffer> mUniformBuffers;
		std::vector<VkDeviceMemory> mUniformBuffersMemory;
		std::vector<void*> mUniformBuffersMapped;

		VkDescriptorSet mMainSceneDescriptorSet;

		// VkBuffer mMainVertexBuffer = VK_NULL_HANDLE;
		// VkDeviceMemory mMainVertexBufferMemory = VK_NULL_HANDLE;
		// VkBuffer mMainIndexBuffer = VK_NULL_HANDLE;
		// VkDeviceMemory mMainIndexBufferMemory = VK_NULL_HANDLE;

		std::vector<VkBuffer> mMainInstanceMaterialBuffers = std::vector<VkBuffer>();
		std::vector<VkDeviceMemory> mMainInstanceMaterialBufferMemories = std::vector<VkDeviceMemory>();

		std::vector<VkBuffer> mMainInstanceMaterialOffsetsBuffers = std::vector<VkBuffer>();
		std::vector<VkDeviceMemory> mMainInstanceMaterialOffsetsBufferMemories = std::vector<VkDeviceMemory>();

		std::vector<VkBuffer> mMainInstanceRenderGroupOffsetsBuffers = std::vector<VkBuffer>();
		std::vector<VkDeviceMemory> mMainInstanceRenderGroupOffsetsBufferMemories = std::vector<VkDeviceMemory>();

		std::vector<VkBuffer> mBoneMatricesBuffers = std::vector<VkBuffer>();
		std::vector<VkDeviceMemory> mBoneMatricesBufferMemories = std::vector<VkDeviceMemory>();
		std::vector<std::vector<unsigned int>> mInstanceModelMaterialOffsets = std::vector<std::vector<unsigned int>>();
		std::vector<unsigned int> mInstanceModelMaterialsIndex = std::vector<unsigned int>();

		std::vector<MaterialData> mUploadedMaterials = std::vector<MaterialData>();
		std::unordered_map<uint64_t, unsigned int> mMaterialsHandles = std::unordered_map<uint64_t, unsigned int>();
		std::vector<VkBuffer> mMaterialBuffers = std::vector<VkBuffer>();
		std::vector<VkDeviceMemory> mMaterialBufferMemories = std::vector<VkDeviceMemory>();

		// ImGui variables
		VkDescriptorPool mImguiDescriptorPool;
		VkDescriptorSetLayout mImguiDescriptorSetLayout;
		VkDescriptorSet mImguiDescriptorSet;
		VkPipeline mImguiPipeline;
		VkPipelineLayout mImguiPipelineLayout;
		VkImage mImguiImage;

		VkImage mDepthImage;
		VkDeviceMemory mDepthImageMemory;
		void CreateDepthResources();
		VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
									 VkFormatFeatureFlags features);
		bool HasStencilComponent(VkFormat format);

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
															VkDebugUtilsMessageTypeFlagsEXT messageType,
															const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
															void* pUserData);

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		struct VertexMaterialsOffsets {
			std::vector<unsigned int> renderGroupOffsets = std::vector<unsigned int>();
			std::vector<unsigned int> renderGroupMaterialsOffsets = std::vector<unsigned int>();
		};

	  public:
		static std::array<VkVertexInputBindingDescription, 2> VertexGetBindingDescription() {
			std::array<VkVertexInputBindingDescription, 2> bindingDescriptions = {};
			bindingDescriptions[0].binding = 0;
			bindingDescriptions[0].stride = sizeof(Vertex);
			bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			VkVertexInputBindingDescription instanceBindingDescription = {};
			instanceBindingDescription.binding = 1;
			instanceBindingDescription.stride = sizeof(glm::vec4) * 4;
			instanceBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

			bindingDescriptions[1] = instanceBindingDescription;

			return bindingDescriptions;
		}

		static std::array<VkVertexInputAttributeDescription, 11> VertexGetAttributeDescriptions() {
			std::array<VkVertexInputAttributeDescription, 11> attributeDescriptions{};
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, position);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, normal);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(Vertex, texCoords);

			attributeDescriptions[3].binding = 0;
			attributeDescriptions[3].location = 3;
			attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[3].offset = offsetof(Vertex, tangent);

			attributeDescriptions[4].binding = 1;
			attributeDescriptions[4].location = 4;
			attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescriptions[4].offset = 0;

			attributeDescriptions[5].binding = 1;
			attributeDescriptions[5].location = 5;
			attributeDescriptions[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescriptions[5].offset = sizeof(float) * 4;

			attributeDescriptions[6].binding = 1;
			attributeDescriptions[6].location = 6;
			attributeDescriptions[6].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescriptions[6].offset = sizeof(float) * 8;

			attributeDescriptions[7].binding = 1;
			attributeDescriptions[7].location = 7;
			attributeDescriptions[7].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescriptions[7].offset = sizeof(float) * 12;

			// Material
			attributeDescriptions[8].binding = 0;
			attributeDescriptions[8].location = 8;
			attributeDescriptions[8].format = VK_FORMAT_R32_UINT;
			attributeDescriptions[8].offset = offsetof(Vertex, materialIndex);
			return attributeDescriptions;
		}

		friend class VulkanShadows;
		friend class VulkanSkybox;
		friend class VulkanMesh;
		friend class VulkanTexture;
		friend class VulkanGuiRenderer;
		friend class VulkanPicking;
	};
} // namespace Plaza
