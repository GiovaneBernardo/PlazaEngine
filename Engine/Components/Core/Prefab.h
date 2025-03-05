#pragma once
#include "Engine/Core/AssetsManager/Asset.h"
#include "Engine/ECS/ECSManager.h"
namespace Plaza {
	class PLAZA_API Prefab : public Asset {
	  public:
		std::string mPrefabName = "";
		uint64_t mMainEntityUuid = 0;
		std::unordered_map<uint64_t, Entity> mEntities = std::unordered_map<uint64_t, Entity>();
		std::vector<ComponentPool*> mComponentPools = std::vector<ComponentPool*>();
		std::set<uint64_t> mMeshesUuid = std::set<uint64_t>();

		Prefab(){};
		Prefab(Scene* scene, Entity* rootEntity);

		void LoadToScene(Scene* dstScene);

		template <class Archive> void serialize(Archive& archive) {
			archive(PL_SER(mAssetUuid), PL_SER(mAssetName), PL_SER(mPrefabName), PL_SER(mMainEntityUuid),
					PL_SER(mEntities));

			if constexpr (Archive::is_saving::value) {
				size_t poolCount = mComponentPools.size();
				archive(poolCount);
				for (ComponentPool* pool : mComponentPools) {
					bool exists = (pool != nullptr);
					archive(exists);
					if (exists) {
						archive(*pool);
					}
				}
			}
			else if constexpr (Archive::is_loading::value) {
				size_t poolCount;
				archive(poolCount);
				std::vector<ComponentPool*> deserializedPools;

				deserializedPools.resize(poolCount, nullptr);

				for (size_t i = 0; i < poolCount; ++i) {
					bool exists;
					archive(exists);
					if (exists) {
						deserializedPools[i] = new ComponentPool();
						archive(*deserializedPools[i]);
					}
				}

				for (unsigned int i = 0; i < deserializedPools.size(); ++i) {
					mComponentPools.push_back(deserializedPools[i]);
				}
			}
		}

	  private:
		void GetEntityAndChildrenComponents(Scene* scene, Entity* entity);
	};
} // namespace Plaza
