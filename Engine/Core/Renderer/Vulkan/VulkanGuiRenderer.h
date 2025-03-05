#pragma once
#include <Engine/Core/Renderer/GuiRenderer.h>
#include "VulkanPlazaPipeline.h"

#include <sstream>
#include <iomanip>

#include "../stb_font_consolas_24_latin1.inl"
#define TEXTOVERLAY_MAX_CHAR_COUNT 2048

namespace Plaza {
	class VulkanRenderer;
	class PLAZA_API VulkanGuiRenderer : public GuiRenderer {
	  public:
		enum TextAlign {
			alignLeft,
			alignCenter,
			alignRight
		};

		void Init() override;
		void RenderText(Scene* scene, Drawing::UI::TextRenderer* textRendererComponent = nullptr) override;
		void Terminate() override;
		VkRenderPass mRenderPass;
		static void AddText(std::string text, float x, float y, float scale, TextAlign align, glm::vec4*& mapped,
							int& letters);
		stb_fontchar stbFontData[STB_FONT_consolas_24_latin1_NUM_CHARS];
		void FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, bool free);

	  private:
		void InitializeRenderPass();

		VkPipelineLayoutCreateInfo mPipelineLayoutInfo{};
		VulkanPlazaPipeline* mTextPipeline = nullptr;
		void PreparePipeline();
		VulkanRenderer* mRenderer;

		int numLetters = 0;

		VkSampler mSampler;
		VkImage mImage;
		VkImageView mImageView;
		VkDeviceMemory mImageMemory;
		// Character vertex buffer
		VkBuffer mBuffer;
		VkDeviceMemory mMemory;
		VkDescriptorPool mDescriptorPool;
		VkDescriptorSetLayout mDescriptorSetLayout;
		VkDescriptorSet mDescriptorSet;
		VkPipelineLayout mPipelineLayout;
		VkPipeline mPipeline;

		// Passed from the sample
		VkQueue mQueue;
		VkDevice* mDevice;
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		// Pointer to mapped vertex buffer
		glm::vec4* mappede = nullptr;
	};
} // namespace Plaza
