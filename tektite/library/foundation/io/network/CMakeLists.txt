project(rainy-toolkit-foundation-io-network)

rainy_configure_target(rainy-toolkit-foundation-io-network)

target_include_directories(rainy-toolkit-foundation-io-network PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

set(NETWORK_SOURCES "")
rainy_load_flodar_files("${CMAKE_CURRENT_SOURCE_DIR}/sources" ".cxx" NETWORK_SOURCES)
target_sources(rainy-toolkit-foundation-io-network PRIVATE ${NETWORK_SOURCES})

target_link_libraries(rainy-toolkit-foundation-io-network PUBLIC rainy-toolkit-foundation-io-context)

find_package(OpenSSL QUIET)
if (OpenSSL_FOUND)
    target_link_libraries(rainy-toolkit-foundation-io-network PRIVATE OpenSSL::SSL OpenSSL::Crypto)
    target_compile_definitions(rainy-toolkit-foundation-io-network PUBLIC RAINY_HAS_OPENSSL=1)
else ()
    target_compile_definitions(rainy-toolkit-foundation-io-network PUBLIC RAINY_HAS_OPENSSL=0)
    message(STATUS "OpenSSL not found, building foundation-io-network without TLS support")
endif ()

set(RAINY_FOUNDATION_IO_NETWORK_TEST_FILES "")
rainy_load_flodar_files("${CMAKE_CURRENT_SOURCE_DIR}/tests" ".cc" RAINY_FOUNDATION_IO_NETWORK_TEST_FILES)
add_executable(rainy-toolkit-foundation-io-network-unit-test ${RAINY_FOUNDATION_IO_NETWORK_TEST_FILES})
target_link_libraries(rainy-toolkit-foundation-io-network-unit-test PRIVATE Catch2::Catch2WithMain rainy-toolkit-foundation-io-network)
catch_discover_tests(rainy-toolkit-foundation-io-network-unit-test)
set_target_properties(rainy-toolkit-foundation-io-network-unit-test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
unset(RAINY_FOUNDATION_IO_NETWORK_TEST_FILES)

if (OpenSSL_FOUND)
target_link_libraries(
        rainy-toolkit-foundation-io-network-unit-test
        PRIVATE
        OpenSSL::SSL
        OpenSSL::Crypto
)
endif ()

install(TARGETS rainy-toolkit-foundation-io-network EXPORT rainy-toolkit-targets
        RUNTIME DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/bin
        LIBRARY DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/lib
        ARCHIVE DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/lib)

install(DIRECTORY include/ DESTINATION ${RAINY_TOOLKIT_INSTALL_DIR}/include)

if (WIN32)
    target_link_libraries(rainy-toolkit-foundation-io-network PRIVATE mswsock)
endif ()

unset(NETWORK_SOURCES)