add_executable(rainy-toolkit-examples-reflection "main.cpp" "header.h")

set_target_properties(rainy-toolkit-examples-reflection PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
target_link_libraries(rainy-toolkit-examples-reflection PRIVATE rainy-toolkit)

if (RAINY_USE_CROSSCOMPILE)
    set_target_properties(rainy-toolkit-examples-reflection PROPERTIES LINK_FLAGS "-static")
endif ()

if (MSVC)
    target_compile_options(rainy-toolkit-examples-reflection PRIVATE /source-charset:utf-8)
    target_compile_options(rainy-toolkit-examples-reflection PRIVATE /execution-charset:utf-8)
endif ()

if (RAINY_TOOLKIT_HAVE_CXX26_STATIC_REFLECTION)
    target_compile_features(rainy-toolkit-examples-reflection PUBLIC cxx_std_26)
    if (COMPILER_ID MATCHES "GCC")
        target_compile_options(rainy-toolkit-examples-reflection PUBLIC -freflection)
    endif ()
endif ()

#set_target_properties(rttr_core PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)