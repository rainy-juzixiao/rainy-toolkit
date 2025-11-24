get_filename_component(_rainy_dir "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(_rainy_prefix "${_rainy_dir}" PATH)
get_filename_component(_rainy_prefix "${_rainy_prefix}" PATH)

list(APPEND CMAKE_MODULE_PATH "${_rainy_dir}")

include("${_rainy_dir}/utility.cmake")
include("${_rainy_dir}/rainyToolkitTargets.cmake")

set(rainy-toolkit_INCLUDE_DIRS "${_rainy_prefix}/include")
set(rainy-toolkit_FOUND TRUE)

unset(_rainy_dir)
unset(_rainy_prefix)