set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

add_compile_options(
	-Wno-unsafe-buffer-usage
	-fsafe-buffer-usage-suggestions
	-Wno-switch-default
	-Wswitch
	-Wunused-value
	-Wno-invalid-offsetof
	-fdeclspec
	-fms-extensions
	-Wno-unused-but-set-variable
	-O2
)