add_subdirectory(context)
add_subdirectory(network)
add_subdirectory(ipc)
add_subdirectory(filesystem)

add_library(rainy-toolkit-foundation-io INTERFACE)
target_link_libraries(rainy-toolkit-foundation-io INTERFACE
    rainy-toolkit-foundation-io-context
    rainy-toolkit-foundation-io-network
    rainy-toolkit-foundation-io-ipc
    rainy-toolkit-foundation-io-filesystem
)

install(TARGETS rainy-toolkit-foundation-io EXPORT rainy-toolkit-targets)