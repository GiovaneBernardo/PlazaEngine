#pragma once

#include "Engine/Vendor/glm/glm.hpp"
#include "Engine/Vendor/glad/glad.h"
#include "Engine/Vendor/glm/gtc/matrix_transform.hpp"
#include <vector>

#include "Engine/Application/Application.h"
#include "Engine/Application/EntryPoint.h"
#include "Engine/Application/ApplicationSizes.h"


// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 90.0f;

using namespace Engine;

struct ViewFrustum {
	glm::mat4 viewProjectionMatrix;
	glm::vec4 leftPlane;
	glm::vec4 rightPlane;
	glm::vec4 bottomPlane;
	glm::vec4 topPlane;
	glm::vec4 nearPlaneFrustum;
	glm::vec4 farPlaneFrustum;
};


namespace Engine {
	class Camera {
	public:
		enum Camera_Movement {
			FORWARD,
			BACKWARD,
			LEFT,
			RIGHT,
			UP,
			DOWN,
			ROLLLEFT,
			ROLLRIGHT
		};

		ViewFrustum frustum = ViewFrustum();


		// camera Attributes
		glm::vec3 Position;
		glm::vec3 Front;
		glm::vec3 Up;
		glm::vec3 Right;
		glm::vec3 WorldUp;
		// euler Angles
		float Yaw;
		float Pitch;
		// camera options
		float MovementSpeed;
		float MovementSpeedTemporaryBoost;
		float MouseSensitivity;
		float Zoom;
		float nearPlane = 0.1f;
		float farPlane = 15000.0f;

		// constructor with vectors
		Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);
		// constructor with scalar values
		Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
		{
			Position = glm::vec3(posX, posY, posZ);
			WorldUp = glm::vec3(upX, upY, upZ);
			Yaw = yaw;
			Pitch = pitch;
			updateCameraVectors();
		}
		glm::mat4 GetProjectionMatrix();
		glm::mat4 GetProjectionMatrix(float nearPlaneCustom, float farPlaneCustom);

		void updateCameraAppSizes();

		// returns the view matrix calculated using Euler Angles and the LookAt Matrix
		glm::mat4 GetViewMatrix()
		{
			return glm::lookAt(Position, Position + Front, Up);
		}


		void UpdateFrustum();

		bool IsInsideViewFrustum(glm::vec3 pos);

		// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
		void ProcessKeyboard(Camera_Movement direction, float deltaTime);

		// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
		void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

		// processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
		void ProcessMouseScroll(float yoffset)
		{
			Zoom -= (float)yoffset;
			if (Zoom < 1.0f)
				Zoom = 1.0f;
			if (Zoom > 90.0f)
				Zoom = 90.0f;

			UpdateFrustum();
		}

	private:
		// calculates the front vector from the Camera's (updated) Euler Angles
		void updateCameraVectors()
		{
			// calculate the new Front vector
			glm::vec3 front;
			front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
			front.y = sin(glm::radians(Pitch));
			front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
			Front = glm::normalize(front);
			// also re-calculate the Right and Up vector
			Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
			Up = glm::normalize(glm::cross(Right, Front));
		}
	};
}