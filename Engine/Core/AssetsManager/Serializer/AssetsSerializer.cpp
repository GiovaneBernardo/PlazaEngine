#include "Engine/Core/PreCompiledHeaders.h"
#include "AssetsSerializer.h"
#include "Engine/Core/AssetsManager/Metadata/Metadata.h"
#include "Engine/Core/Scene.h"
#include "Engine/Core/Engine.h"
#include <cereal/types/polymorphic.hpp>
#include "Engine/Components/Rendering/AnimationComponent.h"

namespace Plaza {
	void AssetsSerializer::SerializeAssetByExtension(Asset* asset) {
		if (asset->GetExtension() == Standards::metadataExtName)
			AssetsSerializer::SerializeFile<Metadata::MetadataStructure>(*static_cast<Metadata::MetadataStructure*>(asset), asset->mAssetPath.string(), Application::Get()->mSettings.mMetaDataSerializationMode);
		//else
		//	AssetsManager::NewAsset<Asset>(asset->mAssetUuid, asset->mAssetPath.string());

		if (asset->GetExtension() == Standards::materialExtName)
			AssetsSerializer::SerializeFile<Material>(*static_cast<Material*>(AssetsManager::GetMaterial(asset->mAssetUuid)), asset->mAssetPath.string(), Application::Get()->mSettings.mMaterialSerializationMode);
		else if (asset->GetExtension() == Standards::animationExtName)
			AssetsSerializer::SerializeFile<Animation>(*static_cast<Animation*>(AssetsManager::GetAnimation(asset->mAssetUuid)), asset->mAssetPath.string(), Application::Get()->mSettings.mAnimationSerializationMode);
		else if (asset->GetExtension() == Standards::sceneExtName)
			AssetsSerializer::SerializeFile<Scene>(*static_cast<Scene*>(asset), asset->mAssetPath.string(), Application::Get()->mSettings.mSceneSerializationMode);

	}
}

CEREAL_REGISTER_TYPE(Plaza::Texture);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Plaza::Asset, Plaza::Texture);
CEREAL_REGISTER_TYPE(Plaza::VulkanTexture);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Plaza::Texture, Plaza::VulkanTexture);
CEREAL_REGISTER_TYPE(Plaza::GuiRectangle);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Plaza::GuiItem, Plaza::GuiRectangle);
CEREAL_REGISTER_TYPE(Plaza::GuiButton);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Plaza::GuiItem, Plaza::GuiButton);