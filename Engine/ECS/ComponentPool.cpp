#include "Engine/Core/PreCompiledHeaders.h"
#include "ComponentPool.h"
#include "Engine/ECS/ECSManager.h"

namespace Plaza {
	ComponentPool::ComponentPool(size_t elementsSize, size_t componentMask, const std::string& rawComponentName) {
		mElementSize = elementsSize;
		mComponentMask = componentMask;
		mData.push_back(nullptr);
		mSize = mData.size();
		mSparseMap[0] = 0;
		hash<std::string> hasher;
		mComponentRawNameHash = hasher(rawComponentName);
	}

	ComponentPool::ComponentPool(ComponentPool& other) {
		mElementSize = other.mElementSize;
		mComponentMask = other.mComponentMask;
		for (size_t i = 0; i < other.mData.size(); ++i) {
			if (other.mData[i]) {
				ECS::InstantiateComponent(&other, this, other.mData[i]->mUuid, other.mData[i]->mUuid);
			}
			else {
				mData.push_back(nullptr);
			}
		}
		mSize = mData.size();
	}
} // namespace Plaza
