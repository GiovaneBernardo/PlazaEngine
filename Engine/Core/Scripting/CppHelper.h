#pragma once
#include "Engine/Core/PreCompiledHeaders.h"
#include "Engine/Core/Engine.h"
#include "Engine/Core/Physics.h"
#include "Engine/Core/Scene.h"

#include "Engine/Components/Component.h"
#include "Engine/Components/Core/Entity.h"
#include "Engine/Components/Audio/AudioListener.h"
#include "Engine/Components/Audio/AudioSource.h"
#include "Engine/Components/Core/Camera.h"
#include "Engine/Components/Core/Prefab.h"
#include "Engine/Components/Core/Transform.h"
#include "Engine/Components/Drawing/UI/TextRenderer.h"
#include "Engine/Components/Physics/Collider.h"
#include "Engine/Components/Physics/RigidBody.h"
#include "Engine/Components/Rendering/AnimationComponent.h"
#include "Engine/Components/Rendering/Light.h"
#include "Engine/Components/Rendering/Material.h"
#include "Engine/Components/Rendering/MeshRenderer.h"
#include "Engine/Components/Scripting/CppScriptComponent.h"

#include "ThirdParty/imgui/imgui.h"

#include "Engine/Core/Scripting/CppUtilities/PerlinNoise.h"

namespace Plaza {
	class PLAZA_API Mathf {
	  public:
		static inline constexpr float sDeg2Rad = 3.14159265359f / 180.0f;
		static inline constexpr float sRad2Deg = 180.0f / 3.14159265359f;
	};
	class PLAZA_API CppHelper {
	  public:
		static Entity* NewEntity(const std::string& name);
		static Entity* FindEntity(const std::string& name);
		static Entity* FindEntity(uint64_t uuid);
		static Entity* Instantiate(Entity* entityToInstantiate);
		static glm::vec3 MousePosToRayDirection(float fov, const glm::vec2& position, const glm::vec2& screenSize);
	};
} // namespace Plaza
