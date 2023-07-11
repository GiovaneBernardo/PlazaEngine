#include "Shadows.h"
#include "Engine/Core/PreCompiledHeaders.h"

#include "Engine/Core/Renderer.h"
const glm::vec3 lightDir = glm::normalize(glm::vec3(20.0f, 50, 20.0f));

namespace Engine {
	void ShadowsClass::Init() {
		glGenFramebuffers(1, &shadowsFBO);
		glGenTextures(1, &shadowsDepthMap);
		glBindTexture(GL_TEXTURE_2D, shadowsDepthMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		// attach depth texture as FBO's depth buffer
		glBindFramebuffer(GL_FRAMEBUFFER, shadowsFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowsDepthMap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!";
			throw 0;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void ShadowsClass::GenerateDepthMap() {
		lightDistance = glm::vec3(80.0f, 120.0f, 0.0f); //* glm::vec3(glfwGetTime());
		near_plane = 100.0f, far_plane = 250.5f;
		float size = 100.0f;

		glm::vec3 cameraPosition = glm::vec3(0.0f);//Application->activeCamera->Position;

		lightProjection = glm::ortho(
			cameraPosition.x - size,
			cameraPosition.x + size,
			cameraPosition.y - size,
			cameraPosition.y + size,
			near_plane,
			far_plane
		);

		lightPos = Application->activeCamera->Position + lightDistance;
		cameraPosition = Application->activeCamera->Position;
		lightView = glm::lookAt(lightPos, cameraPosition, glm::vec3(0.0, 1.0, 0.0));
		//lightProjection = glm::ortho(-500.0f + -Application->activeCamera->Position.x, 500.0f + Application->activeCamera->Position.x, -500.0f + -Application->activeCamera->Position.y, 500.0f + Application->activeCamera->Position.y, near_plane, far_plane);
		glViewport(Application->appSizes->sceneStart.x, Application->appSizes->sceneStart.y, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, shadowsFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		//glEnable(GL_CULL_FACE);
		ShadowsClass::RenderScene(*Application->shadowsDepthShader);;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// reset viewport
		glViewport(Application->appSizes->sceneStart.x, Application->appSizes->sceneStart.y, Application->appSizes->sceneSize.x, Application->appSizes->sceneSize.y);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void ShadowsClass::RenderScene(Shader& shader) {
		shader.use();

		glm::vec3 lightPos = Application->Shadows->lightPos;
		glm::mat4 lightProjection, lightView;
		glm::mat4 lightSpaceMatrix;
		lightProjection = Application->Shadows->lightProjection;
		lightView = Application->Shadows->lightView;
		//lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
		lightSpaceMatrix = lightProjection * lightView;
		shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
		for (MeshRenderer* meshRenderer : meshRenderers) {
			//MeshRenderer* meshRenderer = gameObject->GetComponent<MeshRenderer>();
			GameObject* gameObject = meshRenderer->gameObject;
			glm::mat4 modelMatrix = gameObject->transform->modelMatrix;

			shader.setMat4("model", modelMatrix);

			meshRenderer->mesh->Draw(shader);

		}
	}
}