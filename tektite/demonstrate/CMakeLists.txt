add_executable(rainy-toolkit-demo-tekkitte "main.cpp")

set_target_properties(rainy-toolkit-demo-tekkitte PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
target_link_libraries(rainy-toolkit-demo-tekkitte PRIVATE rainy-toolkit-core-platform)

if (RAINY_USE_CROSSCOMPILE)
    set_target_properties(rainy-toolkit-demo-tekkitte PROPERTIES LINK_FLAGS "-static")
endif ()

if (MSVC)
    target_compile_options(rainy-toolkit-demo-tekkitte PRIVATE /source-charset:utf-8)
    target_compile_options(rainy-toolkit-demo-tekkitte PRIVATE /execution-charset:utf-8)
endif ()

if (RAINY_TOOLKIT_HAVE_CXX26_STATIC_REFLECTION)
    target_compile_features(rainy-toolkit-demo-tekkitte PUBLIC cxx_std_26)
    if (COMPILER_ID MATCHES "GCC")
        target_compile_options(rainy-toolkit-demo-tekkitte PUBLIC -freflection)
    endif ()
endif ()

#set_target_properties(rttr_core PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)