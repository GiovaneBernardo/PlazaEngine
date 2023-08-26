#include "EditorCamera.h"
#include "Engine/Core/PreCompiledHeaders.h"
namespace Plaza {
	EditorCamera::EditorCamera(glm::vec3 position, glm::vec3 up, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}

	glm::mat4 EditorCamera::GetProjectionMatrix() {
		return glm::perspective(glm::radians(this->Zoom), (Application->appSizes->sceneSize.x / Application->appSizes->sceneSize.y), nearPlane, farPlane);
	}

	glm::mat4 EditorCamera::GetProjectionMatrix(float nearPlaneCustom, float farPlaneCustom) {
		nearPlaneCustom = nearPlaneCustom == NULL ? nearPlane : nearPlaneCustom;
		farPlaneCustom = farPlaneCustom == NULL ? nearPlane : farPlaneCustom;
		return glm::perspective(this->Zoom, (Application->appSizes->sceneSize.x / Application->appSizes->sceneSize.y), nearPlaneCustom, farPlaneCustom);
	}

	void EditorCamera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
	{
		float velocity = MovementSpeed * MovementSpeedTemporaryBoost * deltaTime;
		if (direction == FORWARD)
			Position += Front * velocity;
		if (direction == BACKWARD)
			Position -= Front * velocity;
		if (direction == LEFT)
			Position -= Right * velocity;
		if (direction == RIGHT)
			Position += Right * velocity;
		if (direction == UP)
			Position -= Up * velocity;
		if (direction == DOWN)
			Position += Up * velocity;

	}

	void EditorCamera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
	{
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		Yaw += xoffset;
		Pitch += yoffset;

		// make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (Pitch > 89.0f)
				Pitch = 89.0f;
			if (Pitch < -89.0f)
				Pitch = -89.0f;
		}

		// update Front, Right and Up Vectors using the updated Euler angles
		updateCameraVectors();

		UpdateFrustum();
	}

	void EditorCamera::UpdateFrustum() {
		glm::mat4 viewProjectionMatrix = GetProjectionMatrix() * GetViewMatrix();
		glm::vec4 leftPlane = viewProjectionMatrix[3] + viewProjectionMatrix[0];
		glm::vec4 rightPlane = viewProjectionMatrix[3] - viewProjectionMatrix[0];
		glm::vec4 bottomPlane = viewProjectionMatrix[3] + viewProjectionMatrix[1];
		glm::vec4 topPlane = viewProjectionMatrix[3] - viewProjectionMatrix[1];
		glm::vec4 nearPlaneFrustum = viewProjectionMatrix[3] + viewProjectionMatrix[2];
		glm::vec4 farPlaneFrustum = viewProjectionMatrix[3] - viewProjectionMatrix[2];

		// Normalize the planes
		frustum.leftPlane = glm::normalize(leftPlane);
		frustum.rightPlane = glm::normalize(rightPlane);
		frustum.bottomPlane = glm::normalize(bottomPlane);
		frustum.topPlane = glm::normalize(topPlane);
		frustum.nearPlaneFrustum = glm::normalize(nearPlaneFrustum);
		frustum.farPlaneFrustum = glm::normalize(farPlaneFrustum);
	}
	bool EditorCamera::IsInsideViewFrustum(glm::vec3 pos) {
		glm::vec4 objectPos = glm::vec4(pos, 1.0f);
		return true; /// TEMPORARILY DISABLED FRUSTUM CULLING -------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		return
			(glm::dot(objectPos, frustum.leftPlane) > 0.0f) &&
			(glm::dot(objectPos, frustum.rightPlane) > 0.0f) &&
			(glm::dot(objectPos, frustum.bottomPlane) > 0.0f) &&
			(glm::dot(objectPos, frustum.topPlane) > 0.0f) &&
			(glm::dot(objectPos, frustum.nearPlaneFrustum) > 0.0f) &&
			(glm::dot(objectPos, frustum.farPlaneFrustum) > 0.0f);
	}

	std::vector<glm::vec4> EditorCamera::getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view)
	{
		const auto inv = glm::inverse(proj * view);

		std::vector<glm::vec4> frustumCorners;
		for (unsigned int x = 0; x < 2; ++x)
		{
			for (unsigned int y = 0; y < 2; ++y)
			{
				for (unsigned int z = 0; z < 2; ++z)
				{
					const glm::vec4 pt =
						inv * glm::vec4(
							2.0f * x - 1.0f,
							2.0f * y - 1.0f,
							2.0f * z - 1.0f,
							1.0f);
					frustumCorners.push_back(pt / pt.w);
				}
			}
		}

		return frustumCorners;
	}
}