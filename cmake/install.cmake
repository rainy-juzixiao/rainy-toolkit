include(CMakePackageConfigHelpers)
set(RAINY_TOOLKIT_INSTALL_DIR "${CMAKE_BINARY_DIR}/install")

write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/rainy-toolkitConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
)

install(
        DIRECTORY ${PROJECT_SOURCE_DIR}/cmake/
        DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/lib/cmake/rainy-toolkit
)

install(
        EXPORT rainy-toolkit-targets
        FILE rainy-toolkitTargets.cmake
        NAMESPACE rainy::
        DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/lib/cmake/rainy-toolkit
)

install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/rainy-toolkitConfigVersion.cmake
        DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/lib/cmake/rainy-toolkit
)
