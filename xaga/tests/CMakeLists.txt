set(rainy_test_suite_files
    ${CMAKE_CURRENT_SOURCE_DIR}/test_suites/test_source_location.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_suites/test_format_wrapper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_suites/test_function_pointer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_suites/test_basic_string.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_suites/test_filesystem_pal.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_suites/pal/test_pal_atomic.cpp
)

add_executable(rainy_toolkit_test test_main.cpp 
    ${rainy_test_suite_files}
)

set_target_properties(rainy_toolkit_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
target_link_libraries(rainy_toolkit_test PRIVATE rainy-toolkit)

target_include_directories(rainy_toolkit_test PUBLIC ${PROJECT_SOURCE_DIR}/third_party/googletest/googletest/include)
target_include_directories(rainy_toolkit_test PUBLIC ${PROJECT_SOURCE_DIR}/third_party/rttr/src/)
target_include_directories(rainy_toolkit_test PUBLIC ${PROJECT_SOURCE_DIR}/third_party/udrefl/include)
target_link_libraries(rainy_toolkit_test PRIVATE rainy-toolkit GTest::gtest)

# 添加测试到 CTest
add_test(NAME rainy_toolkit_test COMMAND rainy_toolkit_test)

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message("Detect Clang compiler or GNU compiler")
    add_compile_options(-mavx2)
endif ()

if (MSVC AND NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message("Detect MSVC compiler")
    add_compile_options(/arch:AVX2)
    if (RAINY_USING_UTF8_INPUT_FOR_MSVC)
        message("Using UTF-8 for input encoding.")
        target_compile_options(rainy_toolkit_test PRIVATE /source-charset:utf-8)
    else()
        message("Using GBK for input encoding.")
        target_compile_options(rainy_toolkit_test PRIVATE /execution-charset:gbk)
    endif()
    if (RAINY_USING_UTF8_OUTPUT_FOR_MSVC)
        message("Using UTF-8 for output encoding.")
        target_compile_options(rainy_toolkit_test PRIVATE /source-charset:utf-8)
    else()
        message("Using GBK for output encoding.")
        target_compile_options(rainy_toolkit_test PRIVATE /execution-charset:gbk)
    endif()
endif ()