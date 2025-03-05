#include "Engine/Core/PreCompiledHeaders.h"
#include "ECSManager.h"
#include "Engine/Core/Scene.h"

namespace Plaza {
	void ECS::EntitySystem::SetParent(Scene* scene, Entity* child, Entity* newParent) {
		uint64_t oldParentUuid = child->parentUuid;
		if (oldParentUuid == newParent->uuid)
			return;

		Entity* oldParent = scene->GetEntity(oldParentUuid);
		if (oldParent) {
			bool haveChild = std::find(oldParent->childrenUuid.begin(), oldParent->childrenUuid.end(), child->uuid) !=
							 oldParent->childrenUuid.end();
			if (haveChild)
				oldParent->childrenUuid.erase(
					std::find(oldParent->childrenUuid.begin(), oldParent->childrenUuid.end(), child->uuid));
		}

		child->parentUuid = newParent->uuid;
		newParent->childrenUuid.push_back(child->uuid);
	}

	uint64_t ECS::EntitySystem::Instantiate(Scene* scene, uint64_t uuidToInstantiate, bool newTransform) {
		return ECS::EntitySystem::Instantiate(scene, scene, uuidToInstantiate, 0, newTransform);
	}

	uint64_t ECS::EntitySystem::Instantiate(Scene* srcScene, Scene* dstScene, uint64_t srcUuid, uint64_t dstUuid,
											bool newTransform) {
		return ECS::EntitySystem::Instantiate(srcScene->entities, srcScene->mComponentPools, dstScene, srcUuid, dstUuid,
											  newTransform);
	}

	uint64_t ECS::EntitySystem::Instantiate(const std::unordered_map<uint64_t, Entity>& srcEntities,
											const std::vector<ComponentPool*>& srcComponentPools, Scene* dstScene,
											uint64_t srcUuid, uint64_t dstUuid, bool newTransform) {
		PLAZA_PROFILE_SECTION("Instantiate");
		const Entity* entityToInstantiate = nullptr;
		if (srcEntities.find(srcUuid) == srcEntities.end())
			return 0;
		else
			entityToInstantiate = &srcEntities.at(srcUuid);

		Entity* instantiatedEntity = dstScene->NewEntity(entityToInstantiate->name,
														 entityToInstantiate->uuid == entityToInstantiate->parentUuid
															 ? dstScene->GetEntity(dstScene->mainSceneEntityUuid)
															 : dstScene->GetEntity(entityToInstantiate->parentUuid),
														 newTransform);
		uint64_t newUuid = instantiatedEntity->uuid;

		int poolIndex = 0;
		for (auto& pool : srcComponentPools) {
			if (!pool || !entityToInstantiate->mComponentMask.test(pool->mComponentMask))
				continue;

			if (dstScene->mComponentPools.size() <= pool->mComponentMask)
				dstScene->mComponentPools.resize(pool->mComponentMask + 1, nullptr);
			if (dstScene->mComponentPools[pool->mComponentMask] == nullptr) {
				dstScene->mComponentPools[pool->mComponentMask] =
					new ComponentPool(pool->mElementSize, pool->mComponentMask, "");
				dstScene->mComponentPools[pool->mComponentMask]->mComponentRawNameHash = pool->mComponentRawNameHash;
			}

			ECS::InstantiateComponent(pool, dstScene->mComponentPools[poolIndex], srcUuid, newUuid);
			Component* component = pool->Get(
				newUuid); //(Component*)pool->mInstantiateFactory(pool, pool->Get(uuidToInstantiate), newUuid);
			instantiatedEntity->mComponentMask.set(pool->mComponentMask);
			poolIndex++;
		}

		for (auto& pool : dstScene->mComponentPools) {
			if (!pool || !entityToInstantiate->mComponentMask.test(pool->mComponentMask) ||
				pool->Get(newUuid) == nullptr)
				continue;
			Component* component = static_cast<Component*>(pool->Get(newUuid));
			if (component->mUuid != newUuid)
				continue;
			component->OnInstantiate(dstScene, srcUuid);
		}

		/* Instantiate children */
		for (unsigned int i = 0; i < entityToInstantiate->childrenUuid.size(); i++) {
			uint64_t childUuid = entityToInstantiate->childrenUuid[i];
			uint64_t uuid =
				ECS::EntitySystem::Instantiate(srcEntities, srcComponentPools, dstScene, childUuid, 0, newTransform);
			if (uuid) {
				dstScene->SetParent(dstScene->GetEntity(uuid), dstScene->GetEntity(newUuid));
				// ECS::TransformSystem::UpdateSelfAndChildrenTransform(*scene->GetComponent<TransformComponent>(uuid),
				// scene->GetComponent<TransformComponent>(newUuid), scene);
			}
		}
		ECS::TransformSystem::UpdateSelfAndChildrenTransform(
			*dstScene->GetComponent<TransformComponent>(newUuid),
			dstScene->GetComponent<TransformComponent>(dstScene->GetEntity(entityToInstantiate->parentUuid)->uuid),
			dstScene);
		return instantiatedEntity->uuid;
	}

	void ECS::EntitySystem::Delete(Scene* scene, uint64_t uuid) {
		// FIX: Implement proper deletion of entites
		for (auto& componentPool : scene->mComponentPools) {
		}
	}

} // namespace Plaza
