add_executable(app "main.cc")
target_link_libraries(app PUBLIC rainy-toolkit)

get_target_property(APP_SRCS app SOURCES)
