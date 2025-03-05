#pragma once
#include <Engine/Core/Renderer/Lighting.h>
#include "VulkanPlazaPipeline.h"
#include "VulkanComputeShaders.h"
#include "VulkanTexture.h"

namespace Plaza {
	class VulkanLighting : public Lighting {
	  public:
		void Init() override;
		void GetLights(Scene* scene) override;
		void UpdateTiles() override;
		void DrawDeferredPass();
		void Terminate() override;

		VulkanTexture mDeferredEndTexture;
		VulkanPlazaPipeline mDeferredEndPassRenderer;
		const VkFormat mDeferredEndTextureFormat = VK_FORMAT_R32G32B32A32_SFLOAT;

	  private:
		struct LightSorterPushConstants {
			glm::mat4 view;
			glm::mat4 projection;
			int lightCount;
			bool first;
			glm::vec2 screenSize;
			glm::vec2 clusterSize;
		};

		struct DeferredPassPushConstants {
			glm::vec3 viewPos;
			float time;
			glm::mat4 view;
			glm::mat4 projection;
			int lightCount;
			glm::vec4 ambientLightColor;
		};

		void InitializeDescriptorSets();
		void InitializeBuffers();

	  private:
		VulkanComputeShaders mLightSorterComputeShaders;
		VulkanShaders* mDeferredPass = nullptr;

		std::vector<VkBuffer> mLightsBuffer;
		std::vector<VkDeviceMemory> mLightsBufferMemory;
		std::vector<VkBuffer> mTilesBuffer;
		std::vector<VkDeviceMemory> mTilesBufferMemory;
		std::vector<VkBuffer> mDepthValuesBuffer;
		std::vector<VkDeviceMemory> mDepthValuesBufferMemory;
	};
} // namespace Plaza
