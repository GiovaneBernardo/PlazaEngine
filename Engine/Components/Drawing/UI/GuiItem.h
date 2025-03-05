#pragma once
#include <vector>

namespace Plaza {
	class GuiComponent;

	enum class GuiType {
		PL_GUI_RECTANGLE = 0,
		PL_GUI_BUTTON,
		PL_GUI_TEXT
	};

	class GuiItem {
	  public:
		uint64_t mGuiUuid = 0;
		std::string mGuiName = "";
		uint64_t mComponentUuid = 0;
		GuiType mGuiType = GuiType::PL_GUI_RECTANGLE;
		// GuiMesh mGuiMesh;

		bool mRecalculateVertices = true;

		uint64_t mGuiParentUuid = 0;
		std::vector<uint64_t> mGuiChildren = std::vector<uint64_t>();

		GuiItem(std::string guiName) : mGuiName(guiName) { mGuiUuid = Plaza::UUID::NewUUID(); }

		GuiItem() { mGuiUuid = Plaza::UUID::NewUUID(); }

		virtual ~GuiItem() = default;

		GuiComponent* GetOwnerComponent();

		glm::vec2 GetLocalPosition() { return mPosition; }
		glm::vec2 GetLocalSize() { return mSize; }

		glm::vec2 GetWorldPosition() { return glm::vec2(mTransform[3]); }
		glm::vec2 GetWorldSize() { return mSize; }

		void SetPosition(glm::vec2 newPosition);
		void SetSize(glm::vec2 newSize);

		static void UpdateLocalTransform(GuiItem* item, glm::mat4& parentTransform);
		static void UpdateSelfAndChildrenTransform(GuiItem* item, glm::mat4& parentTransform);

		glm::mat4 mTransform = glm::mat4(1.0f);

	  private:
		glm::vec2 mPosition = glm::vec2(0.0f);
		glm::vec2 mSize = glm::vec2(1.0f);

		glm::vec2 mWorldPosition = glm::vec2(0.0f);

	  public:
		template <class Archive> void serialize(Archive& archive) {
			archive(PL_SER(mGuiUuid), PL_SER(mGuiName), PL_SER(mGuiType), PL_SER(mComponentUuid),
					PL_SER(mGuiParentUuid), PL_SER(mGuiChildren), PL_SER(mPosition), PL_SER(mSize));
		}
	};
} // namespace Plaza
