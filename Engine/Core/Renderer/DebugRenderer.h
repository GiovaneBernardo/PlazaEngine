#pragma once
#include <vector>

#include "fwd.hpp"
#include "vec3.hpp"
#include "ext/matrix_transform.hpp"
#include "Color.h"

namespace Plaza {
    struct DebugLine {
        glm::vec3 mStart = glm::vec3(0, 0, 0);
        glm::vec3 mEnd = glm::vec3(0, 0, 0);
        float mThickness = 1.0f;
        PlColor mColor = PlColor(1, 1, 1, 1);
    };

    struct DebugBox {
        glm::vec3 mPosition = glm::vec3(0, 0, 0);
        glm::quat mRotation = glm::identity<glm::quat>();
        glm::vec3 mScale = glm::vec3(1, 1, 1);
        PlColor mColor = PlColor(1, 1, 1, 1);
        bool mSolid = true;
    };

    // 3D
    struct DebugSphere {
        glm::vec3 mPosition = glm::vec3(0, 0, 0);
        glm::quat mRotation = glm::identity<glm::quat>();
        glm::vec3 mScale = glm::vec3(1, 1, 1);
        PlColor mColor = PlColor(1, 1, 1, 1);
        bool mSolid = true;
        float radius = 1.0f;
    };

    // 2D
    struct DebugCircle {
        glm::vec3 mPosition = glm::vec3(0, 0, 0);
        glm::vec3 mScale = glm::vec3(1, 1, 1);
        PlColor mColor = PlColor(1, 1, 1, 1);
        bool mSolid = true;
        float radius = 1.0f;
    };

    struct DebugRectangle {
        glm::vec3 mPosition = glm::vec3(0, 0, 0);
        glm::quat mRotation = glm::identity<glm::quat>();
        glm::vec3 mScale = glm::vec3(1, 1, 1);
        PlColor mColor = PlColor(1, 1, 1, 1);
        bool mSolid = true;
        std::array<glm::vec3, 8> mCorners = {};
    };

    class DebugRenderer {
    public:
        std::vector<DebugLine> mDebugLines;
        std::vector<DebugBox> mDebugBoxes;
        std::vector<DebugSphere> mDebugSpheres;
        std::vector<DebugCircle> mDebugCircles;
        std::vector<DebugRectangle> mDebugRectangles;

        void Clear();
        void AddLine(const glm::vec3& start, const glm::vec3& end, float thickness, const PlColor& color = PlColor::WHITE);
        void AddBox(const glm::vec3& position, const glm::quat& rotation = glm::identity<glm::quat>(), const glm::vec3& scale = glm::vec3(1, 1, 1), const PlColor& color = PlColor::WHITE, bool solid = true);
        void AddSphere(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale, const PlColor& color, float radius, bool solid);
        void AddCircle(const glm::vec3& position, const glm::vec3& scale, const PlColor& color, float radius, bool solid);
        void AddRectangle(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale, const PlColor& color, bool solid, std::array<glm::vec3, 8>& corners);
        void Terminate();

        // Overloads to make it simple to use
        inline void AddSphere(const glm::vec3& position, float radius, const PlColor& color = PlColor::WHITE, bool solid = true) {
            AddSphere(position,  glm::identity<glm::quat>(), glm::vec3(1.0f, 1.0f, 1.0f), color, radius, solid);
        }

        inline void AddCircle(const glm::vec3& position, float radius, const PlColor& color = PlColor::WHITE, bool solid = true) {
            AddCircle(position, glm::vec3(1.0f, 1.0f, 1.0f), color, radius, solid);
        }

        inline void AddBox(const glm::vec3& position, std::array<glm::vec3, 8>& corners, const PlColor& color = PlColor::WHITE, bool solid = true, const glm::quat& rotation = glm::identity<glm::quat>(), const glm::vec3& scale = glm::vec3(1, 1, 1)) {
            AddRectangle(position, rotation, scale, color, solid, corners);
        }
    };
}
