#include "Engine/Core/PreCompiledHeaders.h"
#include "Material.h"

namespace Plaza {
	Texture* GetTexture(uint64_t uuid) {
		if (AssetsManager::GetTexture(uuid))
			return AssetsManager::GetTexture(uuid);
		else
			return AssetsManager::GetTexture(1);
	}
	void Material::GetDeserializedTextures() {
		if (mDeserializedTexturesUuid[0]) *diffuse.get() = *GetTexture(mDeserializedTexturesUuid[0]);
 		if (mDeserializedTexturesUuid[1]) *normal.get() = *GetTexture(mDeserializedTexturesUuid[1]);
		if (mDeserializedTexturesUuid[2]) *metalness.get() = *GetTexture(mDeserializedTexturesUuid[2]);
		if (mDeserializedTexturesUuid[3]) *roughness.get() = *GetTexture(mDeserializedTexturesUuid[3]);
		if (mDeserializedTexturesUuid[4]) *height.get() = *GetTexture(mDeserializedTexturesUuid[4]);
		if (mDeserializedTexturesUuid[5]) *aoMap.get() = *GetTexture(mDeserializedTexturesUuid[5]);
	}
}

