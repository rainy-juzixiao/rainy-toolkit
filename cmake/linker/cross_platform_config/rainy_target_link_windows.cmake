cmake_minimum_required(VERSION 3.22.1)

if (RAINY_MOUDLE_USE_SOURCE_BUILD_MODE)
	message("Buiding windows target")
	if (WIN32)
		target_sources(rainy-toolkit PRIVATE
			${PROJECT_SOURCE_DIR}/sources/winapi/system/multithread/thread.cxx
			${PROJECT_SOURCE_DIR}/sources/winapi/system/multithread/mutex.cxx
			#${PROJECT_SOURCE_DIR}/include/rainy/sources/winapi/ui/impl_notifcations.cxx
		)
		target_link_libraries(rainy-toolkit PRIVATE windowsapp)
	endif()
endif()