install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/cmake/
    DESTINATION lib/cmake/rainy-toolkit
)

install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/xaga/include/
    DESTINATION include
)

install(
    TARGETS rainy-toolkit
    EXPORT rainyToolkitTargets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(
    EXPORT rainyToolkitTargets
    FILE rainyToolkitTargets.cmake
    NAMESPACE rainy::
    DESTINATION lib/cmake/rainy-toolkit
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/rainy-toolkitConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/rainy-toolkitConfigVersion.cmake
    DESTINATION lib/cmake/rainy-toolkit
)