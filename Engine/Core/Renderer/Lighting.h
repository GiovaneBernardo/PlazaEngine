#pragma once

namespace Plaza {
	class Lighting {
	  public:
		struct LightStruct {
			alignas(16) glm::vec3 color;
			alignas(0) float radius;
			alignas(16) glm::vec3 position;
			alignas(0) float intensity;
			alignas(4) float cutoff;
			alignas(4) float minRadius;
		};

		struct Tile {
			int lightIndices[256];
			alignas(16) glm::vec3 minBounds;
			alignas(0) int lightsCount = 0;
			alignas(16) glm::vec3 maxBounds;
			alignas(0) int alignment = 0;
		};

		struct Plane {
			glm::vec3 Normal;
			float Distance;
		};

		struct Frustum {
			Plane planes[4];
		};

		virtual void Init() = 0;
		virtual void GetLights(Scene* scene) = 0;
		virtual void UpdateTiles() = 0;
		virtual void Terminate() = 0;

		glm::vec4 directionalLightColor = glm::vec4(1.0f);
		float directionalLightIntensity = 0.9f;
		glm::vec4 ambientLightColor = glm::vec4(1.0f);
		float ambientLightIntensity = 0.3f;
		glm::vec2 mScreenSize = glm::vec2(0.0f);
		int mLightsSize;
		std::vector<Tile> mTiles;
		std::vector<LightStruct> mLights;
	};
} // namespace Plaza
