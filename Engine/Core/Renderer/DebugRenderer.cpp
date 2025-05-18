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
    void DebugRenderer::AddBox(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale, const PlColor& color, bool solid) {
        mDebugBoxes.push_back(DebugBox(position, rotation, scale, color, solid));
    }
    void DebugRenderer::AddSphere(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale, const PlColor& color, float radius, bool solid) {
        mDebugSpheres.push_back(DebugSphere(position, rotation, scale, color, radius, solid));
    }
    void DebugRenderer::AddCircle(const glm::vec3& position, const glm::vec3& scale, const PlColor& color, float radius, bool solid) {
        mDebugCircles.push_back(DebugCircle(position, scale, color, radius, solid));
    }
    void DebugRenderer::AddRectangle(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale, const PlColor& color, bool solid, std::array<glm::vec3, 8>& corners) {
        mDebugRectangles.push_back(DebugRectangle(position, rotation, scale, color, solid, corners));
    }

}
