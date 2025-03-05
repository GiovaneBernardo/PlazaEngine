#pragma once
#include "any"

namespace Plaza::Editor {
	struct CopyContent {
		std::unique_ptr<std::any> mData;
	};

	class CopyPasteManager {
	  public:
		template <typename T> std::unique_ptr<T> GetContent() { return mContents.end(); }
		template <typename T> std::vector<std::unique_ptr<T>> GetContents(int start, int count) {
			return (mContents.at(start), mContents.at(start + count));
		}

		template <typename T> void AddContent(T& data) { mContents.push_back({std::make_unique<T>(data)}); }

	  private:
		std::vector<CopyContent> mContents = std::vector<CopyContent>();
	};
} // namespace Plaza::Editor
