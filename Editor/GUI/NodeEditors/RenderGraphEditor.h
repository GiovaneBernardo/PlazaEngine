#pragma once
#include "NodeEditor.h"
#include "Engine/Core/Renderer/RendererTypes.h"

namespace Plaza {
	class PlazaRenderGraph;
}
namespace Plaza::Editor {
	class RenderGraphEditor : public NodeEditor {
	  public:
		RenderGraphEditor(GuiLayer layer, bool startOpen = true) : NodeEditor("RenderGraphEditor", layer, startOpen) {}
		void Init() override;
		void Process() override;
		void SpawnLockedNodes() override;
		// void Update() override;
		// void OnKeyPress(int key, int scancode, int action, int mods) override;

		// ax::NodeEditor::EditorContext* mContext = nullptr;
		void LoadRenderGraphNodes(PlazaRenderGraph* renderGraph);

	  public:
		struct TextureNodeStruct {
			uint64_t descriptorCount = 0;
			PlImageUsage imageUsage = PlImageUsage::PL_IMAGE_USAGE_COLOR_ATTACHMENT;
			PlTextureType imageType = PlTextureType::PL_TYPE_2D;
			PlViewType viewType = PlViewType::PL_VIEW_TYPE_2D;
			PlTextureFormat format = PlTextureFormat::PL_FORMAT_R8G8B8A8_UNORM;
			glm::vec3 resolution = glm::vec3(0.0f);
			uint8_t mipCount = 0;
			uint16_t layersCount = 1;
			std::string name = "";
			std::string resourceName = "";

			template <class Archive> void serialize(Archive& archive) {
				archive(PL_SER(descriptorCount), PL_SER(imageUsage), PL_SER(imageType), PL_SER(viewType),
						PL_SER(format), PL_SER(resolution), PL_SER(mipCount), PL_SER(layersCount), PL_SER(name),
						PL_SER(resourceName));
			}
		};

		struct BufferNodeStruct {
			PlBufferType type = PlBufferType::PL_BUFFER_UNIFORM_BUFFER;
			uint64_t maxItems = 0;
			uint16_t stride = 0;
			uint8_t bufferCount = 0;
			PlBufferUsage bufferUsage = PlBufferUsage::PL_BUFFER_USAGE_STORAGE_BUFFER;
			PlMemoryUsage memoryUsage = PlMemoryUsage::PL_MEMORY_USAGE_CPU_TO_GPU;
			std::string name = "";
			std::string resourceName = "";

			template <class Archive> void serialize(Archive& archive) {
				archive(PL_SER(type), PL_SER(maxItems), PL_SER(stride), PL_SER(bufferCount), PL_SER(bufferUsage),
						PL_SER(memoryUsage), PL_SER(name), PL_SER(resourceName));
			}
		};
	};
} // namespace Plaza::Editor
