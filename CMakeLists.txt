cmake_minimum_required(VERSION 3.25)
project(PlazaEngineProject VERSION 0.0.2)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(POLICY CMP0091) # Force thidparty libraries to use static MSVC
    cmake_policy(SET CMP0091 NEW)
endif()


if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "" FORCE)
    add_compile_options(/MP)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX- /W4 /O2")
    add_compile_options(/bigobj)
endif()

set(BUILD_STATIC_LIB ON)

set(TARGET_BUILD_PLATFORM "windows")
set(PLATFORM "Windows")

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
    set(CMAKE_CXX_FLAGS_DEBUG "/Zi /Ob0 /Od /RTC1")
    option(MSVC_CRT_STATIC   "Link primesieve.lib with /MT instead of the default /MD" OFF)
    #set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:>")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    add_compile_definitions(NDEBUG)
    add_definitions(-DNDEBUG)
    option(PHYSX_BUILD_TYPE "release")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
    option(MSVC_CRT_STATIC   "Link primesieve.lib with /MT instead of the default /MD" ON)
    #set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
endif()
#add_compile_definitions(<$<CONFIG:Debug>:_DEBUG><$<CONFIG:Debug>:NDEBUG>)
add_definitions(-DLIBTYPE=STATIC)
add_compile_definitions(IMGUI_DEFINE_MATH_OPERATORS)
#set(CMAKE_BUILD_TYPE Debug)

set(ENABLE_BOOST_WORKAROUND ON)
set(BUILD_STATIC_LIB ON)
set(GLM_FORCE_DEPTH_ZERO_TO_ONE ON)

set(PLAZA_ITERATOR_LEVEL 2)

set(BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/out/build/x64-${CMAKE_BUILD_TYPE}")
set(PHYSX_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/PhysX/physx")
set(PX_OUTPUT_LIB_DIR "${BUILD_DIR}/libs")
set(PX_OUTPUT_BIN_DIR "${BUILD_DIR}/bin")

OPTION(COPY_EXTERNAL_DLLS "Control the copying of external DLLs" OFF) # PhysX external dlls
option(PX_BUILDSNIPPETS "Generate the snippets" OFF)
option(PX_BUILDPVDRUNTIME "Generate the OmniPVD project" OFF)
option(PX_GENERATE_STATIC_LIBRARIES "Generate static libraries" ON)
option(NV_USE_STATIC_WINCRT "Use the statically linked windows CRT" ON)
option(NV_USE_DEBUG_WINCRT "Use the debug version of the CRT" ON)
option(PX_FLOAT_POINT_PRECISE_MATH "Float point precise math" OFF)
#include("${PHYSX_ROOT_DIR}/compiler/public/CMakeLists.txt")
OPTION(PX_BUILDSNIPPETS "Generate the snippets" OFF)
OPTION(PX_CMAKE_SUPPRESS_REGENERATION "Disable zero_check projects" OFF)

set(LIBTYPE STATIC)
set(BUILD_SHARED_LIBS OFF)
add_subdirectory("ThirdParty/AL/" EXCLUDE_FROM_ALL)
add_subdirectory("ThirdParty/DirectXTex/" EXCLUDE_FROM_ALL)
add_subdirectory("ThirdParty/embed/" EXCLUDE_FROM_ALL)
add_subdirectory("ThirdParty/GLFW/" EXCLUDE_FROM_ALL)
#add_subdirectory("ThirdParty/pfr/" EXCLUDE_FROM_ALL)
add_subdirectory("ThirdParty/Tracy/" EXCLUDE_FROM_ALL)
add_subdirectory("ThirdParty/PhysX/physx/compiler/public/" EXCLUDE_FROM_ALL)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /C2220")



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
set(PHYSX_LIBS
    "${PX_OUTPUT_LIB_DIR}/bin/win.x86_64.vc143.mt/${CMAKE_BUILD_TYPE_LOWER}/PhysX_static.lib"
    "${PX_OUTPUT_LIB_DIR}/bin/win.x86_64.vc143.mt/${CMAKE_BUILD_TYPE_LOWER}/PhysXExtensions_static.lib"
    "${PX_OUTPUT_LIB_DIR}/bin/win.x86_64.vc143.mt/${CMAKE_BUILD_TYPE_LOWER}/PhysXCooking_static.lib"
    "${PX_OUTPUT_LIB_DIR}/bin/win.x86_64.vc143.mt/${CMAKE_BUILD_TYPE_LOWER}/PhysXCommon_static.lib"
    "${PX_OUTPUT_LIB_DIR}/bin/win.x86_64.vc143.mt/${CMAKE_BUILD_TYPE_LOWER}/PhysXFoundation_static.lib"
    "${PX_OUTPUT_LIB_DIR}/bin/win.x86_64.vc143.mt/${CMAKE_BUILD_TYPE_LOWER}/PhysXPvdSDK_static.lib"
    "${PX_OUTPUT_LIB_DIR}/bin/win.x86_64.vc143.mt/${CMAKE_BUILD_TYPE_LOWER}/PhysXVehicle_static.lib"
    "${PX_OUTPUT_LIB_DIR}/bin/win.x86_64.vc143.mt/${CMAKE_BUILD_TYPE_LOWER}/PhysXVehicle2_static.lib"
)

set(MONO_PATH "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/mono/lib/${CMAKE_BUILD_TYPE}")

# PlazaEngineCore
add_library(PlazaEngineCore STATIC "${IMGUI_SOURCES}" "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Application/EntryPoint.cpp" ${MY_SOURCES} ${MY_SOURCES2})

b_embed(PlazaEngineCore Editor/DefaultAssets/NewProject/CMakeLists.txt)
b_embed(PlazaEngineCore Editor/DefaultAssets/NewProject/CMakePresets.json)
b_embed(PlazaEngineCore Editor/DefaultAssets/NewProject/.gitignore)

set_target_properties(PlazaEngineCore PROPERTIES
    VS_GLOBAL_MONO_USE_STATIC_C_RUNTIME "true"
    VS_GLOBAL_MONO_USE_STATIC_LIBMONO "true"
)
target_compile_definitions(PlazaEngineCore PUBLIC  PX_PHYSX_STATIC_LIB)
target_compile_definitions(PlazaEngineCore PUBLIC  EDITOR_MODE)
target_compile_options(PlazaEngineCore PUBLIC  /WX-)

if(MSVC)
  target_compile_options(PlazaEngineCore PUBLIC "/ZI")
  target_link_options(PlazaEngineCore PUBLIC "/INCREMENTAL")
endif()
#target_include_directories(PlazaEngineCore PUBLIC "lib/")

target_include_directories(PlazaEngineCore PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>                        # For builds
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/AL/include/AL>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/cereal/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/DirectXTex/DirectXTex/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/filesystem/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/FileWathcer/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glad/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/GLFW/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/glm/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/Json/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/KHR/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/minimp3/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/mono/include/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/PhysX/physx/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/pfr/include/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/premake/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/stb/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/spdlog/include/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/tinyobjloader/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/uuid_v4/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/vswhere/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/vulkan/vulkan/>
    
    $<INSTALL_INTERFACE:include>  # For installs (adjust paths if necessary)
)

#add_dependencies(PlazaEngineCore PhysX)
#target_include_directories(PlazaEngineCore PUBLIC "ThirdParty/imgui/")
target_link_libraries(PlazaEngineCore PUBLIC  glfw OpenAL TracyClient "${MONO_PATH}/mono-2.0-sgen.lib" "${MONO_PATH}/libmono-static-sgen.lib"  DirectXTex ${PHYSX_LIBS} "${CMAKE_CURRENT_SOURCE_DIR}/lib/OpenAL32.lib" Vulkan::Vulkan)
target_compile_definitions(PlazaEngineCore PRIVATE ENGINE_EXPORT)

if(MSVC)
 target_compile_options(PlazaEngineCore PRIVATE "/MP")
endif()


set_target_properties(PlazaEngineCore PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"  # For the DLL
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"  # For the .lib (import library)
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"  # For shared libs
)
  set_property(TARGET PlazaEngineCore PROPERTY CXX_STANDARD 20)

# PlazaEngine and PlazaShippingGame
add_executable(PlazaEngine  "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Application/EntryPoint.cpp")

set_target_properties(PlazaEngine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"  # For the DLL
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"  # For the .lib (import library)
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"  # For shared libs
)

add_executable(PlazaShippingGame "${IMGUI_SOURCES}" "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Application/EntryPoint.cpp" "${MY_SOURCES}" "${MY_SOURCES2}")
set_target_properties(PlazaShippingGame PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"  # For the DLL
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"  # For the .lib (import library)
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"  # For shared libs
)

set_target_properties(PlazaEngine PROPERTIES
    VS_GLOBAL_MONO_USE_STATIC_C_RUNTIME "true"
    VS_GLOBAL_MONO_USE_STATIC_LIBMONO "true"
)

set_target_properties(PlazaShippingGame PROPERTIES
    VS_GLOBAL_MONO_USE_STATIC_C_RUNTIME "true"
    VS_GLOBAL_MONO_USE_STATIC_LIBMONO "true"
)

#target_link_libraries(PlazaEngine PUBLIC  glfw OpenAL TracyClient "${MONO_PATH}/mono-2.0-sgen.lib" "${MONO_PATH}/libmono-static-sgen.lib"  DirectXTex ${PHYSX_LIBS} "${CMAKE_CURRENT_SOURCE_DIR}/lib/OpenAL32.lib" Vulkan::Vulkan Boost::pfr)
target_compile_definitions(PlazaEngine PUBLIC  PX_PHYSX_STATIC_LIB)
target_compile_definitions(PlazaEngine PUBLIC  EDITOR_MODE)
target_compile_options(PlazaEngine PUBLIC  /WX-)
target_compile_definitions(PlazaEngine PRIVATE ENGINE_EXPORT)

target_compile_definitions(PlazaShippingGame PUBLIC  GAME_MODE)
target_compile_definitions(PlazaShippingGame PUBLIC  PX_PHYSX_STATIC_LIB)
target_compile_options(PlazaShippingGame PUBLIC  /WX-)
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
set(JSON_FILE_PATH "$ENV{APPDATA}/PlazaEngine/cache.plzcache")
file(READ "${JSON_FILE_PATH}" json_content)
string(REGEX MATCH "\"value0\"[ \t\r\n]*:[ \t\r\n]*\"([^\"]+)\"" matched_value0 ${json_content})
if (matched_value0)
    string(REGEX REPLACE ".*\"([^\"]*)\".*" "\\1" project_path ${matched_value0})
    get_filename_component(parent_path "${project_path}" DIRECTORY)
    set(target_path "${parent_path}/bin/")
    add_custom_command(TARGET PlazaEngineCore
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_LINKER_FILE:PlazaEngineCore> ${target_path}
        COMMENT "Copying PlazaEngineCore.lib to game project folder")
else()
    message("Failed to find project path in JSON file.")
endif()

# Install PlazaEngineCore with exported targets
install(TARGETS PlazaEngineCore
        EXPORT PlazaEngineCoreTargets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
)

# Export the targets to a file
install(EXPORT PlazaEngineCoreTargets
        FILE PlazaEngineCoreTargets.cmake
        NAMESPACE Plaza::
        DESTINATION lib/cmake/PlazaEngineCore
)

# Add a package configuration file for PlazaEngineCore
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_BINARY_DIR}/PlazaEngineCoreConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/PlazaEngineCoreConfig.cmake.in"
    "${CMAKE_BINARY_DIR}/PlazaEngineCoreConfig.cmake"
    INSTALL_DESTINATION lib/cmake/PlazaEngineCore
)

install(FILES
    "${CMAKE_BINARY_DIR}/PlazaEngineCoreConfig.cmake"
    "${CMAKE_BINARY_DIR}/PlazaEngineCoreConfigVersion.cmake"
    DESTINATION lib/cmake/PlazaEngineCore
)

add_custom_target(auto_install
    COMMAND "${CMAKE_COMMAND}" --build . --target install
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# Make the build depend on the auto_install target
add_dependencies(auto_install PlazaEngineCore)