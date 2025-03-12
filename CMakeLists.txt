cmake_minimum_required(VERSION 3.25)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    set(TARGET_BUILD_PLATFORM "windows")
    set(PLATFORM "Windows")
    set(EDITOR_CONFIG_DIRECTORY "$ENV{APPDATA}")
    set(GLFW_BUILD_WIN32)
	set(WINDOWS ON)
elseif(UNIX)
    set(TARGET_BUILD_PLATFORM "linux")
    set(PLATFORM "Linux")
    set(EDITOR_CONFIG_DIRECTORY "$ENV{HOME}/.config")
    set(GLFW_BUILD_X11)
	set(LINUX ON)
endif()

if(WIN32)
	if (MSVC)
        message(STATUS "Building with MSVC on Windows.")
    endif()
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "" FORCE)
    add_compile_options(/MP)
    add_compile_options(/bigobj)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX- /W4 /O2")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /C2220")
	set(CMAKE_CXX_FLAGS_DEBUG "/Zi /Ob0 /Od /RTC1")
	set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fno-inline -DDEBUG")
elseif(UNIX AND NOT APPLE)
    find_program(CLANGC_PATH NAMES clang)
    find_program(CLANG_PATH NAMES clang++)
    if(CLANG_PATH)
        message(STATUS "Using Clang for Linux build")
        set(CMAKE_C_COMPILER "${CLANGC_PATH}")
        set(CMAKE_CXX_COMPILER "${CLANG_PATH}")
    else()
        message(FATAL_ERROR "Clang not found. Please install Clang before building.")
    endif()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unsafe-buffer-usage -fsafe-buffer-usage-suggestions -Wno-switch-default -Wswitch -Wunused-value -Wno-invalid-offsetof -fdeclspec -fms-extensions")
endif()

project(PlazaEngineProject VERSION 0.0.2)
set(BUILD_STATIC_LIB ON)

include(FindVulkan)
if(NOT VULKAN_FOUND)
    message(FATAL_ERROR "Vulkan SDK not installed.")
endif()
find_package(Vulkan REQUIRED)

set(PLATFORM_CMAKELISTS "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")

message(WARNING "${CMAKE_BUILD_TYPE}")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
    add_compile_definitions(_DEBUG)
    add_definitions(-D_DEBUG)
    add_definitions(-D_ITERATOR_DEBUG_LEVEL=2)
    option(PHYSX_BUILD_TYPE "debug")

    #set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:>")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    add_compile_definitions(NDEBUG)
    add_definitions(-DNDEBUG)
    option(PHYSX_BUILD_TYPE "release")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
    option(MSVC_CRT_STATIC "Link primesieve.lib with /MT instead of the default /MD" ON)
endif()

add_definitions(-DLIBTYPE=STATIC)
add_compile_definitions(IMGUI_DEFINE_MATH_OPERATORS)

set(ENABLE_BOOST_WORKAROUND ON)
set(BUILD_STATIC_LIB ON)
set(GLM_FORCE_DEPTH_ZERO_TO_ONE ON)
set(PLAZA_ITERATOR_LEVEL 2)

set(BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/out/build/x64-${CMAKE_BUILD_TYPE}")
set(PHYSX_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/PhysX/physx")
set(PX_OUTPUT_LIB_DIR "${BUILD_DIR}/libs")
set(PX_OUTPUT_BIN_DIR "${BUILD_DIR}/bin")

set(PHYSX_LIBRARIES
    PhysXExtensions
    PhysX
    PhysXPvdSDK
    PhysXVehicle
    PhysXCharacterKinematic
    PhysXCooking
    PhysXCommon
    PhysXFoundation
)
option(COPY_EXTERNAL_DLLS "Control the copying of external DLLs" OFF) # PhysX external dlls
option(PX_BUILDSNIPPETS "Generate the snippets" OFF)
option(PX_BUILDPVDRUNTIME "Generate the OmniPVD project" OFF)
option(PX_GENERATE_STATIC_LIBRARIES "Generate static libraries" ON)
option(NV_USE_STATIC_WINCRT "Use the statically linked windows CRT" ON)
option(NV_USE_DEBUG_WINCRT "Use the debug version of the CRT" ON)
option(PX_FLOAT_POINT_PRECISE_MATH "Float point precise math" OFF)
#include("${PHYSX_ROOT_DIR}/compiler/public/CMakeLists.txt")
option(PX_BUILDSNIPPETS "Generate the snippets" OFF)
option(PX_CMAKE_SUPPRESS_REGENERATION "Disable zero_check projects" OFF)

set(GLFW_USE_WAYLAND OFF CACHE BOOL "Disable Wayland and use X11")
set(GPU_LIB_COPIED TRUE)

set(LIBTYPE STATIC)
set(BUILD_SHARED_LIBS OFF)
add_subdirectory("ThirdParty/AL/" EXCLUDE_FROM_ALL)
add_subdirectory("ThirdParty/embed/" EXCLUDE_FROM_ALL)
add_subdirectory("ThirdParty/GLFW/" EXCLUDE_FROM_ALL)
#add_subdirectory("ThirdParty/pfr/" EXCLUDE_FROM_ALL)
add_subdirectory("ThirdParty/Tracy/" EXCLUDE_FROM_ALL)
add_subdirectory("ThirdParty/PhysX/physx/compiler/public/" EXCLUDE_FROM_ALL)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Editor/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/Editor/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Core/Renderer/stb_font_consolas_24_latin1.inl")
file(GLOB_RECURSE MY_SOURCES2 CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Engine/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/Engine/*.h")

set(IMGUI_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/imgui.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/imgui_demo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/imgui_draw.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/imgui_widgets.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/ImGuizmo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/imgui_impl_opengl3.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/imgui_impl_glfw.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/imgui_impl_vulkan.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/imgui_tables.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/nodes/crude_json.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/nodes/imgui_canvas.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/nodes/imgui_node_editor.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/nodes/imgui_node_editor_api.cpp"
)

string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER)
set(PX_OUTPUT_LIB_PREFIX "bin/win.x86_64.vc143.mt")
if(WIN32)
	set(PHYSX_LIBS
		"${PX_OUTPUT_LIB_DIR}/${PX_OUTPUT_LIB_PREFIX}/${CMAKE_BUILD_TYPE_LOWER}/PhysX_static.lib"
		"${PX_OUTPUT_LIB_DIR}/${PX_OUTPUT_LIB_PREFIX}/${CMAKE_BUILD_TYPE_LOWER}/PhysXExtensions_static.lib"
		"${PX_OUTPUT_LIB_DIR}/${PX_OUTPUT_LIB_PREFIX}/${CMAKE_BUILD_TYPE_LOWER}/PhysXCooking_static.lib"
		"${PX_OUTPUT_LIB_DIR}/${PX_OUTPUT_LIB_PREFIX}/${CMAKE_BUILD_TYPE_LOWER}/PhysXCommon_static.lib"
		"${PX_OUTPUT_LIB_DIR}/${PX_OUTPUT_LIB_PREFIX}/${CMAKE_BUILD_TYPE_LOWER}/PhysXFoundation_static.lib"
		"${PX_OUTPUT_LIB_DIR}/${PX_OUTPUT_LIB_PREFIX}/${CMAKE_BUILD_TYPE_LOWER}/PhysXPvdSDK_static.lib"
		"${PX_OUTPUT_LIB_DIR}/${PX_OUTPUT_LIB_PREFIX}/${CMAKE_BUILD_TYPE_LOWER}/PhysXVehicle_static.lib"
		"${PX_OUTPUT_LIB_DIR}/${PX_OUTPUT_LIB_PREFIX}/${CMAKE_BUILD_TYPE_LOWER}/PhysXVehicle2_static.lib"
	)
elseif(UNIX)
    set(PX_OUTPUT_LIB_PREFIX "bin/linux.clang")
	set(PHYSX_LIBS
		"${PX_OUTPUT_LIB_DIR}/${PX_OUTPUT_LIB_PREFIX}/${CMAKE_BUILD_TYPE_LOWER}/libPhysX_static.a"
		"${PX_OUTPUT_LIB_DIR}/${PX_OUTPUT_LIB_PREFIX}/${CMAKE_BUILD_TYPE_LOWER}/libPhysXExtensions_static.a"
		"${PX_OUTPUT_LIB_DIR}/${PX_OUTPUT_LIB_PREFIX}/${CMAKE_BUILD_TYPE_LOWER}/libPhysXCooking_static.a"
		"${PX_OUTPUT_LIB_DIR}/${PX_OUTPUT_LIB_PREFIX}/${CMAKE_BUILD_TYPE_LOWER}/libPhysXCommon_static.a"
		"${PX_OUTPUT_LIB_DIR}/${PX_OUTPUT_LIB_PREFIX}/${CMAKE_BUILD_TYPE_LOWER}/libPhysXFoundation_static.a"
		"${PX_OUTPUT_LIB_DIR}/${PX_OUTPUT_LIB_PREFIX}/${CMAKE_BUILD_TYPE_LOWER}/libPhysXPvdSDK_static.a"
		"${PX_OUTPUT_LIB_DIR}/${PX_OUTPUT_LIB_PREFIX}/${CMAKE_BUILD_TYPE_LOWER}/libPhysXVehicle_static.a"
		"${PX_OUTPUT_LIB_DIR}/${PX_OUTPUT_LIB_PREFIX}/${CMAKE_BUILD_TYPE_LOWER}/libPhysXVehicle2_static.a"
	)
endif()

# PlazaEngineCore
add_library(PlazaEngineCore SHARED "${IMGUI_SOURCES}" "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Application/EntryPoint.cpp" ${MY_SOURCES} ${MY_SOURCES2})

b_embed(PlazaEngineCore Editor/DefaultAssets/NewProject/CMakeLists.txt)
b_embed(PlazaEngineCore Editor/DefaultAssets/NewProject/CMakePresets.json)
b_embed(PlazaEngineCore Editor/DefaultAssets/NewProject/.gitignore)

target_compile_definitions(PlazaEngineCore PUBLIC PX_PHYSX_STATIC_LIB)
target_compile_definitions(PlazaEngineCore PUBLIC EDITOR_MODE)
#target_compile_options(PlazaEngineCore PUBLIC /WX-)

if(MSVC)
    target_compile_options(PlazaEngineCore PUBLIC "/ZI")
    target_link_options(PlazaEngineCore PUBLIC "/INCREMENTAL")
endif()
#target_include_directories(PlazaEngineCore PUBLIC "lib/")

target_include_directories(PlazaEngineCore PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/> # For builds
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/AL/include/AL>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/cereal/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/filesystem/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/FileWatcher/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glad/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/GLFW/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glm/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/Json/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/KHR/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/minimp3/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/PhysX/physx/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/pfr/include/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/premake/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/stb/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/spdlog/include/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/tinyobjloader/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/uuid_v4/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/vswhere/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/vulkan/vulkan/>

    $<INSTALL_INTERFACE:include> # For installs (adjust paths if necessary)
)

if(WIN32)
    set(LIB_PREFIX "")
elseif(UNIX)
    set(LIB_PREFIX "lib")
endif()

#add_dependencies(PlazaEngineCore PhysX)
#target_include_directories(PlazaEngineCore PUBLIC "ThirdParty/imgui/")

add_library(PlazaEngineCoreLibs INTERFACE)
#add_dependencies(PlazaEngineCore PhysX)
#add_dependencies(PlazaEngineCoreLibs PhysX)
target_link_libraries(PlazaEngineCoreLibs INTERFACE
    $<BUILD_INTERFACE:glfw>
    $<BUILD_INTERFACE:OpenAL>
    $<BUILD_INTERFACE:TracyClient>
    $<BUILD_INTERFACE:${PHYSX_LIBS}>
    $<BUILD_INTERFACE:Vulkan::Vulkan>
)
foreach(PHYSX_LIB ${PHYSX_LIBS})
    target_link_libraries(PlazaEngineCoreLibs INTERFACE
        $<BUILD_INTERFACE:${PHYSX_LIB}>
    )
endforeach()

target_link_libraries(PlazaEngineCore PUBLIC PlazaEngineCoreLibs)

target_include_directories(PlazaEngineCoreLibs INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/> # For builds
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/AL/include/AL>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/cereal/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/filesystem/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/FileWatcher/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glad/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/GLFW/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glm/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/Json/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/KHR/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/minimp3/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/PhysX/physx/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/pfr/include/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/premake/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/stb/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/spdlog/include/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/tinyobjloader/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/uuid_v4/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/vswhere/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/vulkan/vulkan/>

    $<INSTALL_INTERFACE:include> # For installs (adjust paths if necessary)
)
#target_link_options(PlazaEngineCore PUBLIC
#    #$<$<CXX_COMPILER_ID:MSVC>:/WHOLEARCHIVE:glfw>
#    #$<$<CXX_COMPILER_ID:MSVC>:/WHOLEARCHIVE:OpenAL>
#    $<$<CXX_COMPILER_ID:MSVC>:/WHOLEARCHIVE:TracyClient>
#    $<$<CXX_COMPILER_ID:MSVC>:/WHOLEARCHIVE:DirectXTex>
#    #$<$<CXX_COMPILER_ID:MSVC>:/WHOLEARCHIVE:${CMAKE_CURRENT_SOURCE_DIR}/lib/OpenAL32.lib>
#    #$<$<CXX_COMPILER_ID:MSVC>:/WHOLEARCHIVE:Vulkan::Vulkan>
#)
#foreach(PHYSX_LIB ${PHYSX_LIBS})
#    target_link_options(PlazaEngineCore PUBLIC $<$<CXX_COMPILER_ID:MSVC>:/WHOLEARCHIVE:${PHYSX_LIB}>)
#endforeach()

target_compile_definitions(PlazaEngineCore PUBLIC ENGINE_EXPORT)

if(MSVC)
    target_compile_options(PlazaEngineCore PRIVATE "/MP")
endif()


set_target_properties(PlazaEngineCore PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" # For the DLL
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" # For the .lib (import library)
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" # For shared libs
)
set_property(TARGET PlazaEngineCore PROPERTY CXX_STANDARD 20)

# PlazaEngine and PlazaShippingGame
add_executable(PlazaEngine "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Application/EntryPoint.cpp")

set_target_properties(PlazaEngine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" # For the DLL
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" # For the .lib (import library)
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" # For shared libs
)

add_executable(PlazaShippingGame "${IMGUI_SOURCES}" "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Application/EntryPoint.cpp" "${MY_SOURCES}" "${MY_SOURCES2}")
set_target_properties(PlazaShippingGame PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" # For the DLL
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" # For the .lib (import library)
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" # For shared libs
)

target_compile_definitions(PlazaEngine PUBLIC PX_PHYSX_STATIC_LIB)
target_compile_definitions(PlazaEngine PUBLIC EDITOR_MODE)
#target_compile_options(PlazaEngine PUBLIC /WX-)
target_compile_definitions(PlazaEngine PUBLIC ENGINE_EXPORT)

target_compile_definitions(PlazaShippingGame PUBLIC GAME_MODE)
target_compile_definitions(PlazaShippingGame PUBLIC PX_PHYSX_STATIC_LIB)
#target_compile_options(PlazaShippingGame PUBLIC /WX-)
#target_compile_definitions(PlazaShippingGame PRIVATE ENGINE_EXPORT)

target_precompile_headers(PlazaEngine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Core/PreCompiledHeaders.h")
target_precompile_headers(PlazaEngineCore PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Core/PreCompiledHeaders.h")
target_precompile_headers(PlazaShippingGame PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Core/PreCompiledHeaders.h")

if(MSVC)
    target_compile_options(PlazaEngine PUBLIC "/ZI")
    target_link_options(PlazaEngine PUBLIC "/INCREMENTAL")

    target_compile_options(PlazaShippingGame PUBLIC "/ZI")
    target_link_options(PlazaShippingGame PUBLIC "/INCREMENTAL")
endif()

link_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib")

target_include_directories(PlazaEngine PUBLIC "lib/")
target_include_directories(PlazaShippingGame PUBLIC "lib/")

target_link_libraries(PlazaEngine PRIVATE PlazaEngineCore)
target_link_libraries(PlazaShippingGame PRIVATE PlazaEngineCore)

# Copy the PlazaEngineCore.lib to game project folder (if found)

set(JSON_FILE_PATH "${EDITOR_CONFIG_DIRECTORY}/PlazaEngine/cache.plzcache")
if(NOT EXISTS "${JSON_FILE_PATH}")
    message("Failed to find project path in JSON file.")
elseif(matched_value0)
    file(READ "${JSON_FILE_PATH}" json_content)
    string(REGEX MATCH "\"value0\"[ \t\r\n]*:[ \t\r\n]*\"([^\"]+)\"" matched_value0 ${json_content})
    string(REGEX REPLACE ".*\"([^\"]*)\".*" "\\1" project_path ${matched_value0})
    get_filename_component(parent_path "${project_path}" DIRECTORY)
    set(target_path "${parent_path}/bin/")
    add_custom_command(TARGET PlazaEngineCore
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_LINKER_FILE:PlazaEngineCore> ${target_path}
        COMMENT "Copying PlazaEngineCore.lib to game project folder")
endif()

add_custom_command(TARGET PlazaEngineCore
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glslc" "${CMAKE_CURRENT_BINARY_DIR}/ThirdParty/glslc"
    COMMENT "Copying GLSLC to engine executable folder")


#set(LOCAL_INSTALL_DIR "${CMAKE_BINARY_DIR}/install")
#
## Install PlazaEngineCore headers
#file(GLOB_RECURSE PlazaEngineCore_HEADERS
#    "${CMAKE_CURRENT_SOURCE_DIR}/Engine/**/*.h"
#    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/**/*.h"
#    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/**/*.hpp"
#)
#install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Engine/"
#        DESTINATION include/PlazaEngineCore/Engine
#        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp" PATTERN "*.inl")
#install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Editor/"
#        DESTINATION include/PlazaEngineCore/Editor
#        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp" PATTERN "*.inl")
#install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/"
#        DESTINATION include/PlazaEngineCore/ThirdParty
#        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp" PATTERN "*.inl")
#
## Install PlazaEngineCore with exported targets
#install(TARGETS PlazaEngineCore
#        EXPORT PlazaEngineProjectTargets
#        ARCHIVE DESTINATION lib
#        LIBRARY DESTINATION lib
#        RUNTIME DESTINATION bin
#        INCLUDES DESTINATION include
#)
#
## Export the targets to a file
#install(EXPORT PlazaEngineProjectTargets
#        FILE PlazaEngineProjectTargets.cmake
#        NAMESPACE Plaza::
#        DESTINATION lib/cmake/PlazaEngineProject
#)
#
## Add a package configuration file for PlazaEngineCore
#include(CMakePackageConfigHelpers)
#write_basic_package_version_file(
#    "${CMAKE_BINARY_DIR}/PlazaEngineProjectConfigVersion.cmake"
#    COMPATIBILITY SameMajorVersion
#)
#
#configure_package_config_file(
#    "${CMAKE_CURRENT_SOURCE_DIR}/PlazaEngineProjectConfig.cmake.in"
#    "${CMAKE_BINARY_DIR}/PlazaEngineProjectConfig.cmake"
#    INSTALL_DESTINATION lib/cmake/PlazaEngineProject
#)
#
#install(FILES
#    "${CMAKE_BINARY_DIR}/PlazaEngineProjectConfig.cmake"
#    "${CMAKE_BINARY_DIR}/PlazaEngineProjectConfigVersion.cmake"
#    DESTINATION lib/cmake/PlazaEngineProject
#)
#
## Automatically run the install step after building PlazaEngineCore
#add_custom_command(
#    TARGET PlazaEngineCore
#    POST_BUILD
#    COMMAND ${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR} --prefix ${LOCAL_INSTALL_DIR}
#    COMMENT "Installing PlazaEngineCore to ${LOCAL_INSTALL_DIR}"
#)
