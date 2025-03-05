#pragma once
#include "Engine/Components/Component.h"
#include "GuiItem.h"
#include "GuiButton.h"
#include "GuiRectangle.h"
#include "Engine/Core/Engine.h"

namespace Plaza {
	struct PLAZA_API GuiMesh {
		std::vector<glm::vec2> mVertices = std::vector<glm::vec2>();
		std::vector<unsigned int> mIndices = std::vector<unsigned int>();
	};

	class PLAZA_API GuiComponent : public Component {
	  public:
		std::unordered_map<uint64_t, std::shared_ptr<GuiItem>> mGuiItems =
			std::unordered_map<uint64_t, std::shared_ptr<GuiItem>>();

		template <typename T, typename = std::enable_if_t<std::is_base_of<GuiItem, T>::value>>
		std::shared_ptr<T> NewGuiItem(std::string newGuiName, uint64_t parentUuid) {
			auto item = std::make_shared<T>(newGuiName);
			item->mGuiParentUuid = parentUuid;
			mGuiItems.emplace(item->mGuiUuid, item);

			glm::mat4 parentTransform = this->HasGuiItem(item->mGuiParentUuid)
											? this->GetGuiItem<GuiItem>(item->mGuiParentUuid)->mTransform
											: glm::mat4(1.0f);
			GuiItem::UpdateSelfAndChildrenTransform(item.get(), parentTransform);
			return item;
		}

		template <typename T, typename = std::enable_if_t<std::is_base_of<GuiItem, T>::value>>
		std::shared_ptr<T> NewGuiItem(std::shared_ptr<T> item) {
			mGuiItems.emplace(item->mGuiUuid, item);
			if (item->mGuiParentUuid != 0)
				mGuiItems[item->mGuiParentUuid]->mGuiChildren.push_back(item->mGuiUuid);

			glm::mat4 parentTransform = this->HasGuiItem(item->mGuiParentUuid)
											? this->GetGuiItem<GuiItem>(item->mGuiParentUuid)->mTransform
											: glm::mat4(1.0f);
			GuiItem::UpdateSelfAndChildrenTransform(item.get(), parentTransform);
			return item;
		}

		template <typename T> T* GetGuiItem(uint64_t uuid) {
			if (this->HasGuiItem(uuid))
				return static_cast<T*>(mGuiItems.at(uuid).get());
			else
				return nullptr;
		}

		bool HasGuiItem(uint64_t uuid) { return mGuiItems.find(uuid) != mGuiItems.end(); }

		template <class Archive> void serialize(Archive& archive) {
			archive(cereal::base_class<Component>(this), PL_SER(mGuiItems));
		}
	};
} // namespace Plaza
