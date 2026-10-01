project(rainy-toolkit-foundation-willow)

rainy_configure_target(rainy-toolkit-foundation-willow)

target_include_directories(rainy-toolkit-foundation-willow PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

rainy_load_flodar_files("${CMAKE_CURRENT_SOURCE_DIR}/sources" ".cxx" WILLOW_SOURCES)
target_sources(rainy-toolkit-foundation-willow PRIVATE ${WILLOW_SOURCES})

target_link_libraries(rainy-toolkit-foundation-willow PUBLIC
        rainy-toolkit-core
        rainy-toolkit-foundation-collections
)

unset(WILLOW_SOURCES)

install(TARGETS rainy-toolkit-foundation-willow EXPORT rainy-toolkit-targets
        RUNTIME DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/bin
        LIBRARY DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/lib
        ARCHIVE DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/lib)

install(DIRECTORY include/ DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/include)
