#pragma once

namespace Plaza {
	typedef uint64_t EntityId;
#define MAX_ENTITIES 65536 * 4
	struct PLAZA_API ComponentPool {
		ComponentPool() {}
		ComponentPool(size_t elementsSize, size_t componentMask, const std::string& rawComponentName);
		ComponentPool(ComponentPool& other);

		~ComponentPool() { mData.clear(); }

		inline Component* Get(size_t index) {
			uint64_t sparsedIndex = mSparseMap[index];
			if (mSize < sparsedIndex)
				return nullptr;
			return mData[sparsedIndex].get();
		}

		inline bool Has(size_t index) {
			return mSparseMap.find(index) != mSparseMap.end();
		}

		template <typename T> inline T* New(size_t index) {
			mSparseMap[index] = mSize;
			std::shared_ptr<T> component = std::make_shared<T>();
			mData.push_back(component);
			static_cast<Component*>(component.get())->mUuid = index;
			mSize++;
			return component.get();
		}

		template <typename T> T* Add(size_t index, T* component = nullptr) {
			void* storage = Get(index);

			if (component == nullptr)
				component = new T();
			mData.push_back(std::make_shared<T>(component));

			mSparseMap[index] = mSize;
			mSize++; //= std::max(mSize, index + 1);

			return component;
		}

		void Remove(size_t index) {
			void* storage = Get(index);

			mData.erase(std::find(mData.begin(), mData.end(), mData[mSparseMap[index]]));

			mSparseMap.erase(mSparseMap.find(index));
			mSize--;
		}

		std::map<EntityId, EntityId> mSparseMap;
		std::vector<std::shared_ptr<Component>> mData;
		size_t mSize = 1;
		size_t mCapacity = MAX_ENTITIES;
		size_t mElementSize{0};
		size_t mComponentMask;
		size_t mComponentRawNameHash;

		std::function<void*(ComponentPool* poolToAddComponent, void*, uint64_t)> mInstantiateFactory;

		template <typename Archive> void serialize(Archive& archive) {
			archive(PL_SER(mSize), PL_SER(mCapacity), PL_SER(mElementSize), PL_SER(mComponentMask),
					PL_SER(mComponentRawNameHash), PL_SER(mSparseMap), PL_SER(mData));
			mSize = mData.size();
		}
	};
} // namespace Plaza
