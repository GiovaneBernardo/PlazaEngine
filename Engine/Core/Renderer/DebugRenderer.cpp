#include "Engine/Core/PreCompiledHeaders.h"
#include "DebugRenderer.h"

namespace Plaza {
	void DebugRenderer::Clear() {
		mDebugLines.clear();
		mDebugBoxes.clear();
		mDebugSpheres.clear();
		mDebugCircles.clear();
		mDebugRectangles.clear();
	}

	void DebugRenderer::AddLine(const glm::vec3& start, const glm::vec3& end, float thickness, const PlColor& color) {
		mDebugLines.push_back(DebugLine(start, end, thickness, color));
	}

	void DebugRenderer::AddBox(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale,
							   const PlColor& color, bool solid) {
		if (solid) {
			mDebugBoxes.push_back({position, rotation, scale, color, true});
			return;
		}

		const glm::vec3 h = scale * 0.5f;

		// Local-space corners
		glm::vec3 corners[8] = {{-h.x, -h.y, -h.z}, {h.x, -h.y, -h.z}, {h.x, h.y, -h.z}, {-h.x, h.y, -h.z},
								{-h.x, -h.y, h.z},	{h.x, -h.y, h.z},  {h.x, h.y, h.z},	 {-h.x, h.y, h.z}};

		// Transform to world space
		for (glm::vec3& c : corners) {
			c = position + (rotation * c);
		}

		auto line = [&](int a, int b) { mDebugLines.emplace_back(corners[a], corners[b], 1.0f, color); };

		// Bottom face
		line(0, 1);
		line(1, 2);
		line(2, 3);
		line(3, 0);

		// Top face
		line(4, 5);
		line(5, 6);
		line(6, 7);
		line(7, 4);

		// Vertical edges
		line(0, 4);
		line(1, 5);
		line(2, 6);
		line(3, 7);
	}

	void DebugRenderer::AddSphere(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale,
								  const PlColor& color, float radius, bool solid) {
		mDebugSpheres.push_back(DebugSphere(position, rotation, scale, color, radius, solid));
	}

	void DebugRenderer::AddCircle(const glm::vec3& position, const glm::vec3& scale, const PlColor& color, float radius,
								  bool solid) {
		mDebugCircles.push_back(DebugCircle(position, scale, color, radius, solid));
	}
	void DebugRenderer::AddRectangle(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale,
									 const PlColor& color, bool solid, std::array<glm::vec3, 8>& corners) {
		mDebugRectangles.push_back(DebugRectangle(position, rotation, scale, color, solid, corners));
	}

} // namespace Plaza
