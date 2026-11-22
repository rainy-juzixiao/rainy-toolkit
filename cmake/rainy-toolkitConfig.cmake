include("${CMAKE_CURRENT_LIST_DIR}/rainy-toolkitTargets.cmake")

get_filename_component(_rainy_dir "${CMAKE_CURRENT_LIST_FILE}" PATH)

include("${_rainy_dir}/utility.cmake")
include("${_rainy_dir}/automoc.cmake")

unset(_rainy_dir)
