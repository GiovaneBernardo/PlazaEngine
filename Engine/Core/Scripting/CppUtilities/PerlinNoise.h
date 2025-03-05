#pragma once
#include "Engine/Core/Engine.h"

namespace Plaza {
	class PLAZA_API PerlinNoise {
	  public:
		PerlinNoise() {
			srand(time(0));
			for (int i = 0; i < 256; ++i) {
				p[i] = rand() % 256;
			}

			for (int i = 0; i < 256; ++i) {
				p[256 + i] = p[i];
			}
		}

		float Noise(glm::vec3 vec);

	  private:
		int p[512];

		static float Fade(float t);

		static float Lerp(float t, float a, float b);

		static float Grad(int hash, float x, float y, float z);
	};
} // namespace Plaza
