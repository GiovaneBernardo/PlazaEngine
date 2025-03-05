#include "AssetsSerializer.h"

namespace Plaza {
	void AssetsSerializer::SerializeAnimation(Animation& animation, std::filesystem::path outPath,
											  SerializationMode serializationMode) {
		AssetsSerializer::SerializeFile<Animation>(animation, outPath.string(), serializationMode);
	}
} // namespace Plaza
