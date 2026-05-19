rainy_load_flodar_files("${PROJECT_SOURCE_DIR}/xaga/tests/test_suites" ".cc" RAINY_TEST_SUITE_FILES)

add_executable(rainy-toolkit-test ${RAINY_TEST_SUITE_FILES})

set_target_properties(rainy-toolkit-test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set_target_properties(rainy-toolkit-test PROPERTIES EXCLUDE_FROM_ALL OFF)
target_link_libraries(rainy-toolkit-test PRIVATE rainy-toolkit)

target_link_libraries(rainy-toolkit-test PRIVATE rainy-toolkit Catch2::Catch2WithMain)

if (RAINY_USE_CROSSCOMPILE)
    set_target_properties(rainy-toolkit-test PROPERTIES LINK_FLAGS "-static")
endif ()

include(Catch)
catch_discover_tests(rainy-toolkit-test)

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message("Detect Clang compiler or GNU compiler")
    add_compile_options(-mavx2)
endif ()

if (MSVC AND NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message("Detect MSVC compiler")
    add_compile_options(/arch:AVX2)
    if (RAINY_USING_UTF8_INPUT_FOR_MSVC)
        message("Using UTF-8 for input encoding.")
        target_compile_options(rainy-toolkit-test PRIVATE /source-charset:utf-8)
    else ()
        message("Using GBK for input encoding.")
        target_compile_options(rainy-toolkit-test PRIVATE /execution-charset:gbk)
    endif ()
    if (RAINY_USING_UTF8_OUTPUT_FOR_MSVC)
        message("Using UTF-8 for output encoding.")
        target_compile_options(rainy-toolkit-test PRIVATE /source-charset:utf-8)
    else ()
        message("Using GBK for output encoding.")
        target_compile_options(rainy-toolkit-test PRIVATE /execution-charset:gbk)
    endif ()
endif ()

find_package(OpenSSL)
if(OpenSSL_FOUND)
    target_link_libraries(rainy-toolkit-test PRIVATE OpenSSL::SSL OpenSSL::Crypto)
    target_compile_definitions(rainy-toolkit-test PRIVATE RAINY_HAS_OPENSSL=1)

    if(WIN32 AND MSVC)
        find_file(OPENSSL_APPLINK_FILE
            NAMES applink.c
            PATHS
                "${OPENSSL_INCLUDE_DIR}/openssl"
                "${OPENSSL_INCLUDE_DIR}/../openssl"
                "${OPENSSL_INCLUDE_DIR}/../"
            NO_DEFAULT_PATH
        )

        if(OPENSSL_APPLINK_FILE)
            message(STATUS "Found OpenSSL applink.c: ${OPENSSL_APPLINK_FILE}")
            target_sources(rainy-toolkit-test PRIVATE ${OPENSSL_APPLINK_FILE})
            set_source_files_properties(${OPENSSL_APPLINK_FILE}
                PROPERTIES LANGUAGE C
            )
        else()
            message(WARNING "Cannot find applink.c under ${OPENSSL_INCLUDE_DIR}")
            if(TARGET OpenSSL::applink)
                target_link_libraries(rainy-toolkit-test PRIVATE OpenSSL::applink)
            endif()
        endif()
    endif()
else()
    target_compile_definitions(rainy-toolkit-test PRIVATE RAINY_HAS_OPENSSL=0)
endif()