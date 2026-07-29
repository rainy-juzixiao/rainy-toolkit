include_guard()

macro(rainy_validate_compiler _var _name)
    if("${${_var}}" STREQUAL "")
        message(FATAL_ERROR "${_name} is not set. Please set ${_var}.")
    elseif(NOT EXISTS "${${_var}}")
        message(FATAL_ERROR "${_name} = \"${${_var}}\" does not exist.")
    endif()
endmacro()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
