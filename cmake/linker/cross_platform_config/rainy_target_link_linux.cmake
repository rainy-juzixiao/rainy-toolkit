cmake_minimum_required(VERSION 3.22.1)

message("Buiding windows target")
	if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
		target_sources(rainy-toolkit PRIVATE
			${PROJECT_SOURCE_DIR}/master/sources/linux_api/system/multithread/thread.cxx
			#${PROJECT_SOURCE_DIR}/master/sources/rainy_cxxfiles/winapi/system/multithread/mutex.cxx
		)
endif()