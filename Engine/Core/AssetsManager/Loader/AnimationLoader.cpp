#include "Engine/Core/PreCompiledHeaders.h"
#include "AssetsLoader.h"
#include "Engine/Core/AssetsManager/Serializer/AssetsSerializer.h"

namespace Plaza {
	Animation& AssetsLoader::LoadAnimation(Asset* asset, SerializationMode serializationMode) {
		Animation animation =
			*AssetsSerializer::DeSerializeFile<Animation>(asset->mAssetPath.string(), serializationMode).get();
		return AssetsManager::AddAnimation(animation);
	}
} // namespace Plaza
