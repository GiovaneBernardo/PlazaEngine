set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unsafe-buffer-usage -fsafe-buffer-usage-suggestions -Wno-switch-default -Wswitch -Wunused-value -Wno-invalid-offsetof -fdeclspec -fms-extensions -Wno-unused-but-set-variable")

if(CMAKE_BUILD_TYPE STREQUAL "Release")
	set(CMAKE_CXX_FLAGS_RELEASE "/O2 ${CMAKE_CXX_FLAGS}")
endif()