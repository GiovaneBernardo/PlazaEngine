set(CMAKE_C_COMPILER cl)
set(CMAKE_CXX_COMPILER cl)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "" FORCE)

add_compile_options(
        /MP
        /bigobj
        /WX-
        /W4
        /wd4244
        /wd2220
        /INCREMENTAL
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(/Zi /Ob0 /Od /RTC1)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(/O2)
endif()