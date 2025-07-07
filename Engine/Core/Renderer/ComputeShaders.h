#pragma once
#include "Renderer.h"

namespace Plaza {
	class ComputeShaders {
	public:
		std::vector<std::shared_ptr<PlazaShadersBinding>> mInputs = std::vector<std::shared_ptr<PlazaShadersBinding>>();
		std::vector<std::shared_ptr<PlazaShadersBinding>> mOutputs = std::vector<std::shared_ptr<PlazaShadersBinding>>();

		virtual void AddInput() = 0;
		virtual void AddOutput() = 0;
		virtual void Init(const std::string& path);
		virtual void Dispatch(int x, int y, int z);
		virtual void Terminate();
	};
}
