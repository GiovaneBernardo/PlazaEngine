#pragma once
#include "Engine/Core/Scene.h"

#include "Editor/GUI/Utils/DataVisualizer.h"

#include "Engine/Application/EditorCamera.h"
#include "Editor/GUI/EditorInspector.h"
bool visualizingNormals = false;
namespace Plaza::Editor {
	void callback(float) {
		Application->editorCamera->Update();
		//Application->editorCamera = new Plaza::Camera(*Application->editorCamera);
		//Application->activeCamera = Application->editorCamera;
	}
	void callbacke(glm::vec3) {

	}
	void callbacke2(glm::vec3) {
		Application->editorCamera->Update();
	}
	static class SceneInspector {
	public:
		static bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale)
		{
			// From glm::decompose in matrix_decompose.inl

			using namespace glm;
			using T = float;

			mat4 LocalMatrix(transform);

			// Normalize the matrix.
			if (epsilonEqual(LocalMatrix[3][3], static_cast<float>(0), epsilon<T>()))
				return false;

			// First, isolate perspective.  This is the messiest.
			if (
				epsilonNotEqual(LocalMatrix[0][3], static_cast<T>(0), epsilon<T>()) ||
				epsilonNotEqual(LocalMatrix[1][3], static_cast<T>(0), epsilon<T>()) ||
				epsilonNotEqual(LocalMatrix[2][3], static_cast<T>(0), epsilon<T>()))
			{
				// Clear the perspective partition
				LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<T>(0);
				LocalMatrix[3][3] = static_cast<T>(1);
			}

			// Next take care of translation (easy).
			translation = vec3(LocalMatrix[3]);
			LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);

			vec3 Row[3], Pdum3;

			// Now get scale and shear.
			for (length_t i = 0; i < 3; ++i)
				for (length_t j = 0; j < 3; ++j)
					Row[i][j] = LocalMatrix[i][j];

			// Compute X scale factor and normalize first row.
			scale.x = length(Row[0]);
			Row[0] = detail::scale(Row[0], static_cast<T>(1));
			scale.y = length(Row[1]);
			Row[1] = detail::scale(Row[1], static_cast<T>(1));
			scale.z = length(Row[2]);
			Row[2] = detail::scale(Row[2], static_cast<T>(1));

			// At this point, the matrix (in rows[]) is orthonormal.
			// Check for a coordinate system flip.  If the determinant
			// is -1, then negate the matrix and the scaling factors.
#if 0
			Pdum3 = cross(Row[1], Row[2]); // v3Cross(row[1], row[2], Pdum3);
			if (dot(Row[0], Pdum3) < 0)
			{
				for (length_t i = 0; i < 3; i++)
				{
					scale[i] *= static_cast<T>(-1);
					Row[i] *= static_cast<T>(-1);
				}
			}
#endif

			rotation.y = asin(-Row[0][2]);
			if (cos(rotation.y) != 0) {
				rotation.x = atan2(Row[1][2], Row[2][2]);
				rotation.z = atan2(Row[0][1], Row[0][0]);
			}
			else {
				rotation.x = atan2(-Row[2][0], Row[1][1]);
				rotation.z = 0;
			}


			return true;
		}
		static glm::vec3 DrawGizmo(glm::vec3 vector) {
			glm::mat4 projection = Application->activeCamera->GetProjectionMatrix();
			glm::mat4 view = Application->activeCamera->GetViewMatrix();
			glm::mat4 rotationMatrix = glm::eulerAngleXYZ(vector.x, vector.y, vector.z);
			glm::mat4 gizmoTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f))
				* glm::toMat4(glm::quat(vector))
				* glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));

			//glm::mat4 gizmoTransform = glm::lookAt(glm::vec3(0), vector, glm::vec3(0, 1, 0));

			ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection), ImGuizmo::OPERATION::ROTATE, ImGuizmo::MODE::WORLD, glm::value_ptr(gizmoTransform));

			if (ImGuizmo::IsUsing()) {
				glm::vec3 position, rotation, scale;
				SceneInspector::DecomposeTransform(gizmoTransform, position, rotation, scale);
				return rotation;
			}
			return vector;
		}
		SceneInspector(Scene* scene) {
			Shadows* shadows = Application->mRenderer->GetShadows();
			/* Draw Gizmo for rotating Sun */
			shadows->mLightDirection = SceneInspector::DrawGizmo(shadows->mLightDirection);

			/* Shadows */
			if (ImGui::TreeNodeEx("Shadows", ImGuiTreeNodeFlags_DefaultOpen)) {
				glm::vec3& lightDir = shadows->mLightDirection;
				Utils::DragFloat3("Light Direction: ", lightDir, 0.1f, callbacke, -360.0f, 360.0f);
				ImGui::DragInt("Depth Map Resolution: ", reinterpret_cast<int*>(&shadows->mShadowResolution), 1024, 0, *"%d");
				ImGui::DragFloat3("Light Distance: ", &shadows->mLightDirection.x);

				ImGui::TreePop();
			}

			/* Rendering */
			if (ImGui::TreeNodeEx("Rendering", ImGuiTreeNodeFlags_DefaultOpen)) {
				if (ImGui::Checkbox("Visualize Normals", &visualizingNormals)) {
					if (visualizingNormals) {
						Application->shader = new Shader((Application->enginePath + "\\Shaders\\normals\\normalVisualizerVertex.glsl").c_str(),
							(Application->enginePath + "\\Shaders\\normals\\normalVisualizerFragment.glsl").c_str(), (Application->enginePath + "\\Shaders\\normals\\normalVisualizerGeometry.glsl").c_str());
					}
					else {
						Application->shader = new Shader((Application->enginePath + "\\Shaders\\1.model_loadingVertex.glsl").c_str(), (Application->enginePath + "\\Shaders\\1.model_loadingFragment.glsl").c_str());
					}
				}
				ImGui::TreePop();
			}

			/* Editor Camera */
			if (ImGui::TreeNodeEx("Editor Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
				Utils::DragFloat("Zoom: ", Application->editorCamera->Zoom, 0.1f, callback, 0.0f);
				Utils::DragFloat("Movement Speed: ", Application->editorCamera->MovementSpeed, 0.1f, callback, 0.0f);
				Utils::DragFloat("Camera Sensitivity: ", Application->editorCamera->MouseSensitivity, 0.1f, callback, 0.0f);
				Utils::DragFloat("Nearplane: ", Application->editorCamera->nearPlane, 0.1f, callback, -10000.0f);
				Utils::DragFloat("Farplane: ", Application->editorCamera->farPlane, 0.1f, callback, 0.0f);

				Utils::DragFloat("Pitch: ", Application->editorCamera->Pitch, 0.1f, callback);
				Utils::DragFloat3("Front: ", Application->editorCamera->Front, 0.1f, callbacke);
				Utils::DragFloat3("Right: ", Application->editorCamera->Right, 0.1f, callbacke2);
				Utils::DragFloat3("Up: ", Application->editorCamera->Up, 0.1f, callbacke2);
				Utils::DragFloat3("WorldUp: ", Application->editorCamera->WorldUp, 0.1f, callbacke2);
				Utils::DragFloat("Yaw: ", Application->editorCamera->Yaw, 0.1f, callback);

				Utils::DragFloat("Aspect Ratio: ", Application->editorCamera->aspectRatio, 0.1f, callback);
				Utils::DragFloat("X: ", Application->appSizes->sceneSize.x, 0.1f, callback);
				Utils::DragFloat("Y: ", Application->appSizes->sceneSize.y, 0.1f, callback);


				//Utils::DragFloat("Farplane: ", Application->editorCamera->farPlane, 0.1f, callback, 0.0f);
				//Utils::DragFloat("Farplane: ", Application->editorCamera->farPlane, 0.1f, callback, 0.0f);
				//Utils::DragFloat("Farplane: ", Application->editorCamera->farPlane, 0.1f, callback, 0.0f);

				ImGui::TreePop();
			}
			EditorInspector::Update();
		}
	};
}