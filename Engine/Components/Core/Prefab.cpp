#include "Engine/Core/PreCompiledHeaders.h"
#include "Prefab.h"
#include "Engine/Core/Scene.h"

namespace Plaza {
	Prefab::Prefab(Scene* scene, Entity* rootEntity) {
		mAssetUuid = Plaza::UUID::NewUUID();
		if (!rootEntity)
			return;

		mPrefabName = rootEntity->name;
		mMainEntityUuid = rootEntity->uuid;
		this->GetEntityAndChildrenComponents(scene, rootEntity);
	}

	void Prefab::GetEntityAndChildrenComponents(Scene* scene, Entity* entity) {
		mEntities.emplace(entity->uuid, *entity);
		for (ComponentPool* pool : scene->mComponentPools) {
			if (mComponentPools.size() <= pool->mComponentMask)
				mComponentPools.resize(pool->mComponentMask + 1, nullptr);
			if (mComponentPools[pool->mComponentMask] == nullptr) {
				mComponentPools[pool->mComponentMask] = new ComponentPool(pool->mElementSize, pool->mComponentMask, "");
				mComponentPools[pool->mComponentMask]->mComponentRawNameHash = pool->mComponentRawNameHash;
			}

			Component* component = pool->Get(entity->uuid);
			if (component) {
				ECS::InstantiateComponent(pool, mComponentPools[pool->mComponentMask], entity->uuid, entity->uuid);
			}
		}

		MeshRenderer* meshRenderer = scene->GetComponent<MeshRenderer>(entity->uuid);
		if (meshRenderer && meshRenderer->GetMesh()) {
			mMeshesUuid.emplace(meshRenderer->mMeshUuid);
		}

		for (uint64_t child : entity->childrenUuid) {
			this->GetEntityAndChildrenComponents(scene, scene->GetEntity(child));
		}
	}

	void Prefab::LoadToScene(Scene* dstScene) {
		ECS::EntitySystem::Instantiate(mEntities, mComponentPools, dstScene, mMainEntityUuid, 0);
	}
} // namespace Plaza
