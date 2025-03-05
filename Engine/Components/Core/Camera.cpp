#include "Engine/Core/PreCompiledHeaders.h"
#include "Camera.h"
#include "Engine/Core/Scene.h"

namespace Plaza {
	Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
		: Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		// updateCameraVectors();
	}

	void Camera::Update(Scene* scene) {
		if (!this->isEditorCamera)
			Position = scene->GetComponent<TransformComponent>(this->mUuid)->GetWorldPosition();
		updateCameraVectors(scene);
		UpdateFrustum();
	}

	glm::mat4 Camera::GetProjectionMatrix() {
		return glm::infinitePerspective(
			glm::radians(Zoom), Application::Get()->appSizes->sceneSize.x / Application::Get()->appSizes->sceneSize.y,
			nearPlane);
	}

	glm::mat4 Camera::GetProjectionMatrix(float nearPlaneCustom, float farPlaneCustom) {
		nearPlaneCustom = nearPlaneCustom == NULL ? nearPlane : nearPlaneCustom;
		farPlaneCustom = farPlaneCustom == NULL ? nearPlane : farPlaneCustom;
		return glm::perspective(this->Zoom,
								(Application::Get()->appSizes->sceneSize.x / Application::Get()->appSizes->sceneSize.y),
								nearPlaneCustom, farPlaneCustom);
	}

	glm::mat4 Camera::GetOrthogonalMatrix() {
		return glm::ortho(0.0f, Application::Get()->appSizes->sceneSize.x, 0.0f,
						  Application::Get()->appSizes->sceneSize.y, -1.0f, 1.0f);
	}

	void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime) {
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

	void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
		// FIX: Implement proper way to get the scene
		Scene* scene = Scene::GetActiveScene();
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		Yaw += xoffset;
		Pitch += yoffset;

		// make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch) {
			if (Pitch > 89.0f)
				Pitch = 89.0f;
			if (Pitch < -89.0f)
				Pitch = -89.0f;
		}

		// update Front, Right and Up Vectors using the updated Euler angles
		updateCameraVectors(scene);

		UpdateFrustum();
	}

	void Camera::UpdateFrustum() {
		Camera& cam = *this;
		float aspect = Application::Get()->appSizes->sceneSize.x / Application::Get()->appSizes->sceneSize.y;
		float fovY = glm::radians(Zoom);
		float zNear = nearPlane;
		float zFar = farPlane;
		ViewFrustum newFrustum;
		const float halfVSide = zFar * tanf(fovY * .5f);
		const float halfHSide = halfVSide * aspect;
		const glm::vec3 frontMultFar = zFar * cam.Front;

		newFrustum.nearFace = {cam.Position + zNear * cam.Front, cam.Front};
		newFrustum.farFace = {cam.Position + frontMultFar, -cam.Front};
		newFrustum.rightFace = {cam.Position, glm::cross(frontMultFar - cam.Right * halfHSide, cam.Up)};
		newFrustum.leftFace = {cam.Position, glm::cross(cam.Up, frontMultFar + cam.Right * halfHSide)};
		newFrustum.topFace = {cam.Position, glm::cross(cam.Right, frontMultFar - cam.Up * halfVSide)};
		newFrustum.bottomFace = {cam.Position, glm::cross(frontMultFar + cam.Up * halfVSide, cam.Right)};
		this->frustum = newFrustum;
	}
	bool Camera::IsInsideViewFrustum(glm::vec3 pos) { return true; }

	std::vector<glm::vec4> Camera::getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view) {
		const auto inv = glm::inverse(proj * view);

		std::vector<glm::vec4> frustumCorners;
		for (unsigned int x = 0; x < 2; ++x) {
			for (unsigned int y = 0; y < 2; ++y) {
				for (unsigned int z = 0; z < 2; ++z) {
					const glm::vec4 pt = inv * glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
					frustumCorners.push_back(pt / pt.w);
				}
			}
		}

		return frustumCorners;
	};

	void Camera::updateCameraVectors(Scene* scene) {
		TransformComponent* transform = scene->GetComponent<TransformComponent>(
			this->mUuid); // scene->GetComponent<TransformComponent>(this->mUuid);
		if (this->isEditorCamera) {
			// calculate the new Front vector
			glm::vec3 front;
			front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
			front.y = sin(glm::radians(Pitch));
			front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
			Front = glm::normalize(front);
			Right = glm::normalize(glm::cross(Front, WorldUp));
			Up = glm::normalize(glm::cross(Right, Front));
		}
		else {
			if (this->mUuid && transform) {
				glm::mat4 transformationMatrix = transform->mWorldMatrix;

				/*
				The first column (index [0]) represents the Right vector (X-axis).
				The second column (index [1]) represents the Up vector (Y-axis).
				The third column (index [2]) represents the negative Forward vector (Z-axis).
				*/

				glm::vec3 cubeRight = glm::normalize(glm::vec3(transformationMatrix[0])); // X-axis
				glm::vec3 cubeUp = glm::normalize(glm::vec3(transformationMatrix[1]));	  // Y-axis
				glm::vec3 cubeFront = glm::normalize(glm::vec3(transformationMatrix[2])); // Z-axis

				Right = cubeRight;
				Up = cubeUp;
				Front = cubeFront;
			}
		}
	};

	glm::vec3 Camera::ScreenPositionToRay(const glm::vec2& position, const glm::vec2& size) {
		float x = (2.0f * position.x) / size.x - 1.0f;
		float y = 1.0f - (2.0f * position.y) / size.y;
		float z = 1.0f;

		glm::vec4 rayClip = glm::vec4(x, y, -1.0f, 1.0f);
		glm::vec4 rayEye = glm::inverse(this->GetProjectionMatrix()) * rayClip;
		rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);
		glm::vec3 rayWorld = glm::vec3(glm::inverse(this->GetViewMatrix()) * rayEye);
		return glm::normalize(rayWorld);
	}
} // namespace Plaza
