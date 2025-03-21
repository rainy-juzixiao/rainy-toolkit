message("Starting adding sources files.")

if (WIN32)
    include(${PROJECT_SOURCE_DIR}/cmake/linker/cross_platform_config/rainy_target_link_windows.cmake)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    include(${PROJECT_SOURCE_DIR}/cmake/linker/cross_platform_config/rainy_target_link_linux.cmake)
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

target_sources(rainy-toolkit PRIVATE 
    ${PROJECT_SOURCE_DIR}/sources/user/hash/impl_sha.cxx 
    ${PROJECT_SOURCE_DIR}/sources/diagnostics/contract.cxx
    ${PROJECT_SOURCE_DIR}/sources/core/core.cxx
    ${PROJECT_SOURCE_DIR}/sources/core/pal_filesystem.cxx
)

include(${PROJECT_SOURCE_DIR}/cmake/linker/rainy_target_pal_link.cmake)