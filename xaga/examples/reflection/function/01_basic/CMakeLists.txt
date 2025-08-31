add_executable(rainy-toolkit-examples-reflection "main.cpp")

set_target_properties(rainy-toolkit-examples-reflection PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
target_link_libraries(rainy-toolkit-examples-reflection PRIVATE rainy-toolkit)
target_link_libraries(rainy-toolkit-examples-reflection PRIVATE magic_enum)
target_link_libraries(rainy-toolkit-examples-reflection PRIVATE Ubpa::UDRefl_core)
#set_target_properties(rttr_core PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)