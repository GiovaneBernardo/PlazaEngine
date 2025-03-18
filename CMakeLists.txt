cmake_minimum_required(VERSION 3.25)
project(PlazaEngineProject VERSION 0.0.2)

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

include(FindVulkan)
if(NOT VULKAN_FOUND)
    message(FATAL_ERROR "Vulkan SDK not installed.")
endif()
find_package(Vulkan REQUIRED)

message(WARNING "${CMAKE_BUILD_TYPE}")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
    add_compile_definitions(_DEBUG)
    add_definitions(-D_DEBUG)
    add_definitions(-D_ITERATOR_DEBUG_LEVEL=2)
    option(PHYSX_BUILD_TYPE "debug")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    add_compile_definitions(NDEBUG)
    add_definitions(-DNDEBUG)
    option(PHYSX_BUILD_TYPE "release")
endif()

add_compile_definitions(IMGUI_DEFINE_MATH_OPERATORS)
set(GLM_FORCE_DEPTH_ZERO_TO_ONE ON)

set(PHYSX_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/PhysX/physx")
set(PX_OUTPUT_LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}/libs")
set(PX_OUTPUT_BIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/bin")

set(PHYSX_LIBRARIES
    PhysX
    PhysXExtensions
    PhysXPvdSDK
    PhysXVehicle
    PhysXCharacterKinematic
    PhysXCooking
    PhysXCommon
    PhysXFoundation
)
option(NV_USE_STATIC_WINCRT "Use the statically linked windows CRT" ON)
option(NV_USE_DEBUG_WINCRT "Use the debug version of the CRT" ON)
option(COPY_EXTERNAL_DLLS "Control the copying of external DLLs" OFF) # PhysX external dlls
option(PX_BUILDSNIPPETS "Generate the snippets" OFF)
option(PX_BUILDPVDRUNTIME "Generate the OmniPVD project" OFF)
option(PX_GENERATE_STATIC_LIBRARIES "Generate static libraries" ON)
option(PX_FLOAT_POINT_PRECISE_MATH "Float point precise math" OFF)
set(GPU_LIB_COPIED TRUE)
set(GPU_DLL_COPIED TRUE)

set(GLFW_USE_WAYLAND OFF CACHE BOOL "Disable Wayland and use X11")
set(BUILD_SHARED_LIBS OFF)
set(LIBTYPE STATIC)
set(CMAKE_SHARED_LINKER_FLAGS_CHECKED "")
set(CMAKE_SHARED_LINKER_FLAGS_PROFILE "")
set(CMAKE_EXE_LINKER_FLAGS_CHECKED "")
set(CMAKE_EXE_LINKER_FLAGS_PROFILE "")

add_subdirectory("ThirdParty/AL/" EXCLUDE_FROM_ALL)
#add_subdirectory("ThirdParty/embed/" EXCLUDE_FROM_ALL)
include(FetchContent)
FetchContent_Declare(
  battery-embed
  GIT_REPOSITORY https://github.com/batterycenter/embed.git
  GIT_TAG        v1.2.19
)
FetchContent_MakeAvailable(battery-embed)
add_subdirectory("ThirdParty/GLFW/" EXCLUDE_FROM_ALL)
#add_subdirectory("ThirdParty/pfr/" EXCLUDE_FROM_ALL)
add_subdirectory("ThirdParty/Tracy/" EXCLUDE_FROM_ALL)
add_subdirectory("ThirdParty/PhysX/physx/compiler/public/")

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

if(WIN32)
    set(PX_OUTPUT_LIB_PREFIX "bin/win.x86_64.vc143.mt")
elseif(UNIX)
    set(PX_OUTPUT_LIB_PREFIX "bin/linux.clang")
endif()

# PlazaEngineCoreLibs
add_library(PlazaEngineCoreLibs INTERFACE)

target_link_directories(PlazaEngineCoreLibs INTERFACE "${PX_OUTPUT_LIB_DIR}/${PX_OUTPUT_LIB_PREFIX}/${CMAKE_BUILD_TYPE_LOWER}/")
target_link_libraries(PlazaEngineCoreLibs INTERFACE
    $<BUILD_INTERFACE:glfw>
    $<BUILD_INTERFACE:OpenAL>
    $<BUILD_INTERFACE:TracyClient>
    $<BUILD_INTERFACE:Vulkan::Vulkan>
    $<BUILD_INTERFACE:${PHYSX_LIBRARIES}>
)

# PlazaEngineCore
add_library(PlazaEngineCore SHARED "${IMGUI_SOURCES}" "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Application/EntryPoint.cpp" ${MY_SOURCES} ${MY_SOURCES2})

b_embed(PlazaEngineCore Editor/DefaultAssets/NewProject/CMakeLists.txt)
b_embed(PlazaEngineCore Editor/DefaultAssets/NewProject/CMakePresets.json)
b_embed(PlazaEngineCore Editor/DefaultAssets/NewProject/.gitignore)

target_compile_definitions(PlazaEngineCore PUBLIC PX_PHYSX_STATIC_LIB)
target_compile_definitions(PlazaEngineCore PUBLIC EDITOR_MODE)
#target_compile_options(PlazaEngineCore PUBLIC /WX-)

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
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

add_executable(PlazaShippingGame "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Application/EntryPoint.cpp")

set_target_properties(PlazaShippingGame PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

target_compile_definitions(PlazaEngine PUBLIC PX_PHYSX_STATIC_LIB)
target_compile_definitions(PlazaEngine PUBLIC EDITOR_MODE)
target_compile_definitions(PlazaEngine PUBLIC ENGINE_EXPORT)

target_compile_definitions(PlazaShippingGame PUBLIC GAME_MODE)
target_compile_definitions(PlazaShippingGame PUBLIC PX_PHYSX_STATIC_LIB)
#target_compile_definitions(PlazaShippingGame PRIVATE ENGINE_EXPORT)

target_precompile_headers(PlazaEngineCore PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Core/PreCompiledHeaders.h")

if(MSVC)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(PlazaEngine PUBLIC "/ZI")
        target_link_options(PlazaEngine PUBLIC "/INCREMENTAL")

        target_compile_options(PlazaShippingGame PUBLIC "/ZI")
        target_link_options(PlazaShippingGame PUBLIC "/INCREMENTAL")
    endif()
elseif(CLANG)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(PlazaEngine PUBLIC "-g")
        target_link_options(PlazaEngine PUBLIC "-fuse-ld=lld")

        target_compile_options(PlazaShippingGame PUBLIC "-g")
        target_link_options(PlazaShippingGame PUBLIC "-fuse-ld=lld")
    endif()
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
