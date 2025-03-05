#include "GuiItem.h"
#include "Engine/Core/Scene.h"

namespace Plaza {
	GuiComponent* GuiItem::GetOwnerComponent() {
		return Scene::GetActiveScene()->GetComponent<GuiComponent>(this->mComponentUuid);
	}

	void GuiItem::SetPosition(glm::vec2 newPosition) {
		mPosition = newPosition;
		glm::mat4 parentMatrix = this->GetOwnerComponent()->HasGuiItem(mGuiParentUuid)
									 ? GetOwnerComponent()->GetGuiItem<GuiItem>(mGuiParentUuid)->mTransform
									 : glm::mat4(1.0f);
		GuiItem::UpdateSelfAndChildrenTransform(this, parentMatrix);
	}

	void GuiItem::SetSize(glm::vec2 newSize) {
		mSize = newSize;
		glm::mat4 parentMatrix = this->GetOwnerComponent()->HasGuiItem(mGuiParentUuid)
									 ? GetOwnerComponent()->GetGuiItem<GuiItem>(mGuiParentUuid)->mTransform
									 : glm::mat4(1.0f);
		GuiItem::UpdateSelfAndChildrenTransform(this, parentMatrix);
	}

	void GuiItem::UpdateLocalTransform(GuiItem* item, glm::mat4& parentTransform) {
		glm::mat4 localTransform = glm::translate(glm::mat4(1.0f), glm::vec3(item->GetLocalPosition(), 0.0f));
		// localTransform = glm::scale(localTransform, glm::vec3(item->GetSize(), 1.0f));
		item->mTransform = parentTransform * localTransform;
	}

	void GuiItem::UpdateSelfAndChildrenTransform(GuiItem* item, glm::mat4& parentTransform) {
		// item->mWorldPosition = parentTransform * glm::vec4(item->GetPosition(), 0.0f, 1.0f);
		glm::vec3 scale;

		// Extract the scale from each column
		scale.x = glm::length(glm::vec3(parentTransform[0])); // Length of the first column (X axis)
		scale.y = glm::length(glm::vec3(parentTransform[1])); // Length of the second column (Y axis)
		scale.z = glm::length(glm::vec3(parentTransform[2])); // Length of the third column (Z axis)

		item->mTransform = parentTransform * glm::translate(glm::mat4(1.0f), glm::vec3(item->GetLocalPosition(), 0.0f));
		item->mTransform = glm::scale(item->mTransform, glm::vec3(1.0f) / scale);
		// GuiItem::UpdateLocalTransform(item, parentTransform);
		// item->mTransform = parentTransform * item->mTransform;
		for (uint64_t& uuid : item->mGuiChildren) {
			GuiItem::UpdateSelfAndChildrenTransform(
				Scene::GetActiveScene()->GetComponent<GuiComponent>(item->mComponentUuid)->GetGuiItem<GuiItem>(uuid),
				item->mTransform);
		}
		item->mTransform = glm::scale(parentTransform, 1.0f / scale) *
						   (glm::translate(glm::mat4(1.0f), glm::vec3(item->GetLocalPosition(), 0.0f))) *
						   glm::scale(glm::mat4(1.0f), glm::vec3(item->GetLocalSize(), 1.0f));
	}
} // namespace Plaza
