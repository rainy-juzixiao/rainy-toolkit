include("${CMAKE_CURRENT_LIST_DIR}/rainyToolkitTargets.cmake")
get_filename_component(_RAINY_INSTALL_PREFIX "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)

set_target_properties(rainy::rainy-toolkit PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${_RAINY_INSTALL_PREFIX}/include"
)

get_filename_component(_rainy_dir "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(_rainy_prefix "${_rainy_dir}" PATH)
get_filename_component(_rainy_prefix "${_rainy_prefix}" PATH)

include("${_rainy_dir}/utility.cmake")
include("${_rainy_dir}/automoc.cmake")

unset(_rainy_dir)

unset(_RAINY_INSTALL_PREFIX)