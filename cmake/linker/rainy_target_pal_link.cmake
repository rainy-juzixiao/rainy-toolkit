if (MOUDLE_USE_SOURCE_BUILD_MODE)
	target_sources(rainy-toolkit PRIVATE 
		${PROJECT_SOURCE_DIR}/master/sources/rainy_cxxfiles/foundation/pal/threading.cxx
	)
endif()