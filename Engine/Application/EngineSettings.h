#pragma once
#include "Engine/Core/Engine.h"
#include "Engine/Core/AssetsManager/Asset.h"

namespace Plaza {
	enum class SerializationMode {
		SERIALIZE_BINARY,
		SERIALIZE_JSON,
		SERIALIZE_YAML
	};
	class EngineSettings : public Asset {
	  public:
		SerializationMode mCommonSerializationMode = SerializationMode::SERIALIZE_JSON;
		SerializationMode mMetaDataSerializationMode = SerializationMode::SERIALIZE_JSON;
		SerializationMode mSceneSerializationMode = SerializationMode::SERIALIZE_JSON;
		SerializationMode mProjectSerializationMode = SerializationMode::SERIALIZE_JSON;
		SerializationMode mSettingsSerializationMode = SerializationMode::SERIALIZE_JSON;
		SerializationMode mModelSerializationMode = SerializationMode::SERIALIZE_BINARY;
		SerializationMode mPrefabSerializationMode = SerializationMode::SERIALIZE_JSON;
		SerializationMode mAnimationSerializationMode = SerializationMode::SERIALIZE_BINARY;
		SerializationMode mMaterialSerializationMode = SerializationMode::SERIALIZE_JSON;
		SerializationMode mRenderGraphSerializationMode = SerializationMode::SERIALIZE_JSON;

		bool mVsync = false;

		template <class Archive> void serialize(Archive& archive) {
			archive(PL_SER(mCommonSerializationMode), PL_SER(mMetaDataSerializationMode),
					PL_SER(mSceneSerializationMode), PL_SER(mSceneSerializationMode), PL_SER(mProjectSerializationMode),
					PL_SER(mSettingsSerializationMode), PL_SER(mModelSerializationMode),
					PL_SER(mAnimationSerializationMode), PL_SER(mMaterialSerializationMode),
					PL_SER(mRenderGraphSerializationMode), PL_SER(mVsync));
		}
	};
} // namespace Plaza
