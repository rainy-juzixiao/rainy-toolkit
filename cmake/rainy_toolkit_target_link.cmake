message("Starting adding sources files.")

if (WIN32)
    include(${PROJECT_SOURCE_DIR}/cmake/muti_platfrom_config/rainy_target_link_windows.cmake)
    target_link_libraries(rainy-toolkit PRIVATE rainy_toolkit_winapi_link_libray)
endif()

if (COMMON_MOUDLE_USE_SOURCE_BUILD_MODE)
    message("Using source build mode. Adding source files to target_sources")
    target_sources(rainy-toolkit PRIVATE 
        ${PROJECT_SOURCE_DIR}/include/rainy/sources/user/hash/impl_sha.cxx 
        ${PROJECT_SOURCE_DIR}/include/rainy/sources/diagnostics/contract.cxx
        ${PROJECT_SOURCE_DIR}/include/rainy/sources/core.cxx
    )
endif()