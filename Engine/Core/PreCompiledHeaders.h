#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <ThirdParty/glad/glad.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RIGHT_HANDED
#include <ThirdParty/glm/glm.hpp>
#include <ThirdParty/glm/gtc/matrix_transform.hpp>
#include <ThirdParty/glm/gtc/type_ptr.hpp>
#include <ThirdParty/glm/gtx/matrix_decompose.hpp>
#include <ThirdParty/glm/trigonometric.hpp>
#include <ThirdParty/glm/gtx/euler_angles.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <ThirdParty/glm/gtx/quaternion.hpp>
#include <ThirdParty/stb/stb_image.h>
#include <ThirdParty/imgui/imgui_internal.h>
#include <ThirdParty/imgui/imgui.h>
#include <ThirdParty/imgui/imgui_impl_glfw.h>

#include <ThirdParty/imgui/ImGuizmo.h>
#include "ThirdParty/yaml-cpp/include/yaml-cpp/yaml.h"
#include "ThirdParty/assimp/include/assimp/Importer.hpp"
#include "ThirdParty/assimp/include/assimp/postprocess.h"
#include "ThirdParty/Json/json.hpp" // Include the nlohmann/json library
#include "Engine/Utils/yamlUtils.h"
#define VK_USE_PLATFORM_WIN32_KHR
#include "ThirdParty/vulkan/vulkan/vulkan.h"
//#include "ThirdParty/physx/PxPhysicsAPI.h"
//#include "ThirdParty/physx/PxConfig.h"
//#include "ThirdParty/physx/PxPhysicsAPI.h"
//#include "ThirdParty/physx/vehicle/PxVehicleSDK.h"
#include <ThirdParty/PhysX/physx/include/PxPhysicsAPI.h>
#include <ThirdParty/PhysX/physx/include/cooking/Pxc.h>
#include "ThirdParty/mono/mono/mini/jit.h"
#include "ThirdParty/mono/mono/metadata/assembly.h"

#include <iostream>
#include <random>
#include <unordered_map>
#include <filesystem>
#include "ThirdParty/filesystem/filesys.h"
#include <list>
#include <string>
#include <chrono>
#include <variant>

#include "Engine/Application/Application.h"
#include "Engine/Application/EntryPoint.h"
#include "Engine/Core/Standards.h"
#include "Engine/Core/UUID.h"
#include "Engine/Components/Core/Entity.h"
#include "Engine/Core/ComponentMapStructure.h"
#include "Engine/Core/Debugging/Profiler.h"
#include "Editor/Settings/EditorSettings.h"