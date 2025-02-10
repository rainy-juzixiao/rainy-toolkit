message("Starting adding sources files.")

if (WIN32)
    include(${PROJECT_SOURCE_DIR}/cmake/linker/cross_platform_config/rainy_target_link_windows.cmake)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    include(${PROJECT_SOURCE_DIR}/cmake/linker/cross_platform_config/rainy_target_link_linux.cmake)
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

if (MOUDLE_USE_SOURCE_BUILD_MODE)
    message("Using source build mode. Adding source files to target_sources")
    target_sources(rainy-toolkit PRIVATE 
        ${PROJECT_SOURCE_DIR}/master/sources/rainy_cxxfiles/user/hash/impl_sha.cxx 
        ${PROJECT_SOURCE_DIR}/master/sources/rainy_cxxfiles/diagnostics/contract.cxx
        ${PROJECT_SOURCE_DIR}/master/sources/rainy_cxxfiles/core/core.cxx
    )
endif()

include(${PROJECT_SOURCE_DIR}/cmake/linker/rainy_target_pal_link.cmake)