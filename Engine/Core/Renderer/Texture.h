#pragma once
#include <string>
#include "RendererTypes.h"
#include "Engine/Core/AssetsManager/Asset.h"
#include "Engine/Core/Engine.h"
#include <cstdint>
#include <type_traits>
#include <unordered_set>
#include <Engine/Core/UUID.h>
#include <cereal/types/base_class.hpp>
#include <fwd.hpp>
#include <vulkan_core.h>
#include <cereal/types/polymorphic.hpp>

namespace Plaza {
	struct PLAZA_API TextureInfo {
	  public:
		uint64_t mDescriptorCount = 1;
		PlTextureType mTextureType = PL_TYPE_2D;
		PlViewType mViewType = PL_VIEW_TYPE_2D;
		PlTextureFormat mFormat = PL_FORMAT_R8G8B8A8_UNORM; // Changing it to srgb might give better results
		PlImageUsage mImageUsage = PL_IMAGE_USAGE_COLOR_ATTACHMENT;
		uint16_t mLayersCount = 1;
		PlImageLayout mInitialLayout = PL_IMAGE_LAYOUT_UNDEFINED;
		std::string mPath = "";
		PlSamplerAddressMode mSamplerAddressMode = PL_SAMPLER_ADDRESS_MODE_REPEAT;
		bool mIsHdr = false;
		uint64_t mUuid = 0;

		bool operator==(const TextureInfo& other) const {
			return mDescriptorCount == other.mDescriptorCount && mTextureType == other.mTextureType &&
				   mViewType == other.mViewType && mFormat == other.mFormat && mImageUsage == other.mImageUsage &&
				   mLayersCount == other.mLayersCount && mInitialLayout == other.mInitialLayout &&
				   mPath == other.mPath && mSamplerAddressMode == other.mSamplerAddressMode && mIsHdr == other.mIsHdr;
		}

		template <class Archive> void serialize(Archive& archive) {
			archive(PL_SER(mUuid), PL_SER(mDescriptorCount), PL_SER(mTextureType), PL_SER(mViewType), PL_SER(mFormat),
					PL_SER(mImageUsage), PL_SER(mLayersCount), PL_SER(mInitialLayout), PL_SER(mPath),
					PL_SER(mSamplerAddressMode), PL_SER(mIsHdr));
		}
	};

	struct TextureInfoHash {
		std::size_t operator()(const TextureInfo& info) const {
			std::size_t hash = std::hash<uint64_t>{}(info.mDescriptorCount);
			hash ^= std::hash<int>{}(info.mTextureType) << 1;
			hash ^= std::hash<int>{}(info.mViewType) << 2;
			hash ^= std::hash<int>{}(info.mFormat) << 3;
			hash ^= std::hash<int>{}(info.mImageUsage) << 4;
			hash ^= std::hash<uint16_t>{}(info.mLayersCount) << 5;
			hash ^= std::hash<int>{}(info.mInitialLayout) << 6;
			hash ^= std::hash<std::string>{}(info.mPath) << 7;
			hash ^= std::hash<int>{}(info.mSamplerAddressMode) << 8;
			hash ^= std::hash<bool>{}(info.mIsHdr) << 9;
			return hash;
		}
	};

	class PLAZA_API Texture : public Asset {
	  public:
		glm::vec4 rgba = glm::vec4(1.0f);
		int mIndexHandle = -1;
		float mIntensity = 1.0f;
		glm::vec3 mResolution = glm::vec3(1, 1, 1);
		uint8_t mMipCount = 1;
		uint64_t mTextureInfoUuid = 0;

		Texture() { this->SetTextureInfo(TextureInfo{}); }
		Texture(glm::vec4 rgba, float intensity = 1.0f) {
			this->rgba = rgba;
			this->mIntensity = intensity;
			this->SetTextureInfo(TextureInfo{});
		}
		Texture(uint64_t descriptorCount, PlImageUsage imageUsage, PlTextureType imageType, PlViewType viewType,
				PlTextureFormat format, glm::vec3 resolution, uint8_t mipCount, uint16_t layersCount,
				const std::string& name) {
			this->SetTextureInfo(TextureInfo{descriptorCount, imageType, viewType, format, imageUsage, layersCount,
											 PL_IMAGE_LAYOUT_UNDEFINED, "", PL_SAMPLER_ADDRESS_MODE_REPEAT, false});
			mResolution = resolution;
			mMipCount = mipCount;
			mAssetName = name;
		}

		bool SameAs(Texture& other) { return (this->mAssetPath == other.mAssetPath && this->rgba == other.rgba); }

		virtual void Load(std::string relativePath = "") {}
		virtual VkDescriptorSet GetDescriptorSet() { return VkDescriptorSet(); }

		bool IsTextureEmpty() const { return false; }

		virtual unsigned int GetTextureID() { return 0; }

		virtual ImTextureID GetImGuiTextureID() { return 0; }

		virtual void Destroy() { return; }

		~Texture(){};

		template <class Archive> void serialize(Archive& archive) {
			// auto infoHash = TextureInfoHash(TextureInfo{});
			// if (mTextureInfo)
			//	infoHash = TextureInfoHash(mTextureInfo);
			archive(cereal::base_class<Asset>(this), PL_SER(rgba), PL_SER(mIntensity), PL_SER(mResolution),
					PL_SER(mMipCount), PL_SER(mTextureInfoUuid));
			// if (sTexturesInfo.find(infoHash) != sTexturesInfo.end()) {
			//	mTextureInfo = sTexturesInfo.at(infoHash);
			// }
		}

		void SetTextureInfo(TextureInfo textureInfo) {
			mTextureInfo = Texture::CreateTextureInfo(textureInfo);
			mTextureInfoUuid = mTextureInfo->mUuid;
		};
		const TextureInfo& GetTextureInfo() {
			if (mTextureInfo == nullptr)
				mTextureInfo = new TextureInfo();
			return *mTextureInfo;
		};

	  private:
		TextureInfo* mTextureInfo = nullptr;

	  public:
		/* Static */
		static TextureInfo* CreateTextureInfo(TextureInfo& info) {
			if (!Texture::HasTextureInfo(info)) {
				info.mUuid = Plaza::UUID::NewUUID();
				sTexturesInfo.emplace(info);
			}
			return Texture::GetTextureInfo(info);
		}
		static bool HasTextureInfo(const TextureInfo& info) { return sTexturesInfo.find(info) != sTexturesInfo.end(); }
		static TextureInfo* GetTextureInfo(const TextureInfo& info) {
			auto it = sTexturesInfo.find(info);
			if (it != sTexturesInfo.end()) {
				return const_cast<TextureInfo*>(&(*it));
			}
			return nullptr;
		}

	  private:
		static inline std::unordered_set<TextureInfo, TextureInfoHash> sTexturesInfo =
			std::unordered_set<TextureInfo, TextureInfoHash>();
	};
} // namespace Plaza
