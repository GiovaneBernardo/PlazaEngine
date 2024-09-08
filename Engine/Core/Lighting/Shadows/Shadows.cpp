#include "Engine/Core/PreCompiledHeaders.h"
#include "Shadows.h"

#include "Engine/Core/Renderer/Renderer.h"

#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <execution>

const glm::vec3 lightDir = glm::normalize(glm::vec3(20.0f, 50, 20.0f));

namespace Plaza {


	void ShadowsClass::Init() {
		glGenFramebuffers(1, &shadowsFBO);
		glGenTextures(1, &shadowsDepthMap);
		glBindTexture(GL_TEXTURE_2D_ARRAY, shadowsDepthMap);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		float mult = 1.0f;
		shadowCascadeLevels = vector{ Application::Get()->activeCamera->farPlane / (9000.0f * mult), Application::Get()->activeCamera->farPlane / (3000.0f * mult), Application::Get()->activeCamera->farPlane / (1000.0f * mult), Application::Get()->activeCamera->farPlane / (500.0f * mult), Application::Get()->activeCamera->farPlane / (100.0f * mult), Application::Get()->activeCamera->farPlane / (35.0f * mult),Application::Get()->activeCamera->farPlane / (10.0f * mult), Application::Get()->activeCamera->farPlane / (2.0f * mult), Application::Get()->activeCamera->farPlane / (1.0f * mult) };
		glTexImage3D(
			GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, depthMapResolution, depthMapResolution, int(shadowCascadeLevels.size()) + 1,
			0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);
		// attach depth texture as FBO's depth buffer
		glBindFramebuffer(GL_FRAMEBUFFER, shadowsFBO);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowsDepthMap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!";
			throw 0;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// configure UBO
		// --------------------
		glGenBuffers(1, &matricesUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, matricesUBO);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4x4) * maximumCascades, nullptr, GL_STATIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, matricesUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void ShadowsClass::GenerateDepthMap() {
		PLAZA_PROFILE_SECTION("Generate Depth Map");
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// 0. UBO setup
		Application::Get()->shadowsDepthShader->use();
		const auto lightMatrices = ShadowsClass::GetLightSpaceMatrices();
		glBindBuffer(GL_UNIFORM_BUFFER, matricesUBO);
		for (size_t i = 0; i < lightMatrices.size(); ++i)
		{
			glBufferSubData(GL_UNIFORM_BUFFER, i * sizeof(glm::mat4x4), sizeof(glm::mat4x4), &lightMatrices[i]);
		}
		glBindBuffer(GL_UNIFORM_BUFFER, 0);


		Application::Get()->shadowsDepthShader->use();
		lightPos = Application::Get()->activeCamera->Position + lightDistance;
		glBindFramebuffer(GL_FRAMEBUFFER, shadowsFBO);
		glViewport(Application::Get()->appSizes->sceneStart.x, Application::Get()->appSizes->sceneStart.y, depthMapResolution, depthMapResolution);
		glClear(GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_FRONT);
		ShadowsClass::RenderScene(*Application::Get()->shadowsDepthShader);
		Application::Get()->mRenderer->RenderShadowMap(*Application::Get()->shadowsDepthShader);
		glCullFace(GL_BACK);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// reset viewport
		glViewport(Application::Get()->appSizes->sceneStart.x, Application::Get()->appSizes->sceneStart.y, Application::Get()->appSizes->sceneSize.x, Application::Get()->appSizes->sceneSize.y);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void ShadowsClass::RenderScene(Shader& shader) {
		shader.use();
		for (const auto& [key, value] : Application::Get()->activeScene->meshRendererComponents) {
			const MeshRenderer& meshRenderer = value;
			auto transformIt = Application::Get()->activeScene->transformComponents.find(key);
			if (transformIt != Application::Get()->activeScene->transformComponents.end()) {
				const Transform& transform = transformIt->second;
				if (Application::Get()->activeCamera->IsInsideViewFrustum(transform.worldPosition)) {
					//Application::Get()->activeScene->entities[transform->uuid].GetComponent<Transform>()->UpdateObjectTransform(&Application::Get()->activeScene->entities[meshRendererPair.first]);
					glm::mat4 modelMatrix = transform.modelMatrix;
					if (meshRenderer.instanced&& meshRenderer.renderGroup && meshRenderer.renderGroup->mesh) {
						meshRenderer.renderGroup->AddInstance(modelMatrix);
					}
					else if(meshRenderer.renderGroup) {
						shader.setMat4("model", modelMatrix);
						//[] meshRenderer.renderGroup->Draw(shader);
					}

				}
			}
		}
	}
	using std::min;
	glm::mat4 getLightSpaceMatrix(const float nearPlane, const float farPlane)
	{
		const auto proj = glm::perspective(
			glm::radians(Application::Get()->activeCamera->Zoom), (float)Application::Get()->appSizes->sceneSize.x / (float)Application::Get()->appSizes->sceneSize.y, nearPlane,
			farPlane);
		const auto corners = Application::Get()->activeCamera->getFrustumCornersWorldSpace(proj, Application::Get()->activeCamera->GetViewMatrix());

		glm::vec3 center = glm::vec3(0, 0, 0);
		for (const auto& v : corners)
		{
			center += glm::vec3(v);
		}
		center /= corners.size();
		const float LARGE_CONSTANT = std::abs(std::numeric_limits<float>::min());
		const auto lightView = glm::lookAt(center + glm::radians(Application::Get()->Shadows->lightDir), center, glm::vec3(0.0f, 1.0f, 0.0f));
		float minX = std::numeric_limits<float>::max();
		float maxX = std::numeric_limits<float>::lowest();
		float minY = std::numeric_limits<float>::max();
		float maxY = std::numeric_limits<float>::lowest();
		float minZ = std::numeric_limits<float>::max();
		float maxZ = std::numeric_limits<float>::lowest();
		for (const auto& v : corners)
		{
			const auto trf = lightView * v;
			minX = std::min(minX, trf.x);
			maxX = std::max(maxX, trf.x);
			minY = std::min(minY, trf.y);
			maxY = std::max(maxY, trf.y);
			minZ = std::min(minZ, trf.z);
			maxZ = std::max(maxZ, trf.z);
		}

		// Tune this parameter according to the scene

		constexpr float zMult = 22.0f;
		if (minZ < 0)
		{
			minZ *= zMult;
		}
		else
		{
			minZ /= zMult;
		}
		if (maxZ < 0)
		{
			maxZ /= zMult;
		}
		else
		{
			maxZ *= zMult;
		}
		const glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);

		return lightProjection * lightView;
	}

	std::vector<glm::mat4> ShadowsClass::GetLightSpaceMatrices()
	{
		std::vector<glm::mat4> ret;
		for (size_t i = 0; i < shadowCascadeLevels.size() + 1; ++i)
		{
			if (i == 0)
			{
				ret.push_back(getLightSpaceMatrix(Application::Get()->editorCamera->nearPlane - 1.0f, shadowCascadeLevels[i]));
			}
			else if (i < shadowCascadeLevels.size())
			{
				ret.push_back(getLightSpaceMatrix(shadowCascadeLevels[i - 1] - 1.0f, shadowCascadeLevels[i]));
			}
		}
		return ret;
	}
}