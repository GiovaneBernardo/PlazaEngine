#pragma once
#include "Engine/Components/Component.h"

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 60.0f;

#include "Engine/Components/Core/Transform.h"
#include "Engine/Core/Engine.h"
#include "Engine/Core/Renderer/Viewport.h"

namespace Plaza {
	class Scene;
	struct PLAZA_API Plane {
		glm::vec3 normal = {0.f, 1.f, 0.f}; // unit vector
		float distance = 0.f;				// Distance with origin

		Plane() = default;

		Plane(const glm::vec3& p1, const glm::vec3& norm)
			: normal(glm::normalize(norm)), distance(glm::dot(normal, p1)) {}

		float getSignedDistanceToPlane(const glm::vec3& point) const { return glm::dot(normal, point) - distance; }
	};
	struct PLAZA_API ViewFrustum {
		Plane topFace;
		Plane bottomFace;

		Plane rightFace;
		Plane leftFace;

		Plane farFace;
		Plane nearFace;
	};
	class PLAZA_API Camera : public Component {
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

		bool mainCamera = false;
		bool isEditorCamera = false;
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
		float nearPlane = 0.01f;
		float farPlane = 15000.0f;
		float aspectRatio = 1;

		Camera(const Camera& other) = default;

		// constructor with vectors
		Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
			   float yaw = YAW, float pitch = PITCH);
		// constructor with scalar values
		Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
			: Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
			Position = glm::vec3(posX, posY, posZ);
			WorldUp = glm::vec3(upX, upY, upZ);
			Yaw = yaw;
			Pitch = pitch;
			// updateCameraVectors();
		}

		void Update(Scene* scene);

		glm::mat4 GetProjectionMatrix();
		glm::mat4 GetProjectionMatrix(float nearPlaneCustom, float farPlaneCustom);
		glm::mat4 GetOrthogonalMatrix();

		// returns the view matrix calculated using Euler Angles and the LookAt Matrix
		glm::mat4 GetViewMatrix() { return glm::lookAt(Position, Position + Front, Up); }

		void UpdateFrustum();

		void UpdateCameraAppSizes();

		bool IsInsideViewFrustum(glm::vec3 pos);

		std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view);

		// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera
		// defined ENUM (to abstract it from windowing systems)
		void ProcessKeyboard(Camera_Movement direction, float deltaTime);

		// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
		void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);

		// processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
		void ProcessMouseScroll(float yoffset) {
			Zoom -= (float)yoffset;
			if (Zoom < 1.0f)
				Zoom = 1.0f;
			if (Zoom > 90.0f)
				Zoom = 90.0f;

			UpdateFrustum();
		}

		glm::vec3 ScreenPositionToRay(const glm::vec2& position, const glm::vec2& size);
		const PlViewport& GetViewport();
		void SetViewport(PlViewport* viewport);

		template <class Archive> void serialize(Archive& archive) {
			archive(cereal::base_class<Component>(this), PL_SER(isEditorCamera), PL_SER(Position), PL_SER(Front),
					PL_SER(Up), PL_SER(Right), PL_SER(WorldUp), PL_SER(Yaw), PL_SER(Pitch), PL_SER(MovementSpeed),
					PL_SER(MovementSpeedTemporaryBoost), PL_SER(MouseSensitivity), PL_SER(Zoom), PL_SER(nearPlane),
					PL_SER(farPlane), PL_SER(aspectRatio));
		}

	  private:
		// calculates the front vector from the Camera's (updated) Euler Angles
		void updateCameraVectors(Scene* scene);
		PlViewport* mViewport = nullptr;
	};
}; // namespace Plaza
