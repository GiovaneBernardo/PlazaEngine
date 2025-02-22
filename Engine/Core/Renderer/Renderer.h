#pragma once
#include "Mesh.h"
#include "Engine/Components/Rendering/Material.h"
#include "Engine/Core/RenderGroup.h"
#include "Shadows.h"
#include "Lighting.h"
#include "Skybox.h"
#include "Picking.h"
#include "GuiRenderer.h"
#include "RenderGraph.h"
#include "Engine/Core/Engine.h"

namespace Plaza {

	enum class RendererAPI {
		Vulkan
	};

	struct TrackerSettings {
		uint8_t mMipLevel = 0;
		uint8_t mLayerLevel = 0;
	};
	struct TrackedImage {
		TrackedImage(const std::string& newName, const TextureInfo& info) : name(newName), mTextureInfo(info) {};
		TrackedImage() {}
		ImTextureID mTextureID = 0;
		std::chrono::system_clock::time_point mCreationDate = std::chrono::system_clock::now();
		std::string name;
		TrackerSettings mTrackerSetting;
		TextureInfo mTextureInfo;
		bool mRecalculateView = true;
	};

	class PLAZA_API Renderer {
	public:

		std::vector<TrackedImage*> mTrackedImages = std::vector<TrackedImage*>();
		TrackedImage* GetTrackedImage(unsigned int index) { return mTrackedImages[index]; };
		template<typename T>
		void AddTrackedImage(const T& trackedImage) { mTrackedImages.push_back(new T(trackedImage)); };
		virtual ImTextureID GetTrackedImageID(TrackedImage* tracked) = 0;

		uint32_t mCurrentFrame = 0;

		float exposure = 4.0f;
		float gamma = 2.2f;
		glm::vec3 sunColor = glm::vec3(1.0f);
		float mSkyboxIntensity = 1.0f;

		RendererAPI api;
		Shadows* mShadows;
		Lighting* mLighting;
		Skybox* mSkybox;
		Picking* mPicking;
		GuiRenderer* mGuiRenderer;
		PlazaRenderGraph* mRenderGraph = nullptr;
		const unsigned int mMaxFramesInFlight = 2;
		const uint64_t mMaxBindlessTextures = 16536;

		virtual void Init() = 0;
		virtual void InitializeRenderGraph(PlazaRenderGraph* renderGraph) = 0;
		virtual void UpdateProjectManager() = 0;
		virtual void Render(Scene* scene) = 0;
		virtual void RenderImGuiFrame(std::vector<ImDrawData*> drawDatas) = 0;
		virtual void RecordImGuiFrame(std::vector<ImDrawData*> drawDatas) = 0;
		virtual void UpdateMainProgressBar(float percentage) = 0;
		virtual void UpdateImGuiDisplayTexture(Texture* texture) = 0;

		virtual Mesh* CreateNewMesh(
			const std::vector<glm::vec3>& vertices,
			const std::vector<glm::vec3>& normals,
			const std::vector<glm::vec2>& uvs,
			const std::vector<glm::vec3>& tangent,
			const std::vector<unsigned int>& indices,
			const std::vector<unsigned int>& materialsIndices,
			bool usingNormal,
			const std::vector<BonesHolder>& boneIds = std::vector<BonesHolder>(),
			const std::vector<Bone>& uniqueBonesInfo = std::vector<Bone>()) = 0;
		virtual void DeleteMesh(Mesh& mesh) = 0;
		virtual Mesh* RestartMesh(Mesh* mesh) = 0;
		virtual void DrawRenderGroupInstanced(RenderGroup* renderGroup) = 0;

		virtual void InitGUI() = 0;
		virtual void NewFrameGUI() = 0;
		virtual void UpdateGUI() = 0;
		virtual ImTextureID GetFrameImage() = 0;

		virtual void Destroy() = 0;

		virtual Texture* LoadTexture(std::string path, uint64_t uuid = 0) {
			return new Texture();
		}
		virtual Texture* LoadImGuiTexture(std::string path) {
			return new Texture();
		}

		static glm::vec3 ReconstructWorldPositionFromDepth(glm::vec2 screenPosition, glm::vec2 screenSize, float depth, Camera* camera) {
			glm::mat4 viewProjection = camera->GetProjectionMatrix() * camera->GetViewMatrix();
			float ndcZ = depth * 2.0f - 1.0f;
			glm::vec4 ndcPosition = glm::vec4(screenPosition * (1.0f / screenSize) * 2.0f - 1.0f, depth, 1.0f);
			ndcPosition.y *= -1.0f;
			glm::vec4 worldPosition = glm::inverse(viewProjection) * ndcPosition;
			return glm::vec3(worldPosition) / worldPosition.w;
		}

		virtual void CopyLastFramebufferToFinalDrawBuffer() = 0;
	};
}