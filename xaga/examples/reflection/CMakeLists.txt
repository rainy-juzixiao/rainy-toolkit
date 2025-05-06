add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/function/01_basic)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/reflect/01_register)

if (MSVC)
	target_compile_options(rainy-toolkit-reflect-01-register PUBLIC /W4)
endif()