#pragma once
#include "GuiItem.h"

namespace Plaza {
	class GuiRectangle : public GuiItem {
	  public:
		float mPosX = 0.0f;
		float mPosY = 0.0f;
		float mSizeX = 0.0f;
		float mSizeY = 0.0f;
		float mScale = 1.0f;
		glm::vec4 mColor = glm::vec4(1.0f);

		GuiRectangle(std::string guiName, float posX, float posY, float sizeX, float sizeY, float scale,
					 glm::vec4 color)
			: GuiItem(guiName), mPosX(posX), mPosY(posY), mSizeX(sizeX), mSizeY(sizeY), mScale(scale), mColor(color){

																									   };
		GuiRectangle() : GuiItem() {}

		template <class Archive> void serialize(Archive& archive) {
			archive(cereal::base_class<GuiItem>(this), PL_SER(mPosX), PL_SER(mPosY), PL_SER(mSizeX), PL_SER(mSizeY),
					PL_SER(mScale), PL_SER(mColor));
		}
	};

} // namespace Plaza
