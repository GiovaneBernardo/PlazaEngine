#include "Engine/Core/PreCompiledHeaders.h"
#include "RenderGroup.h"
#include "Engine/Core/Scene.h"
#include "Engine/Core/Renderer/Vulkan/Renderer.h"

namespace Plaza {
	void RenderGroup::InitializeInstanceBuffer() {
		this->mInstanceBuffers.resize(Application::Get()->mRenderer->mMaxFramesInFlight);
		this->mInstanceBufferMemories.resize(Application::Get()->mRenderer->mMaxFramesInFlight);
		for (int i = 0; i < Application::Get()->mRenderer->mMaxFramesInFlight; ++i) {
			VulkanRenderer::GetRenderer()->CreateBuffer(
				mBufferSize * sizeof(glm::mat4), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mInstanceBuffers[i],
				mInstanceBufferMemories[i]);
		}
	}

	void RenderGroup::ResizeInstanceBuffer(uint64_t newSize) {
		for (int i = 0; i < Application::Get()->mRenderer->mMaxFramesInFlight; ++i) {
			// vkFreeMemory(VulkanRenderer::GetRenderer()->mDevice, this->mInstanceBufferMemories[i], nullptr);
			uint64_t size = newSize == 0 ? this->mCount * 2 : this->mCount + newSize;
			this->mBufferSize = size;
			VulkanRenderer::GetRenderer()->CreateBuffer(size * sizeof(glm::mat4), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
														VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
															VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
														this->mInstanceBuffers[i], this->mInstanceBufferMemories[i]);
		}

		// Application::Get()->mThreadsManager->mFrameRendererAfterFenceThread->AddToQueue([newSize, this]() {
		//
		//	unsigned int currentFrame = Application::Get()->mRenderer->mCurrentFrame;
		//	vkDestroyBuffer(VulkanRenderer::GetRenderer()->mDevice, this->mInstanceBuffers[currentFrame], nullptr);
		//	vkFreeMemory(VulkanRenderer::GetRenderer()->mDevice, this->mInstanceBufferMemories[currentFrame], nullptr);
		//
		//	uint64_t size = newSize == 0 ? this->mCount * 2 : this->mCount + newSize;
		//	this->mBufferSize = size;
		//	VulkanRenderer::GetRenderer()->CreateBuffer(size * sizeof(glm::mat4), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		//VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
		//VK_MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM, this->mInstanceBuffers[currentFrame],
		//this->mInstanceBufferMemories[currentFrame]);
		//
		//	Application::Get()->mThreadsManager->mFrameRendererBeforeFenceThread->AddToQueue([newSize, this]() {
		//
		//		unsigned int currentFrame = Application::Get()->mRenderer->mCurrentFrame;
		//		vkDestroyBuffer(VulkanRenderer::GetRenderer()->mDevice, this->mInstanceBuffers[currentFrame], nullptr);
		//		vkFreeMemory(VulkanRenderer::GetRenderer()->mDevice, this->mInstanceBufferMemories[currentFrame],
		//nullptr);
		//
		//		uint64_t size = newSize == 0 ? this->mCount * 2 : this->mCount + newSize;
		//		this->mBufferSize = size;
		//		VulkanRenderer::GetRenderer()->CreateBuffer(size * sizeof(glm::mat4), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		//VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		//this->mInstanceBuffers[currentFrame], this->mInstanceBufferMemories[currentFrame]);
		//
		//
		//		});
		//	});
	}

	void RenderGroup::ChangeMaterial(Material* newMaterial, unsigned int index) {
		// Scene::GetActiveScene()->RemoveRenderGroup(this->uuid);
		// uint64_t oldMeshUuid = this->mesh->uuid;
		// uint64_t oldMaterialUuid = this->material->uuid;
		this->materials[index] = newMaterial;
		Scene::GetActiveScene()->AddRenderGroup(this);
		// RenderGroup* renderGroup = Scene::GetActiveScene()->GetRenderGroupWithUuids(oldMeshUuid, oldMaterialUuid);
		// if (renderGroup) {
		// Scene::GetActiveScene()->RemoveRenderGroup(renderGroup->uuid);
		//}
	}

	void RenderGroup::ChangeMesh(Mesh* mesh) {
		// Scene::GetActiveScene()->RemoveRenderGroup(this->uuid);
		this->mesh = mesh;
		Scene::GetActiveScene()->AddRenderGroup(this);
	}

	bool RenderGroup::SceneHasRenderGroup(Mesh* mesh, Material* materials) {
		// bool hasSpecificRenderGroup = Scene::GetActiveScene()->renderGroupsFindMap.find(std::pair<uint64_t,
		// uint64_t>(mesh->uuid, material->uuid)) != Scene::GetActiveScene()->renderGroupsFindMap.end(); return
		// hasSpecificRenderGroup;
		return true;
	}

	bool RenderGroup::SceneHasRenderGroup(RenderGroup* renderGroup) {
		// return SceneHasRenderGroup(renderGroup->mesh, renderGroup->mMaterials);
		return true;
	}
} // namespace Plaza
