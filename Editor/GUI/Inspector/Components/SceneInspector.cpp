#pragma once
#include "Engine/Core/Scene.h"
#include "ComponentsInspector.h"
#include "Editor/GUI/Utils/DataVisualizer.h"

#include "Engine/Application/EditorCamera.h"
#include "Editor/GUI/EditorInspector.h"

bool visualizingNormals = false;
namespace Plaza::Editor {
	void callback(float) {
		Application::Get()->editorCamera->Update(Scene::GetActiveScene());
		// Application::Get()->editorCamera = new Plaza::Camera(*Application::Get()->editorCamera);
		// Application::Get()->activeCamera = Application::Get()->editorCamera;
	}
	void callbacke(glm::vec3) {}
	void callbacke2(glm::vec3) { Application::Get()->editorCamera->Update(Scene::GetActiveScene()); }
	static bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation,
								   glm::vec3& scale) {
		// From glm::decompose in matrix_decompose.inl

		using namespace glm;
		using T = float;

		mat4 LocalMatrix(transform);

		// Normalize the matrix.
		if (epsilonEqual(LocalMatrix[3][3], static_cast<float>(0), epsilon<T>()))
			return false;

		// First, isolate perspective.  This is the messiest.
		if (epsilonNotEqual(LocalMatrix[0][3], static_cast<T>(0), epsilon<T>()) ||
			epsilonNotEqual(LocalMatrix[1][3], static_cast<T>(0), epsilon<T>()) ||
			epsilonNotEqual(LocalMatrix[2][3], static_cast<T>(0), epsilon<T>())) {
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
		glm::mat4 projection = Application::Get()->activeCamera->GetProjectionMatrix();
		glm::mat4 view = Application::Get()->activeCamera->GetViewMatrix();
		glm::mat4 rotationMatrix = glm::eulerAngleXYZ(vector.x, vector.y, vector.z);
		glm::mat4 gizmoTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)) *
								   glm::toMat4(glm::quat(vector)) *
								   glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));

		// glm::mat4 gizmoTransform = glm::lookAt(glm::vec3(0), vector, glm::vec3(0, 1, 0));

		ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection), ImGuizmo::OPERATION::ROTATE,
							 ImGuizmo::MODE::WORLD, glm::value_ptr(gizmoTransform));

		if (ImGuizmo::IsUsing()) {
			glm::vec3 position, rotation, scale;
			DecomposeTransform(gizmoTransform, position, rotation, scale);
			return rotation;
		}
		return vector;
	}

	void ComponentsInspector::SceneInspector(Scene* scene, Entity* entity) {
		RendererSettings::LightingSettings& shadows = VulkanRenderer::GetRenderer()->mRendererSettings.mLightingSettings; // Application::Get()->mRenderer->mShadows;
		/* Draw Gizmo for rotating Sun */
		shadows.mLightDirection = DrawGizmo(shadows.mLightDirection);

		/* Shadows */
		if (ImGui::TreeNodeEx("Shadows", ImGuiTreeNodeFlags_DefaultOpen)) {
			glm::vec3& lightDir = shadows.mLightDirection;
			Utils::DragFloat3("Light Direction: ", lightDir, 0.1f, callbacke, -360.0f, 360.0f);
			ImGui::DragInt("Depth Map Resolution: ", reinterpret_cast<int*>(&shadows.mShadowResolution), 1024, 0,
						   *"%d");
			ImGui::DragFloat3("Light Distance: ", &shadows.mLightDirection.x);
			int cascadeCount = shadows.mCascadeCount;
			ImGui::DragInt("Cascade Count: ", &cascadeCount);
			shadows.mCascadeCount = cascadeCount;
			ImGui::DragFloat("Lambda: ", &shadows.mLambda);

			ImGui::TreePop();
		}

		/* Rendering */
		if (ImGui::TreeNodeEx("Rendering", ImGuiTreeNodeFlags_DefaultOpen)) {
			if (ImGui::Checkbox("Visualize Normals", &visualizingNormals)) {
				if (visualizingNormals) {
				}
				else {
				}
			}

			if (ImGui::Checkbox("Visualize Wireframe", &VulkanRenderer::GetRenderer()->mShowWireframe)) {
				Application::Get()->mThreadsManager->mFrameStartThread->AddToQueue([](){
					bool value = VulkanRenderer::GetRenderer()->mShowWireframe;
					VulkanRenderer::GetRenderer()->mRenderGraph->GetRenderPass("Deferred Geometry Pass")
						->mPipelines[0]->mCreateInfo = pl::pipelineCreateInfo(
							"MainShaders", PL_RENDER_PASS_INDIRECT_BUFFER,
							{pl::pipelineShaderStageCreateInfo(
								 PL_STAGE_VERTEX,
								 FilesManager::sEngineFolder.string() + "/Shaders/Vulkan/deferred/geometryPass.vert", "main"),
							 pl::pipelineShaderStageCreateInfo(PL_STAGE_FRAGMENT,
															   FilesManager::sEngineFolder.string() +
																   "/Shaders/Vulkan/deferred/geometryPass.frag",
															   "main")},
							VulkanRenderer::GetRenderer()->mRenderGraph->VertexGetBindingDescription(), VulkanRenderer::GetRenderer()->mRenderGraph->VertexGetAttributeDescriptions(), PL_TOPOLOGY_TRIANGLE_LIST,
							false,
							pl::pipelineRasterizationStateCreateInfo(
								false, false,
								VulkanRenderer::GetRenderer()->mShowWireframe ? PL_POLYGON_MODE_LINE
																			  : PL_POLYGON_MODE_FILL,
								1.0f, false, 0.0f, 0.0f, 0.0f, PL_CULL_MODE_BACK, PL_FRONT_FACE_COUNTER_CLOCKWISE),
							pl::pipelineColorBlendStateCreateInfo({pl::pipelineColorBlendAttachmentState(true),
																   pl::pipelineColorBlendAttachmentState(true),
																   pl::pipelineColorBlendAttachmentState(true)}),
							pl::pipelineDepthStencilStateCreateInfo(true, true, PL_COMPARE_OP_LESS_OR_EQUAL),
							pl::pipelineViewportStateCreateInfo(1, 1),
							pl::pipelineMultisampleStateCreateInfo(PL_SAMPLE_COUNT_1_BIT, 0),
							{PL_DYNAMIC_STATE_VIEWPORT, PL_DYNAMIC_STATE_SCISSOR}, {});
					VulkanRenderer::GetRenderer()->mRenderGraph->GetRenderPass("Deferred Geometry Pass")->ReCompileShaders(true);
					//VulkanRenderer::GetRenderer()->Destroy();
					//Application::Get()->mRenderer = new VulkanRenderer();
					//Application::Get()->mRenderer->api = Application::Get()->mEditor->mSettings.mDefaultRendererAPI;
					//VulkanRenderer::GetRenderer()->mShowWireframe = value;
					//Application::Get()->mRenderer->Init();
				});
			}
			ImGui::TreePop();
		}

		/* Editor Camera */
		if (ImGui::TreeNodeEx("Editor Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
			Utils::DragFloat("Zoom: ", Application::Get()->editorCamera->Zoom, 0.1f, callback, 0.0f);
			Utils::DragFloat("Movement Speed: ", Application::Get()->editorCamera->MovementSpeed, 0.1f, callback, 0.0f);
			Utils::DragFloat("Camera Sensitivity: ", Application::Get()->editorCamera->MouseSensitivity, 0.1f, callback,
							 0.0f);
			Utils::DragFloat("Nearplane: ", Application::Get()->editorCamera->nearPlane, 0.1f, callback, -10000.0f);
			Utils::DragFloat("Farplane: ", Application::Get()->editorCamera->farPlane, 0.1f, callback, 0.0f);

			Utils::DragFloat("Pitch: ", Application::Get()->editorCamera->Pitch, 0.1f, callback);
			Utils::DragFloat3("Front: ", Application::Get()->editorCamera->Front, 0.1f, callbacke);
			Utils::DragFloat3("Right: ", Application::Get()->editorCamera->Right, 0.1f, callbacke2);
			Utils::DragFloat3("Up: ", Application::Get()->editorCamera->Up, 0.1f, callbacke2);
			Utils::DragFloat3("WorldUp: ", Application::Get()->editorCamera->WorldUp, 0.1f, callbacke2);
			Utils::DragFloat("Yaw: ", Application::Get()->editorCamera->Yaw, 0.1f, callback);

			Utils::DragFloat("Aspect Ratio: ", Application::Get()->editorCamera->aspectRatio, 0.1f, callback);
			Utils::DragFloat("X: ", Application::Get()->appSizes->sceneSize.x, 0.1f, callback);
			Utils::DragFloat("Y: ", Application::Get()->appSizes->sceneSize.y, 0.1f, callback);

			// Utils::DragFloat("Farplane: ", Application::Get()->editorCamera->farPlane, 0.1f, callback, 0.0f);
			// Utils::DragFloat("Farplane: ", Application::Get()->editorCamera->farPlane, 0.1f, callback, 0.0f);
			// Utils::DragFloat("Farplane: ", Application::Get()->editorCamera->farPlane, 0.1f, callback, 0.0f);

			ImGui::TreePop();
		}
		EditorInspector::Update();
	}
} // namespace Plaza::Editor
