#include "Engine/Core/PreCompiledHeaders.h"
#include "PerlinNoise.h"

namespace Plaza {
	float PerlinNoise::Noise(glm::vec3 vec) {
		int X = static_cast<int>(floor(vec.x)) & 255;
		int Y = static_cast<int>(floor(vec.y)) & 255;
		int Z = static_cast<int>(floor(vec.z)) & 255;
		vec.x -= floor(vec.x);
		vec.y -= floor(vec.y);
		vec.z -= floor(vec.z);

		float u = Fade(vec.x);
		float v = Fade(vec.y);
		float w = Fade(vec.z);

		int A = p[X] + Y;
		int AA = p[A] + Z;
		int AB = p[A + 1] + Z;
		int B = p[X + 1] + Y;
		int BA = p[B] + Z;
		int BB = p[B + 1] + Z;

		return Lerp(
			w,
			Lerp(v, Lerp(u, Grad(p[AA], vec.x, vec.y, vec.z), Grad(p[BA], vec.x - 1, vec.y, vec.z)),
				 Lerp(u, Grad(p[AB], vec.x, vec.y - 1, vec.z), Grad(p[BB], vec.x - 1, vec.y - 1, vec.z))),
			Lerp(v, Lerp(u, Grad(p[AA + 1], vec.x, vec.y, vec.z - 1), Grad(p[BA + 1], vec.x - 1, vec.y, vec.z - 1)),
				 Lerp(u, Grad(p[AB + 1], vec.x, vec.y - 1, vec.z - 1),
					  Grad(p[BB + 1], vec.x - 1, vec.y - 1, vec.z - 1))));
	}

	float PerlinNoise::Fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }

	float PerlinNoise::Lerp(float t, float a, float b) { return a + t * (b - a); }

	float PerlinNoise::Grad(int hash, float x, float y, float z) {
		int h = hash & 15;
		float u = h < 8 ? x : y;
		float v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
		return ((h & 1 ? -1 : 1) * u + (h & 2 ? -1 : 1) * v);
	}
} // namespace Plaza
