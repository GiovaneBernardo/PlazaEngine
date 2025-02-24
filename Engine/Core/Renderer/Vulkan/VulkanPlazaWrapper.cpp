#include "VulkanPlazaWrapper.h"
#include "Renderer.h"
#include "Engine/Core/Engine.h"

namespace Plaza {
	VmaAllocator PlVkBuffer::GetVmaAllocator() {
		return VulkanRenderer::GetRenderer()->mVmaAllocator;
	}

	static std::mutex vmaMutex;
	void PlVkBuffer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage vmaUsage, VmaAllocationCreateFlags flags, unsigned int buffersCount) {
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = vmaUsage;
		allocInfo.flags = flags;

		mBuffers.resize(buffersCount);
		mAllocations.resize(buffersCount);

		std::lock_guard<std::mutex> lock(vmaMutex);
		for (unsigned int i = 0; i < buffersCount; ++i) {
			vmaCreateBuffer(GetVmaAllocator(), &bufferInfo, &allocInfo, &mBuffers[i], &mAllocations[i], nullptr);
		}
	}

	void PlVkBuffer::CreateMemory(VmaAllocationCreateFlags flags, unsigned int buffersCount) {

	}

	void PlVkBuffer::Destroy() {
		static std::mutex destroyMutex;
		std::lock_guard<std::mutex> lock(destroyMutex);
		std::lock_guard<std::mutex> lock2(vmaMutex);
		for (unsigned int i = 0; i < mBuffers.size(); ++i) {
			vmaDestroyBuffer(GetVmaAllocator(), mBuffers[i], mAllocations[i]);
		}
	}

	void PlVkBuffer::UpdateDataHelper(unsigned int index, const void* newData, size_t size) {
		void* data;
		vmaMapMemory(GetVmaAllocator(), mAllocations[index], &data);
		memcpy(data, newData, size);
		vmaUnmapMemory(GetVmaAllocator(), mAllocations[index]);
	}
}

PL_SER_REGISTER_TYPE(Plaza::PlVkBuffer);
PL_SER_REGISTER_POLYMORPHIC_RELATION(Plaza::PlBuffer, Plaza::PlVkBuffer);