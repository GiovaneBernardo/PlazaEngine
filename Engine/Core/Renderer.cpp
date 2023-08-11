#include "Engine/Core/PreCompiledHeaders.h"
#include "Renderer.h"
#include "Engine/Components/Core/CoreComponents.h"
#include "Engine/Editor/Editor.h"
#include "Engine/Editor/Outline/Outline.h"
#include "Engine/Core/Skybox.h"
void renderFullscreenQuad() {
	// skybox cube
	glBindVertexArray(Engine::Application->blurVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	//glBindVertexArray(0);

}
int i = 0;
namespace Engine {
	// Render all GameObjects
	void Renderer::Render(Shader& shader) {
		static constexpr tracy::SourceLocationData __tracy_source_location18{ "Render", __FUNCTION__, "C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Core\\Renderer.cpp", (uint32_t)18, 0 }; tracy::ScopedZone ___tracy_scoped_zone(&__tracy_source_location18, true);
		shader.use();

		glm::mat4 projection = Application->activeCamera->GetProjectionMatrix();//glm::perspective(glm::radians(activeCamera->Zoom), (float)(appSizes.sceneSize.x / appSizes.sceneSize.y), 0.3f, 10000.0f);
		glm::mat4 view = Application->activeCamera->GetViewMatrix();
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);
		shader.setVec3("viewPos", Application->activeCamera->Position);
		glActiveTexture(GL_TEXTURE30);
		glBindTexture(GL_TEXTURE_2D_ARRAY, Application->Shadows->shadowsDepthMap);
		shader.setInt("shadowsDepthMap", 30);
		shader.setInt("shadowMap", 30);
		glm::vec3 lightPos = Application->Shadows->lightPos;
		shader.setVec3("lightPos", lightPos);
		shader.setVec3("lightDir", Application->Shadows->lightDir);
		shader.setFloat("farPlane", Application->activeCamera->farPlane);
		shader.setInt("cascadeCount", Application->Shadows->shadowCascadeLevels.size());
		for (size_t i = 0; i < Application->Shadows->shadowCascadeLevels.size(); ++i)
		{
			shader.setFloat("cascadePlaneDistances[" + std::to_string(i) + "]", Application->Shadows->shadowCascadeLevels[i]);
		}


		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);

		Application->activeCamera->UpdateFrustum();
		for (const auto& meshRendererPair : Application->activeScene->meshRendererComponents) {
			MeshRenderer meshRenderer = (meshRendererPair.second);
			if (!meshRenderer.instanced) {
				Transform* transform = &Application->activeScene->transformComponents[meshRendererPair.first];
				if (Application->activeCamera->IsInsideViewFrustum(transform->worldPosition)) {
					glm::mat4 modelMatrix = transform->modelMatrix;
					shader.setMat4("model", modelMatrix);
					meshRenderer.mesh->BindTextures(shader);
					meshRenderer.mesh->Draw(shader);
				}
			}
		}
	}

	void Renderer::RenderInstances(Shader& shader) {
		static constexpr tracy::SourceLocationData __tracy_source_location109{ "Render Instances", __FUNCTION__, "C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Core\\Renderer.cpp", (uint32_t)109, 0 }; tracy::ScopedZone ___tracy_scoped_zone(&__tracy_source_location109, true);
		for (shared_ptr<Mesh> mesh : Application->activeScene->meshes) {
			if (mesh->instanceModelMatrices.size() > 0) {
				mesh->DrawInstanced(shader);
			}
		}
	}

	void Renderer::RenderInstancesShadowMap(Shader& shader) {
		shader.use();
		static constexpr tracy::SourceLocationData __tracy_source_location109{ "Render Instances", __FUNCTION__, "C:\\Users\\Giovane\\Desktop\\Workspace 2023\\OpenGL\\OpenGLEngine\\Engine\\Core\\Renderer.cpp", (uint32_t)109, 0 }; tracy::ScopedZone ___tracy_scoped_zone(&__tracy_source_location109, true);
		for (shared_ptr<Mesh> mesh : Application->activeScene->meshes) {
			if (mesh->instanceModelMatrices.size() > 0) {
				mesh->DrawInstancedToShadowMap(shader);
			}
		}
	}

	void Renderer::BlurBuffer()
	{

	}



	void Renderer::RenderOutline(Shader outlineShader) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, Application->selectedFramebuffer);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glStencilMask(0xFF);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		//DetectEdges();

		glViewport(Application->appSizes->sceneStart.x, Application->appSizes->sceneStart.y, Application->appSizes->sceneSize.x, Application->appSizes->sceneSize.y);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// Enable stencil test
		glEnable(GL_STENCIL_TEST);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);

		Application->singleColorShader->use();
		glm::mat4 projection = Application->activeCamera->GetProjectionMatrix();//glm::perspective(glm::radians(activeCamera->Zoom), (float)(appSizes.sceneSize.x / appSizes.sceneSize.y), 0.3f, 10000.0f);
		glm::mat4 view = Application->activeCamera->GetViewMatrix();
		glm::mat4 modelMatrix = Editor::selectedGameObject->GetComponent<Transform>()->modelMatrix;
		Application->singleColorShader->setMat4("projection", projection);
		Application->singleColorShader->setMat4("view", view);
		Application->singleColorShader->setFloat("objectID", Editor::selectedGameObject->uuid);
		glStencilFunc(GL_ALWAYS, 0, 0x00);
		glStencilMask(0x00);
		glDisable(GL_DEPTH_TEST);

		glStencilFunc(GL_EQUAL, 1, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		Editor::Outline::BlurBuffer();
		glDisable(GL_STENCIL_TEST);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0x00);
		glDisable(GL_DEPTH_TEST);
	}




}