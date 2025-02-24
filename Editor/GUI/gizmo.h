#pragma once
#include "Engine/Application/EditorCamera.h"
#include "Engine/Components/Core/Entity.h"

#include <ThirdParty/glm/glm.hpp>
#include <ThirdParty/glm/gtc/matrix_transform.hpp>
#include <ThirdParty/glm/gtc/type_ptr.hpp>
#include <ThirdParty/glm/gtx/matrix_decompose.hpp>
#include <ThirdParty/glm/trigonometric.hpp>
#include <ThirdParty/glm/gtx/euler_angles.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <ThirdParty/glm/gtx/quaternion.hpp>

#include <float.h>
#include <ThirdParty/glm/gtx/euler_angles.hpp>

#include "Editor/GUI/TransformOverlay.h"
#include "Editor/GUI/guiMain.h"
#include "Engine/Application/ApplicationSizes.h"
#include "Editor/GUI/TransformOverlay.h"

namespace Plaza::Editor {
	class Gizmo {
	public:
		static void Draw(Scene* scene, Entity* entity, Camera camera);

		/// <summary>
/// Extract translation, rotation and scale from a Matrix 4 
/// <para> Rotation is returned in radians </para>
/// </summary>
		static bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);

	private:


	};
}