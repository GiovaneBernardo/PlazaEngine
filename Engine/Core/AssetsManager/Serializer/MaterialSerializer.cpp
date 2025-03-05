#include "AssetsSerializer.h"

namespace Plaza {
	void AssetsSerializer::SerializeMaterial(Material* material, std::filesystem::path outPath,
											 SerializationMode serializationMode) {
		AssetsSerializer::SerializeFile<Material>(*material, outPath.string(), serializationMode);
	}
} // namespace Plaza
