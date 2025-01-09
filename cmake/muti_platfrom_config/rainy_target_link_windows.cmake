cmake_minimum_required(VERSION 3.22.1)

if (WIN32)
	add_library(rainy_toolkit_winapi_link_libray)
	set_target_properties(rainy_toolkit_winapi_link_libray PROPERTIES LINKER_LANGUAGE CXX)

	target_include_directories(rainy_toolkit_winapi_link_libray PRIVATE ${PROJECT_SOURCE_DIR}/include)
	target_sources(rainy_toolkit_winapi_link_libray PUBLIC 
		${PROJECT_SOURCE_DIR}/include/rainy/sources/winapi/core/impl_thread.cxx
	)
endif()