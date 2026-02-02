# RainyAutoMoc.cmake

if(RAINY_AUTOMOC_INCLUDED)
    return()
endif()
set(RAINY_AUTOMOC_INCLUDED TRUE)

find_program(RAINY_MOC_EXECUTABLE
    NAMES gen gen.exe
    PATHS "${CMAKE_CURRENT_SOURCE_DIR}"
)

if(NOT RAINY_MOC_EXECUTABLE)
    message("gen.exe not found!")
    return()
endif()

define_property(TARGET PROPERTY RAINY_AUTOMOC
    BRIEF_DOCS "Enable automatic moc generation"
    FULL_DOCS "Enable automatic moc generation for this target"
)

function(_rainy_process_automoc TARGET)
    get_target_property(ORIG_SOURCES ${TARGET} SOURCES)
    if(NOT ORIG_SOURCES)
        return()
    endif()

    set(NEW_SOURCES)
    set(GENERATED_SOURCES)

    foreach(SRC ${ORIG_SOURCES})
        get_filename_component(SRC_ABS "${SRC}" ABSOLUTE)

        # 不存在的直接原样保留（比如已生成的）
        if(NOT EXISTS "${SRC_ABS}")
            list(APPEND NEW_SOURCES "${SRC}")
            continue()
        endif()

        file(READ "${SRC_ABS}" CONTENT)

        if(CONTENT MATCHES "RAINY_REFLECTION|RAINY_ENABLE_MOC")
            get_filename_component(NAME "${SRC_ABS}" NAME_WE)
            set(MOC_OUT "${CMAKE_CURRENT_BINARY_DIR}/moc_${NAME}.cpp")

            add_custom_command(
                OUTPUT "${MOC_OUT}"
                COMMAND "${RAINY_MOC_EXECUTABLE}"
                WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
                DEPENDS "${SRC_ABS}"
                COMMENT "Generating moc_${NAME}.cpp"
                VERBATIM
            )

            # 只加入生成文件，不保留原 SRC
            list(APPEND GENERATED_SOURCES "${MOC_OUT}")
        else()
            list(APPEND NEW_SOURCES "${SRC}")
        endif()
    endforeach()

    if(GENERATED_SOURCES)
        list(APPEND NEW_SOURCES ${GENERATED_SOURCES})
        set_property(TARGET ${TARGET} PROPERTY SOURCES "${NEW_SOURCES}")
        set_source_files_properties(${GENERATED_SOURCES} PROPERTIES GENERATED TRUE)
    endif()
endfunction()

function(_rainy_automoc_finalize)
    get_property(ALL_TARGETS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)
    
    foreach(TARGET ${ALL_TARGETS})
        get_target_property(AUTOMOC_ENABLED ${TARGET} RAINY_AUTOMOC)
        if(AUTOMOC_ENABLED)
            _rainy_process_automoc(${TARGET})
        endif()
    endforeach()
endfunction()

cmake_language(DEFER CALL _rainy_automoc_finalize)

message(STATUS "Rainy AUTOMOC loaded")