add_executable(rainy-toolkit-reflect-01-register
	"main.cpp"
)

target_link_libraries(rainy-toolkit-reflect-01-register PRIVATE
	rainy-toolkit meta rttr_core
)

set_target_properties(rainy-toolkit-reflect-01-register PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
target_include_directories(rainy-toolkit-reflect-01-register PUBLIC ${PROJECT_SOURCE_DIR}/third_party/fastio/include)
set_target_properties(rttr_core PROPERTIES  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)