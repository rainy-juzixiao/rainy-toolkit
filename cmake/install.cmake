set(RAINY_TOOLKIT_INSTALL_DIR "${PROJECT_BINARY_DIR}/install")

install(
        DIRECTORY ${PROJECT_SOURCE_DIR}/cmake/
        DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/lib/cmake/rainy-toolkit
)

install(
        DIRECTORY ${PROJECT_SOURCE_DIR}/xaga/include/
        DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/include
)

install(
        TARGETS rainy-toolkit
        EXPORT rainyToolkitTargets
        RUNTIME DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/bin
        LIBRARY DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/lib
        ARCHIVE DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/lib
)

install(
        EXPORT rainyToolkitTargets
        FILE rainyToolkitTargets.cmake
        NAMESPACE rainy::
        DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/lib/cmake/rainy-toolkit
)

install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/rainy-toolkitConfigVersion.cmake
        DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/lib/cmake/rainy-toolkit
)
