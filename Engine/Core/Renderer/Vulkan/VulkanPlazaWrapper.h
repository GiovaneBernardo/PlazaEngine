#pragma once
#include "ThirdParty/include/VulkanMemoryAllocator/vk_mem_alloc.h"
#include "Engine/Core/Engine.h"
#include "Engine/Core/Renderer/Buffer.h"

namespace Plaza {
	class PLAZA_API PlVkBuffer : public PlBuffer {
	  public:
		PlVkBuffer(){};
		PlVkBuffer(PlBufferType type, uint64_t maxItems, uint16_t stride, uint8_t bufferCount,
				   PlBufferUsage bufferUsage, PlMemoryUsage memoryUsage, const std::string& name)
			: PlBuffer(type, maxItems, stride, bufferCount, bufferUsage, memoryUsage, name){};

		// uint64_t descriptorCount, uint8_t binding, PlBufferType type, PlRenderStage stage, uint64_t maxItems,
		// uint16_t stride, uint8_t bufferCount, const std::string& name

		VmaAllocator GetVmaAllocator();

		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage vmaUsage,
						  VmaAllocationCreateFlags flags = 0, unsigned int buffersCount = 1) override;

		void CreateMemory(VmaAllocationCreateFlags flags = 0, unsigned int buffersCount = 1);

		void Destroy();

		VkBuffer& GetBuffer(unsigned int index = 0) { return mBuffers[index]; }
		VmaAllocation& GetAllocation(unsigned int index = 0) { return mAllocations[index]; }

		VkDeviceMemory& GetMemory(unsigned int index = 0) {
			if (mMemories.size() < mBuffers.size()) {
			}
			return mMemories[index];
		}
		template <class Archive> void serialize(Archive& archive) { archive(cereal::base_class<PlBuffer>(this)); }

	  private:
		std::vector<VkBuffer> mBuffers = std::vector<VkBuffer>();
		std::vector<VkDeviceMemory> mMemories = std::vector<VkDeviceMemory>();
		std::vector<VmaAllocation> mAllocations = std::vector<VmaAllocation>();

		void UpdateDataHelper(unsigned int index, const void* newData, size_t size);
	};

	/*	CEREAL_REGISTER_TYPE(B);
	CEREAL_REGISTER_POLYMORPHIC_RELATION(PlBuffer, B);*/

} // namespace Plaza
