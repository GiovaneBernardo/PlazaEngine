#pragma once
#include "Engine/Core/AssetsManager/Importer/AssetsImporter.h"

namespace Plaza {
	class Metadata {
	  public:
		struct MetadataStructure : public Asset {
			AssetExtension mExtension;
			std::string mContentName;

			template <class Archive> void serialize(Archive& archive) {
				archive(PL_SER(mAssetUuid), PL_SER(mAssetName), PL_SER(mExtension), PL_SER(mContentName));
			}
		};

		static void CreateMetadataFile(Asset* asset);
		static Asset ConvertMetadataToAsset(MetadataStructure metadata);
	};
} // namespace Plaza
