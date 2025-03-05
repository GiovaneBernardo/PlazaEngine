#pragma once
#include <string>
#include "Engine/stb_image.h"
#include "ThirdParty/cereal/cereal/types/polymorphic.hpp"
#include "Engine/Core/Renderer/Texture.h"
#include "Engine/Core/Engine.h"

namespace Plaza {
	class VulkanRenderer;
	class PLAZA_API VulkanTexture : public Texture {
	  public:
		VkDescriptorSet GetDescriptorSet() override;

		VulkanTexture(){};
		VulkanTexture(glm::vec4 rgba, float intensity = 1.0f) {
			this->rgba = rgba;
			this->mIntensity = intensity;
			this->SetTextureInfo(TextureInfo{});
		}
		VulkanTexture(uint64_t descriptorCount, PlImageUsage imageUsage, PlTextureType imageType, PlViewType viewType,
					  PlTextureFormat format, glm::vec3 resolution, uint8_t mipCount, uint16_t layersCount,
					  const std::string& name)
			: Texture(descriptorCount, imageUsage, imageType, viewType, format, resolution, mipCount, layersCount,
					  name) {}
		~VulkanTexture(){};

		VkDescriptorSet mDescriptorSet = VK_NULL_HANDLE;
		static inline std::atomic<int> mLastBindingIndex = 1;
		VkDescriptorSetLayout mDescriptorSetLayout = VK_NULL_HANDLE;
		VkImage mImage = VK_NULL_HANDLE;
		VkDeviceMemory mImageMemory = VK_NULL_HANDLE;
		VkImageView mImageView = VK_NULL_HANDLE;
		VkSampler mSampler = VK_NULL_HANDLE;

		VkImageView CreateImageView(VkFormat format, VkImageAspectFlags aspectFlags,
									VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D, unsigned int layerCount = 1,
									unsigned int baseMipLevel = 0);
		void CreateTextureSampler(VkSamplerAddressMode adressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT,
								  VkSamplerMipmapMode mipMapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
								  VkFilter magFilter = VK_FILTER_LINEAR, VkFilter minFilter = VK_FILTER_LINEAR,
								  VkBorderColor boderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
								  bool useAnisotropy = true);
		void GenerateMipmaps(VkImage image, int32_t texWidth, int32_t texHeight, uint32_t mipLevels, VkFormat format,
							 uint32_t layerCount = 1);
		bool CreateTextureImage(VkDevice& device, std::string path, VkFormat format, bool generateMipMaps = false,
								bool isHdr = false, VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM);
		bool CreateTextureImage(VkDevice device, VkFormat format, int width, int height, bool generateMipMaps = false,
								VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
								VkImageType imageType = VK_IMAGE_TYPE_2D,
								VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
								VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, unsigned int layers = 1,
								VkImageUsageFlags flags = 0, bool transition = true,
								VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE, bool calculateMips = false);
		void InitDescriptorSet();
		void InitDescriptorSetLayout();

		int32_t mWidth = 0;
		int32_t mHeight = 0;
		uint32_t CalculateMipLevels(int32_t width, int32_t height) {
			mMipCount = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
			return mMipCount;
		}

		VkFormat GetFormat();
		void SetFormat(PlTextureFormat newFormat);

		VkImageLayout GetLayout() { return this->mLayout; }
		VkImageLayout mLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		void Destroy() override {
			// TODO: IMPLEMENT TEXTURE DESTROY
		}

		glm::vec4 ReadTexture(glm::vec2 pos, unsigned int bytesPerPixel = 4, unsigned int channels = 4,
							  VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT, bool isDepth = false);

	  private:
		// VkFormat mFormat;
		VkBuffer mStagingBuffer = VK_NULL_HANDLE;
		VkDeviceMemory mStagingBufferMemory = VK_NULL_HANDLE;

		void Load(std::string path) override;
		unsigned int GetTextureID() override;
		ImTextureID GetImGuiTextureID() override;

		friend VulkanRenderer;
	};
} // namespace Plaza
