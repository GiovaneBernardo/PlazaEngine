#include "AssetsSerializer.h"

namespace Plaza {
	void AssetsSerializer::SerializeMesh(Mesh* mesh, std::filesystem::path outPath,
										 SerializationMode serializationMode) {
		AssetsSerializer::SerializeFile<Mesh>(*mesh, outPath.string(), serializationMode);
	}
} // namespace Plaza
