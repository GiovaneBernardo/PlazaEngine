#pragma once
#include "ThirdParty/include/VulkanMemoryAllocator/vk_mem_alloc.h"
#include "RendererTypes.h"
#include "Engine/Core/Engine.h"

namespace Plaza {
	class PLAZA_API PlBuffer {
	  public:
		PlBuffer(){};
		PlBuffer(PlBufferType type, uint64_t maxItems, uint16_t stride, uint8_t bufferCount, PlBufferUsage bufferUsage,
				 PlMemoryUsage memoryUsage, const std::string& name)
			: mType(type), mMaxItems(maxItems), mStride(stride), mBufferCount(bufferCount), mBufferUsage(bufferUsage),
			  mMemoryUsage(memoryUsage), mName(name){};
		std::string mName = "";
		PlBufferType mType = PL_BUFFER_UNIFORM_BUFFER;
		uint64_t mMaxItems = 0;
		uint64_t mStride = 0;
		uint8_t mBufferCount = 0;
		std::atomic<uint64_t> mCurrentItemCount = 0;
		uint64_t mCurrentBufferSize = 0;
		PlBufferUsage mBufferUsage{};
		PlMemoryUsage mMemoryUsage{};
		PlMemoryProperty mMemoryProperties{};

		virtual void CreateMemory(VmaAllocationCreateFlags flags = 0, unsigned int buffersCount = 1) {}
		virtual void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage vmaUsage,
								  VmaAllocationCreateFlags flags = 0, unsigned int buffersCount = 1) {}
		template <typename T> void UpdateData(unsigned int index, const T& newData) {
			UpdateDataHelper(index, &newData, sizeof(T));
		}
		template <typename T> void UpdateData(unsigned int index, const void* newData, const size_t count = 1) {
			UpdateDataHelper(index, newData, sizeof(T) * count);
		}
		virtual void Destroy() {};

		template <class Archive> void serialize(Archive& archive) {
			uint64_t mCurrentItemCountSerialize = mCurrentItemCount;
			archive(PL_SER(mName), PL_SER(mType), PL_SER(mMaxItems), PL_SER(mStride), PL_SER(mBufferCount),
					PL_SER(mCurrentItemCountSerialize), PL_SER(mCurrentBufferSize), PL_SER(mBufferUsage),
					PL_SER(mMemoryUsage), PL_SER(mMemoryProperties));
			mCurrentItemCount = mCurrentItemCountSerialize;
		}

	  private:
		virtual void UpdateDataHelper(unsigned int index, const void* newData, size_t size) = 0;
	};
} // namespace Plaza
