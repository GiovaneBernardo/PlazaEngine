#pragma once
#include "Engine/Components/Component.h"
#include "Engine/Core/AssetsManager/Asset.h"
#include "Engine/Core/Engine.h"

namespace Plaza {
	class PLAZA_API PhysicsMaterial : public Asset {
	  public:
		float mStaticFriction = 0.5f;
		float mDynamicFriction = 0.5f;
		float mRestitution = 0.5f;
		physx::PxMaterial* mPhysxMaterial = nullptr;

		PhysicsMaterial() { this->mAssetUuid = Plaza::UUID::NewUUID(); }

		PhysicsMaterial(float staticFriction, float dynamicFriction, float restitution)
			: mStaticFriction(staticFriction), mDynamicFriction(dynamicFriction), mRestitution(restitution) {
			this->mAssetUuid = Plaza::UUID::NewUUID();
		}

		bool operator==(const PhysicsMaterial& other) const {
			return mStaticFriction == other.mStaticFriction && mDynamicFriction == other.mDynamicFriction &&
				   mRestitution == other.mRestitution;
		}

		template <class Archive> void serialize(Archive& archive) {
			archive(PL_SER(mAssetUuid), PL_SER(mAssetName), PL_SER(mStaticFriction), PL_SER(mDynamicFriction),
					PL_SER(mRestitution));
		}
	};
} // namespace Plaza

namespace std {
	template <> struct PLAZA_API hash<Plaza::PhysicsMaterial> {
		std::size_t operator()(const Plaza::PhysicsMaterial& material) const {
			std::size_t hashValue = 0;
			std::hash<float> floatHasher;

			hashValue ^= floatHasher(material.mStaticFriction) + 0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);
			hashValue ^= floatHasher(material.mDynamicFriction) + 0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);
			hashValue ^= floatHasher(material.mRestitution) + 0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);
			return hashValue;
		}
	};
} // namespace std
