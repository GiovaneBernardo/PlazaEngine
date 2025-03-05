#pragma once
#include "GuiItem.h"

namespace Plaza {
	class GuiButton : public GuiItem {
	  public:
		std::string mText = "";
		float mPosX = 0.0f;
		float mPosY = 0.0f;
		float mSizeX = 0.0f;
		float mSizeY = 0.0f;
		float mScale = 1.0f;
		float mTextScale = 1.0f;
		glm::vec4 mColor = glm::vec4(1.0f);
		std::unordered_map<uint64_t, std::string> mScriptsFunctionNameToCallWhenClicked =
			std::unordered_map<uint64_t, std::string>();

		void AddScriptCallback(uint64_t scriptUuid, std::string functionName) {
			mScriptsFunctionNameToCallWhenClicked.emplace(scriptUuid, functionName);
		}

		GuiButton() { mGuiUuid = Plaza::UUID::NewUUID(); }

		void CallScriptsCallback();

		bool MouseIsInsideButton(glm::vec2 mousePos);

		GuiButton(std::string guiName, std::string text, float posX, float posY, float sizeX, float sizeY, float scale,
				  float textScale, glm::vec4 color)
			: GuiItem(guiName), mText(text), mPosX(posX), mPosY(posY), mSizeX(sizeX), mSizeY(sizeY), mScale(scale),
			  mTextScale(textScale), mColor(color){

									 };

		template <class Archive> void serialize(Archive& archive) {
			archive(cereal::base_class<GuiItem>(this), PL_SER(mText), PL_SER(mPosX), PL_SER(mPosY), PL_SER(mSizeX),
					PL_SER(mSizeY), PL_SER(mScale), PL_SER(mTextScale), PL_SER(mColor));
		}
	};

} // namespace Plaza
