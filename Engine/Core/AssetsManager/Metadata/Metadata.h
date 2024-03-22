#pragma once
#include "Engine/Core/AssetsManager/Importer/AssetsImporter.h"

namespace Plaza {
	class Metadata {
	public:
		struct MetadataStructure {
			uint64_t uuid;
			AssetExtension mExtension;
			std::string mPath;
		};

		static void CreateMetadataFile(Asset* asset) {
			Metadata::SerializeMetadata(Plaza::UUID::NewUUID(), asset->mPath.string(), asset->mAssetExtension);
		}

		static void SerializeMetadata(uint64_t uuid, std::string assetPath, std::string extension) {
			std::filesystem::path path(assetPath);
			std::string outName = path.parent_path().string() + "\\" + path.filename().string() + Standards::metadataExtName;
			std::ofstream binaryFile(outName, std::ios::binary);

			binaryFile.write(reinterpret_cast<const char*>(&uuid), sizeof(uuid));
			Plaza::Utils::SaveStringAsBinary(binaryFile, assetPath);
			Plaza::Utils::SaveStringAsBinary(binaryFile, extension);

			binaryFile.close();
		}

		static Asset DeSerializeMetadata(std::string metadaPath) {
			std::ifstream binaryFile(metadaPath, std::ios::binary);
			Asset asset = Asset();

			binaryFile.read(reinterpret_cast<char*>(&asset.mAssetUuid), sizeof(uint64_t));
			asset.mPath = Plaza::Utils::ReadBinaryString(binaryFile);
			asset.mAssetExtension = Plaza::Utils::ReadBinaryString(binaryFile);

			binaryFile.close();

			return asset;
		}
	};
}