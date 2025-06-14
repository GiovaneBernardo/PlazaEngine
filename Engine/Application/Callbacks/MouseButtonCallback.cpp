#include "Engine/Core/PreCompiledHeaders.h"
#include "CallbacksHeader.h"
#include "Engine/Application/ApplicationSizes.h"
#include "Editor/GUI/guiMain.h"
#include "Engine/Components/Drawing/UI/GuiButton.h"
#include "Engine/Core/Scene.h"
#include "Engine/ECS/ECSManager.h"

namespace Plaza {
	void Callbacks::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
		// FIX: Implement proper way to get scene instead of active scene
		Scene* scene = Scene::GetActiveScene();
		for (CallbackFunction callbackFunction : sOnMouseFunctions) {
			bool isLayerFocused = Editor::Gui::sFocusedLayer == callbackFunction.layerToExecute;
			if (isLayerFocused)
				callbackFunction.function;
		}

		for (auto& tool : Editor::Gui::sEditorTools) {
			tool.second->OnMouseClick(button, action, mods);
		}

		if (scene && scene->mRunning && Application::Get()->focusedMenu == "Scene") {
			for (uint64_t uuid : SceneView<GuiComponent>(scene)) {
				GuiComponent* component = scene->GetComponent<GuiComponent>(uuid);
				for (auto& [itemKey, itemValue] : component->mGuiItems) {
					if (itemValue->mGuiType == GuiType::PL_GUI_BUTTON) {
						double x;
						double y;
						glfwGetCursorPos(window, &x, &y);
						if (static_cast<GuiButton*>(itemValue.get())->MouseIsInsideButton(glm::vec2(x, y)))
							static_cast<GuiButton*>(itemValue.get())->CallScriptsCallback();
					}
				}
			}
		}

		// if (Application::Get()->hoveredMenu != "File Explorer" && Application::Get()->hoveredMenu != "Inspector")
		//	Editor::selectedFiles.clear();
		if (Application::Get()->hoveredMenu == "Editor" && Application::Get()->focusedMenu != "Scene") {
			ApplicationSizes& appSizes = *Application::Get()->appSizes;
			ApplicationSizes& lastAppSizes = *Application::Get()->lastAppSizes;

			// Position entity to where the mouse is looking if pressed ALT + mouse middle button
			if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS && Editor::selectedGameObject) {
				float xposGame = Callbacks::lastX - Application::Get()->appSizes->hierarchySize.x;
				float yposGame = Callbacks::lastY - Application::Get()->appSizes->sceneImageStart.y;

				// Get depth
				float depth = VulkanRenderer::GetRenderer()
					->mRenderGraph->GetTexture<VulkanTexture>("SceneDepth")
					->ReadTexture(glm::vec2(xposGame, yposGame), sizeof(float) + sizeof(uint8_t), 1,
						  VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, true).x;

				// Get world position
				glm::vec3 worldPosition = Renderer::ReconstructWorldPositionFromDepth(
					glm::vec2(xposGame, yposGame), Application::Get()->appSizes->sceneSize,
					depth, Application::Get()->activeCamera);
				ECS::TransformSystem::SetWorldPosition(*scene->GetComponent<TransformComponent>(Editor::selectedGameObject->uuid), scene, worldPosition);
			}

			if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
				// Pressing right button
				rightClickPressed = true;
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}
			else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
				// No more pressing right button
				rightClickPressed = false;
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}

#pragma region Picking
			if (Application::Get()->focusedMenu == "Editor") {
				float xposGame = lastX - appSizes.hierarchySize.x;
				float yposGame = lastY - appSizes.sceneImageStart.y;
				// yposGame = appSizes.sceneSize.y - yposGame;
				yposGame = appSizes.sceneSize.y - (lastY - appSizes.sceneImageStart.y);
				uint64_t clickUuid = 0;

				if (Editor::selectedGameObject &&
					scene->HasComponent<TransformComponent>(Editor::selectedGameObject->uuid) &&
					Editor::selectedGameObject->parentUuid != 0)
					ImGuizmoHelper::IsDrawing = true;
				else
					ImGuizmoHelper::IsDrawing = false;

				bool pressingLeftClick = button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS;
				bool drawingButMouseNotOverGizmo = !ImGuizmo::IsOver() && ImGuizmoHelper::IsDrawing;
				if (pressingLeftClick && (!ImGuizmoHelper::IsDrawing || drawingButMouseNotOverGizmo)) {
					//	Application::Get()->pickingTexture->GenerateTexture();
					//    	clickUuid = Application::Get()->pickingTexture->readPixel(xposGame, yposGame);
					clickUuid =
						Application::Get()->mRenderer->mPicking->DrawAndRead(scene, glm::vec2(xposGame, yposGame));

					Entity* entity = Scene::GetActiveScene()->GetEntity(clickUuid);
					if (entity) {
						// Object with the specified name found
						Plaza::Editor::Gui::changeSelectedGameObject(entity);
					}
					else
						Plaza::Editor::Gui::changeSelectedGameObject(nullptr);
				}
			}
#pragma endregion Picking
		}
		else if (rightClickPressed == true) {
			rightClickPressed = false;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}
} // namespace Plaza
