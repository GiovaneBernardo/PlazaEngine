#pragma once
#include "ThirdParty/include/VulkanMemoryAllocator/vk_mem_alloc.h"
#include "RendererTypes.h"

namespace Plaza {
	class PlBuffer {
	public:
		PlBuffer() {};
		PlBuffer(PlBufferType type, uint64_t maxItems, uint16_t stride, uint8_t bufferCount, PlBufferUsage bufferUsage, PlMemoryUsage memoryUsage, const std::string& name)
			: mType(type), mMaxItems(maxItems), mStride(stride), mBufferCount(bufferCount), mBufferUsage(bufferUsage), mMemoryUsage(memoryUsage), mName(name) {};
		std::string mName = "";
		PlBufferType mType = PL_BUFFER_UNIFORM_BUFFER;
		uint64_t mMaxItems = 0;
		uint64_t mStride = 0;
		uint8_t mBufferCount = 0;
		uint64_t mCurrentItemCount = 0;
		uint64_t mCurrentBufferSize = 0;
		PlBufferUsage mBufferUsage{};
		PlMemoryUsage mMemoryUsage{};
		PlMemoryProperty mMemoryProperties{};

		virtual void CreateMemory(VmaAllocationCreateFlags flags = 0, unsigned int buffersCount = 1) {}
		virtual void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage vmaUsage, VmaAllocationCreateFlags flags = 0, unsigned int buffersCount = 1) {}
		template <typename T>
		void UpdateData(unsigned int index, const T& newData) {
			UpdateDataHelper(index, &newData, sizeof(T));
		}
		virtual void Destroy() {};

	private:
		virtual void UpdateDataHelper(unsigned int index, const void* newData, size_t size) = 0;
	};
}