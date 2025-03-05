#include "Engine/Core/PreCompiledHeaders.h"
#include "Metadata.h"
#include "Engine/Core/AssetsManager/Serializer/AssetsSerializer.h"

namespace Plaza {
	void Metadata::CreateMetadataFile(Asset* asset) {
		MetadataStructure metadata;
		metadata.mAssetUuid = asset->mAssetUuid;
		metadata.mAssetName = asset->mAssetName + Standards::metadataExtName;
		metadata.mAssetPath = asset->mAssetPath;
		metadata.mContentName = asset->mAssetName;
		std::filesystem::path metadataPath = asset->mAssetPath;
		metadataPath.replace_extension(Standards::metadataExtName).string();
		AssetsSerializer::SerializeFile<MetadataStructure>(metadata, metadataPath.string(),
														   Application::Get()->mSettings.mMetaDataSerializationMode);
	}

	Asset Metadata::ConvertMetadataToAsset(MetadataStructure metadata) {
		std::string contentPath = metadata.mAssetPath.parent_path().string() + "/" + metadata.mContentName;
		Asset asset(metadata.mAssetUuid, metadata.mAssetPath.filename().string(), contentPath);
		asset.mAssetName = metadata.mContentName;
		return asset;
	}
} // namespace Plaza
