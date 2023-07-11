#include "Engine/Core/PreCompiledHeaders.h"
#include "Outline.h"
#include "Engine/Editor/Editor.h"
using namespace Engine;
using namespace Engine::Editor;
//using namespace Engine::Editor;
//void Outline::CombineOutlineToScene() {
//
//}

unsigned int Engine::Editor::Outline::quadVAO = 0;
unsigned int Engine::Editor::Outline::quadVBO = 0;
Shader* Engine::Editor::Outline::combiningShader = nullptr;
Shader* Engine::Editor::Outline::blurringShader = nullptr;

//using namespace Editor;

void Engine::Editor::Outline::BlurBuffer() {
	// Enable stencil testing
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	// Clear the stencil buffer to 0
	glClear(GL_STENCIL_BUFFER_BIT);

	// Set the stencil mask to 0xFF
	glStencilMask(0xFF);

	// Render the cubes to update the stencil buffer
	glStencilFunc(GL_ALWAYS, 0, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	// Bind the blur framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, Application->blurFramebuffer);
	glViewport(Application->appSizes->sceneStart.x, Application->appSizes->sceneStart.y, Application->appSizes->sceneSize.x, Application->appSizes->sceneSize.y);

	// Clear the buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Enable stencil test
	glEnable(GL_DEPTH_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 0, 0xFF);
	glStencilMask(0xFF);
	// Render the cubes here
	Application->singleColorShader->use();
	glm::mat4 projection = Application->activeCamera->GetProjectionMatrix();//glm::perspective(glm::radians(activeCamera->Zoom), (float)(appSizes.sceneSize.x / appSizes.sceneSize.y), 0.3f, 10000.0f);
	glm::mat4 view = Application->activeCamera->GetViewMatrix();
	glm::mat4 modelMatrix = Editor::selectedGameObject->transform->GetTransform();
	Application->singleColorShader->setMat4("projection", projection);
	Application->singleColorShader->setMat4("view", view);
	Application->singleColorShader->setMat4("model", modelMatrix);

	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 1, 0xFF); // all fragments should pass the stencil test
	glStencilMask(0xFF); // enable writing to the stencil buffer
	Outline::RenderSelectedObjects(Editor::selectedGameObject, *Application->singleColorShader);

	// Disable stencil test for rendering the blur buffer
	//glDisable(GL_STENCIL_TEST);

	// Render the blur buffer
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Application->selectedColorBuffer);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0x00); // disable writing to the stencil buffer
	glDisable(GL_DEPTH_TEST);

	// Unbind textures and framebuffer
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, Application->edgeDetectionFramebuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glViewport(Application->appSizes->sceneStart.x, Application->appSizes->sceneStart.y, Application->appSizes->sceneSize.x, Application->appSizes->sceneSize.y);
	// Render the blur buffer
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0x00); // disable writing to the stencil buffer
	glDisable(GL_DEPTH_TEST);
	Application->singleColorShader->use();
	Application->singleColorShader->setMat4("projection", projection);
	Application->singleColorShader->setMat4("view", view);
	Application->singleColorShader->setMat4("model", modelMatrix);
	Application->singleColorShader->setFloat("objectID", Editor::selectedGameObject->id);
	//Renderer::OutlineDraw(Editor::selectedGameObject, *Application::singleColorShader, glm::vec3(1.0f));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Application->blurColorBuffer);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, Application->blurDepthStencilRBO);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, Application->selectedDepthStencilRBO);
	Application->outlineBlurShader->use();
	Application->outlineBlurShader->setInt("sceneBuffer", 0);
	Application->outlineBlurShader->setInt("depthStencilTexture", 1);
	Application->outlineBlurShader->setInt("depthStencilTexture2", 2);

	glBindVertexArray(Application->blurVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	/*
	Application::shader->use();
	Application::shader->setMat4("projection", projection);
	Application::shader->setMat4("view", view);
	Application::shader->setMat4("model", modelMatrix);
	Application::shader->setFloat("objectID", Editor::selectedGameObject->id);
	Renderer::OutlineDraw(Editor::selectedGameObject, *Application::shader, glm::vec3(0.0f));
	*/

	// Unbind textures and framebuffer
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Draw the Outline
namespace Engine::Editor {
	void Outline::RenderSelectedObjects(GameObject* gameObject, Shader shader) {
		MeshRenderer* mr = gameObject->GetComponent<MeshRenderer>();
		if (mr) {
			glm::mat4 modelMatrix = gameObject->transform->modelMatrix;
			shader.setMat4("model", modelMatrix);
			mr->mesh->Draw(shader);
		}
		for (GameObject* child : gameObject->children) {
			Outline::RenderSelectedObjects(child, shader);
		}
	}
}
