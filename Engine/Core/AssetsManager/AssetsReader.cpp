#include "Engine/Core/PreCompiledHeaders.h"
#include "AssetsReader.h"
#include "Engine/Core/AssetsManager/Loader/AssetsLoader.h"
#include "Engine/Core/AssetsManager/Serializer/AssetsSerializer.h"

namespace Plaza {
	bool IsJsonFile(const std::string& path) {
		std::ifstream file(path, std::ios::binary);
		char buffer[256];
		file.read(buffer, sizeof(buffer));

		// Check for non-printable characters
		for (size_t i = 0; i < file.gcount(); ++i) {
			if (buffer[i] < 32 && buffer[i] != '\n' && buffer[i] != '\r' && buffer[i] != '\t') {
				return false;
			}
		}
		return true;
	}
	Asset* AssetsReader::ReadAssetAtPath(std::filesystem::path path) {
		SectionProfiler section(path.string());
		std::string extension = path.extension().string();
		if (AssetsLoader::mSupportedLoadFormats.find(extension) == AssetsLoader::mSupportedLoadFormats.end())
			return nullptr;
		shared_ptr<Asset> asset = AssetsSerializer::DeSerializeFile<Asset>(
			path.string(),
			IsJsonFile(path.string()) ? SerializationMode::SERIALIZE_JSON : SerializationMode::SERIALIZE_BINARY);
		AssetsLoader::LoadAsset(asset.get());
		section.Stop();
		Profiler::GetProfiler("ReadGameAssets")->AddSection(section);
		return AssetsManager::GetAsset(asset->mAssetUuid);
	}
} // namespace Plaza
