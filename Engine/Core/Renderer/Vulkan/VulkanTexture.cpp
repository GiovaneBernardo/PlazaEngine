#include "Engine/Core/PreCompiledHeaders.h"
#include "VulkanTexture.h"
#include "Renderer.h"
#include "ThirdParty/dds_image/dds.hpp"

namespace Plaza {
	/// Takes image with VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL and outputs it with VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	void VulkanTexture::GenerateMipmaps(VkImage image, int32_t texWidth, int32_t texHeight, uint32_t mipLevels, VkFormat format, uint32_t layerCount) {
		// Check if image format supports linear blitting
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(VulkanRenderer::GetRenderer()->mPhysicalDevice, format, &formatProperties);

		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
			throw std::runtime_error("texture image format does not support linear blitting!");
		}

		VkCommandBuffer commandBuffer = VulkanRenderer::GetRenderer()->BeginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = layerCount;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = texWidth;
		int32_t mipHeight = texHeight;

		for (uint32_t i = 1; i < mipLevels; i++) {
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			VkImageBlit blit{};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = layerCount;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = layerCount;

			vkCmdBlitImage(commandBuffer,
				image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		this->mLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		VulkanRenderer::GetRenderer()->EndSingleTimeCommands(commandBuffer);
	}

	void FlipYCoordinates(stbi_uc* imageData, int width, int height, int channels) {
		int rowSize = width * channels;
		stbi_uc* tempRow = new stbi_uc[rowSize];

		for (int i = 0; i < height / 2; ++i) {
			stbi_uc* topRow = imageData + i * rowSize;
			stbi_uc* bottomRow = imageData + (height - i - 1) * rowSize;
			// Swap pixel data row by row
			for (int j = 0; j < rowSize; ++j) {
				stbi_uc temp = topRow[j];
				topRow[j] = bottomRow[j];
				bottomRow[j] = temp;
			}
		}

		delete[] tempRow;
	}

	std::vector<unsigned char> read_file(const std::string& filename) {
		std::ifstream file(filename, std::ios::binary | std::ios::ate);
		if (!file.is_open()) {
			throw std::runtime_error("Failed to open file: " + filename);
		}
		
		std::streamsize file_size = file.tellg();
		if (file_size <= 0) {
			throw std::runtime_error("File is empty or size could not be determined: " + filename);
		}
		
		std::vector<unsigned char> buffer(static_cast<size_t>(file_size));
		file.seekg(0, std::ios::beg);
		file.read(reinterpret_cast<char*>(buffer.data()), file_size);
		
		if (!file) {
			throw std::runtime_error("Failed to read file: " + filename);
		}
	
		return buffer;
	}

	bool VulkanTexture::CreateTextureImage(VkDevice& device, std::string path, VkFormat format, bool generateMipMaps, bool isHdr, VkImageUsageFlags imageUsage) {
		//this->SetFormat(format);

		bool isDDS = std::filesystem::path{ path }.extension() == ".dds";
		dds::Image image;

		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = nullptr;
		float* pixelsFloat = nullptr;
		uint32_t pix = 0;
		VkFormat imageFormat = format;

		VkDeviceSize imageSize;
		auto datae = read_file(path.c_str());
		if (!isDDS) {
			if (isHdr)
				pixelsFloat = stbi_loadf(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
			else{
				pixels = stbi_load_from_memory(datae.data(), datae.size(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
				for (size_t i = 0; i < datae.size() && i < 100; ++i) {
					std::cout << std::hex << static_cast<int>(datae[i]) << " ";
				}
				std::cout << std::endl;
			}
			imageSize = texWidth * texHeight * 4;
			if (isHdr)
				imageSize *= 4;
		}
		else {
			generateMipMaps = false;
			dds::ReadResult res = dds::readFile(path, &image);
			texWidth = image.width;
			texHeight = image.height;
			VkImageViewCreateInfo asd = dds::getVulkanImageViewCreateInfo(&image);
			VkImageCreateInfo asd2 = dds::getVulkanImageCreateInfo(&image);
			pix = dds::getBitsPerPixel(image.format);
			pix = dds::getBlockSize(image.format);
			imageFormat = dds::getVulkanFormat(image.format, false);
			format = imageFormat;
			texChannels = 4;

			// Convert std::string to std::wstring
			std::wstring widestr = std::wstring(path.begin(), path.end());
			const wchar_t* widecstr = widestr.c_str();
			//DirectX::LoadFromDDSFile(widecstr, DirectX::DDS_FLAGS_FORCE_RGB, &metadata, scratchImage);

			//texWidth = metadata.width;
			//texHeight = metadata.height;
			//texChannels = 4;

			// Access image data			
			//const DirectX::Image* image2 = scratchImage.GetImage(0, 0, 0);
			pixels = static_cast<stbi_uc*>(image.data.data());
			//FlipYCoordinates(pixels, texWidth, texHeight, 3);

			imageSize = image.data.size();
		}

		this->SetFormat(VkFormatToPlImageFormat(format));

		this->mWidth = texWidth;
		this->mHeight = texHeight;

		this->mMipCount = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
		if (!generateMipMaps)
			this->mMipCount = 1;

		if (!isDDS && !pixels && !pixelsFloat) {
			PL_CORE_CRITICAL("Failed to load image", __FILE__, __LINE__);
			PL_CORE_CRITICAL("Path: " + path);
			throw std::runtime_error("failed to load texture image!");
		}
		VulkanRenderer::GetRenderer()->CreateBuffer(format == VK_FORMAT_R32G32B32A32_SFLOAT ? imageSize * 4 : imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mStagingBuffer, mStagingBufferMemory);

		void* data;
		vkMapMemory(device, mStagingBufferMemory, 0, imageSize, 0, &data);
		if (isHdr)
			memcpy(data, pixelsFloat, static_cast<size_t>(imageSize));
		else
			memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(device, mStagingBufferMemory);

		if (!isDDS && pixels)
			stbi_image_free(pixels);
		else if (!isDDS && pixelsFloat)
			stbi_image_free(pixelsFloat);
		else
			image.data.clear();


		//VulkanRenderer::GetRenderer()->CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mTextureImage, mTextureImageMemory);
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = texWidth;
		imageInfo.extent.height = texHeight;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = this->mMipCount;
		imageInfo.arrayLayers = 1;
		imageInfo.format = imageFormat;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = imageUsage == VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM ? VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT : imageUsage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;

		if (vkCreateImage(device, &imageInfo, nullptr, &mImage) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, mImage, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = VulkanRenderer::GetRenderer()->FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkResult result = vkAllocateMemory(device, &allocInfo, nullptr, &mImageMemory);
		if (result != VK_SUCCESS) {
			VkPhysicalDeviceMemoryProperties memoryProperties2;
			vkGetPhysicalDeviceMemoryProperties(VulkanRenderer::GetRenderer()->mPhysicalDevice, &memoryProperties2);
			std::cout << result << "\n";
			vkDestroyBuffer(device, mStagingBuffer, nullptr);
			vkFreeMemory(device, mStagingBufferMemory, nullptr);
			return false;
			//throw std::runtime_error("failed to allocate image memory!");
		}
		else {

			vkBindImageMemory(device, mImage, mImageMemory, 0);

			VulkanRenderer::GetRenderer()->TransitionImageLayout(mImage, imageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1U, 1, this->mMipCount, true, VK_NULL_HANDLE, true);
			VulkanRenderer::GetRenderer()->CopyBufferToImage(mStagingBuffer, mImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 0, 1, true);
			//TODO: FIX VALIDATION ERROR WHEN GENERATING MIP MAP
			if (generateMipMaps)
				GenerateMipmaps(this->mImage, texWidth, texHeight, this->mMipCount, imageFormat, 1);
			else
				VulkanRenderer::GetRenderer()->TransitionImageLayout(mImage, imageFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1U, 1, this->mMipCount, true, VK_NULL_HANDLE, true);
			mLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			vkDestroyBuffer(device, mStagingBuffer, nullptr);
			vkFreeMemory(device, mStagingBufferMemory, nullptr);
		}
		return true;
	}

	bool VulkanTexture::CreateTextureImage(VkDevice device, VkFormat format, int width, int height, bool generateMipMaps, VkImageUsageFlags usageFlags, VkImageType imageType, VkImageTiling tiling, VkImageLayout initialLayout,
		unsigned int layers, VkImageUsageFlags flags, bool transition, VkSharingMode sharingMode, bool calculateMips) {
		//		mLayersCount = layers;
				//this->SetFormat(format);
		this->mMipCount = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
		if (!generateMipMaps && !calculateMips)
			this->mMipCount = 1;

		this->mWidth = width;
		this->mHeight = height;

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = imageType;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = this->mMipCount;
		imageInfo.arrayLayers = layers;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = initialLayout;
		imageInfo.usage = usageFlags; //VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = sharingMode;
		imageInfo.flags = flags;

		if (vkCreateImage(device, &imageInfo, nullptr, &mImage) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, mImage, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = VulkanRenderer::GetRenderer()->FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &mImageMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}
		vkBindImageMemory(device, mImage, mImageMemory, 0);

		if (transition) {

			VulkanRenderer::GetRenderer()->TransitionImageLayout(mImage, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, layers, this->mMipCount);
			mLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		}

		if (generateMipMaps) {
			GenerateMipmaps(this->mImage, width, height, this->mMipCount, format, layers);
			mLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
		else if (transition) {
			VulkanRenderer::GetRenderer()->TransitionImageLayout(mImage, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, layers, this->mMipCount);
			mLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}

		//vkDestroyBuffer(device, mStagingBuffer, nullptr);
		//vkFreeMemory(device, mStagingBufferMemory, nullptr);
		return true;
	}

	void VulkanTexture::Load(std::string path) {

	}

	unsigned int VulkanTexture::GetTextureID() {
		return static_cast<unsigned int>(reinterpret_cast<uintptr_t>(this->mDescriptorSet));
	}

	ImTextureID VulkanTexture::GetImGuiTextureID() {
		ImTextureID id = static_cast<ImTextureID>(this->mDescriptorSet);
		return id;
	}

	VkImageView VulkanTexture::CreateImageView(VkFormat format, VkImageAspectFlags aspectFlags, VkImageViewType viewType, unsigned int layerCount, unsigned int baseMipLevel) {
		if (mImageView != VK_NULL_HANDLE)
			vkDestroyImageView(VulkanRenderer::GetRenderer()->mDevice, mImageView, nullptr);
		mImageView = VulkanRenderer::GetRenderer()->CreateImageView(mImage, format, aspectFlags, viewType, layerCount, mMipCount, baseMipLevel);
		return mImageView;
		//VkImageViewCreateInfo viewInfo{};
		//viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		//viewInfo.image = this->mImage;
		//viewInfo.viewType = viewType;
		//viewInfo.format = format;
		//viewInfo.subresourceRange.baseMipLevel = baseMipLevel;
		//viewInfo.subresourceRange.levelCount = this->mMipCount;
		//viewInfo.subresourceRange.baseArrayLayer = 0;
		//viewInfo.subresourceRange.layerCount = layerCount;
		//viewInfo.subresourceRange.aspectMask = aspectFlags;
		//
		//if (vkCreateImageView(VulkanRenderer::GetRenderer()->mDevice, &viewInfo, nullptr, &mImageView) != VK_SUCCESS) {
		//	throw std::runtime_error("failed to create texture image view!");
		//}
		//
		//return mImageView;
	}

	VkDescriptorSet VulkanTexture::GetDescriptorSet() {
		return this->mDescriptorSet;
	}

#define ArraySize(a) \
  ((sizeof(a) / sizeof(*(a))) / \
  static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))

	void VulkanTexture::InitDescriptorSet() {
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = mImageView;
		imageInfo.sampler = VulkanRenderer::GetRenderer()->mTextureSampler;

		for (size_t i = 0; i < Application::Get()->mRenderer->mMaxFramesInFlight; i++) {
			std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

			VkDescriptorBufferInfo bufferInfo2{};
			bufferInfo2.buffer = VulkanRenderer::GetRenderer()->mUniformBuffers[i];
			bufferInfo2.offset = 0;
			bufferInfo2.range = sizeof(VulkanRenderer::UniformBufferObject);

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = VulkanRenderer::GetRenderer()->GetGeometryPassDescriptorSet(i);// VulkanRenderer::GetRenderer()->mDescriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo2;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = VulkanRenderer::GetRenderer()->GetGeometryPassDescriptorSet(i);
			descriptorWrites[1].dstBinding = 20;
			descriptorWrites[1].dstArrayElement = VulkanTexture::mLastBindingIndex;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &imageInfo;
			this->mIndexHandle = mLastBindingIndex;

			vkUpdateDescriptorSets(VulkanRenderer::GetRenderer()->mDevice, 2, descriptorWrites.data(), 0, nullptr);
		}
		VulkanTexture::mLastBindingIndex++;
	}

	void VulkanTexture::InitDescriptorSetLayout() {
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = &mSampler; // Optional

		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 0;
		samplerLayoutBinding.descriptorCount = 16536;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		samplerLayoutBinding.pImmutableSamplers = &mSampler;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL;


		std::array<VkDescriptorSetLayoutBinding, 1> bindings = { samplerLayoutBinding };
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(VulkanRenderer::GetRenderer()->mDevice, &layoutInfo, nullptr, &mDescriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}

		InitDescriptorSet();
	}

	void VulkanTexture::CreateTextureSampler(VkSamplerAddressMode adressMode, VkSamplerMipmapMode mipMapMode, VkFilter magFilter, VkFilter minFilter, VkBorderColor borderColor, bool useAnisotropy) {
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = magFilter;
		samplerInfo.minFilter = minFilter;
		samplerInfo.addressModeU = adressMode;
		samplerInfo.addressModeV = adressMode;
		samplerInfo.addressModeW = adressMode;
		samplerInfo.anisotropyEnable = VK_TRUE;
		//samplerInfo.maxAnisotropy = 0;

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(VulkanRenderer::GetRenderer()->mPhysicalDevice, &properties);
		samplerInfo.maxAnisotropy = useAnisotropy ? properties.limits.maxSamplerAnisotropy : 1.0f;
		samplerInfo.borderColor = borderColor;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = mipMapMode;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = static_cast<float>(this->mMipCount);

		if (vkCreateSampler(VulkanRenderer::GetRenderer()->mDevice, &samplerInfo, nullptr, &mSampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}
	}

	VkFormat VulkanTexture::GetFormat() {
		return PlImageFormatToVkFormat(this->GetTextureInfo().mFormat);
	}

	void VulkanTexture::SetFormat(PlTextureFormat newFormat) {
		TextureInfo info = this->GetTextureInfo();
		info.mFormat = newFormat;
		this->SetTextureInfo(info);
	}

	glm::vec4 VulkanTexture::ReadTexture(glm::vec2 pos, unsigned int bytesPerPixel, unsigned int channels, VkImageAspectFlags aspect, bool isDepth) {
		if (pos.x >= mWidth || pos.x < 0 || pos.y >= mHeight || pos.y < 0)
			return glm::vec4(0.0f);

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		VulkanRenderer::GetRenderer()->CreateBuffer(mWidth * mHeight * bytesPerPixel,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory);

		VkImageLayout oldLayout = this->GetLayout();
		VulkanRenderer::GetRenderer()->TransitionTextureLayout(*this, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, aspect);

		VkCommandBuffer commandBuffer = VulkanRenderer::GetRenderer()->BeginSingleTimeCommands();
		VkImageSubresourceLayers subResource = {};
		subResource.aspectMask = aspect & VK_IMAGE_ASPECT_DEPTH_BIT ? VK_IMAGE_ASPECT_DEPTH_BIT : aspect;
		subResource.mipLevel = 0;
		subResource.baseArrayLayer = 0;
		subResource.layerCount = 1;

		VkBufferImageCopy region = {};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource = subResource;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { (unsigned int)mWidth, (unsigned int)mHeight, 1 };

		vkCmdCopyImageToBuffer(commandBuffer, mImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingBuffer, 1, &region);
		VulkanRenderer::GetRenderer()->EndSingleTimeCommands(commandBuffer);

		void* data;
		int64_t size = mWidth * mHeight * bytesPerPixel;
		vkMapMemory(VulkanRenderer::GetRenderer()->mDevice, stagingBufferMemory, 0, size, 0, &data);

		float* pixelData = static_cast<float*>(data);
		int desiredPixelX = pos.x;
		int desiredPixelY = mHeight - pos.y;
		int byteOffset = (desiredPixelY * mWidth + desiredPixelX) * bytesPerPixel;

		float r = 0.0f, g = 0.0f, b = 0.0f, a = 0.0f;
		if (isDepth) {
			float* depthValues = reinterpret_cast<float*>(data);
			uint8_t* stencilValues = reinterpret_cast<uint8_t*>(static_cast<char*>(data) + (mWidth * mHeight * sizeof(float)));

			uint32_t index = pos.y * mWidth + pos.x;
			r = depthValues[index];
		}
		else {
			if (byteOffset < size) {
				if (channels >= 1) r = pixelData[byteOffset];
				if (channels >= 2) g = pixelData[byteOffset + 1];
				if (channels >= 3) b = pixelData[byteOffset + 2];
				if (channels >= 4) a = pixelData[byteOffset + 3];
			}
		}

		vkUnmapMemory(VulkanRenderer::GetRenderer()->mDevice, stagingBufferMemory);
		vkDestroyBuffer(VulkanRenderer::GetRenderer()->mDevice, stagingBuffer, nullptr);
		vkFreeMemory(VulkanRenderer::GetRenderer()->mDevice, stagingBufferMemory, nullptr);

		VulkanRenderer::GetRenderer()->TransitionTextureLayout(*this, oldLayout, aspect);

		return glm::vec4(r, g, b, a);
	}
}