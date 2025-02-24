#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifdef WIN32
#include <Windows.h>
#endif
#include "ThirdParty/glad/glad.h"
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
#include <ThirdParty/imgui/imgui_internal.h>
#include <ThirdParty/imgui/imgui.h>
#include <ThirdParty/imgui/imgui_impl_glfw.h>

#include <ThirdParty/imgui/ImGuizmo.h>
#include "Engine/Utils/binaryUtils.h"
#ifdef WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#elif __linux__
#define VK_USE_PLATFORM_XCB_KHR
#elif __APPLE__
#define VK_USE_PLATFORM_MACOS_KHR
#endif
#include "ThirdParty/vulkan/vulkan/vulkan.h"
#include "Engine/Core/Debugging/Log.h"

//#include "ThirdParty/include/VulkanMemoryAllocator/vk_mem_alloc.h"
//#include "ThirdParty/physx/PxPhysicsAPI.h"
//#include "ThirdParty/physx/PxConfig.h"
//#include "ThirdParty/physx/PxPhysicsAPI.h"
//#include "ThirdParty/physx/vehicle/PxVehicleSDK.h"
#include <ThirdParty/PhysX/physx/include/PxPhysicsAPI.h>
#include <ThirdParty/PhysX/physx/include/cooking/Pxc.h>
#include <ThirdParty/cereal/cereal/archives/binary.hpp>
#include <ThirdParty/cereal/cereal/types/polymorphic.hpp>

#include <iostream>
#include <random>
#include <unordered_map>
#include <filesystem>
//#include "ThirdParty/filesystem/filesys.h"
#include <list>
#include <string>
#include <chrono>
#include <variant>

#include "Engine/Application/Application.h"
//#include "EntryPoint.h"
#include "Engine/Core/Standards.h"
#include "Engine/Core/UUID.h"
#include "Engine/Core/Debugging/Profiler.h"
#include "Engine/Core/AssetsManager/AssetsManager.h"
#include "Engine/Threads/ThreadManager.h"
#include "Engine/Core/Engine.h"
#include "Engine/Core/FilesManager/FilesManager.h"