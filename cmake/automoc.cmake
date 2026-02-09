if(RAINY_AUTOMOC_INCLUDED)
    return()
endif()
set(RAINY_AUTOMOC_INCLUDED TRUE)

find_program(RAINY_MOC_EXECUTABLE
        NAMES prism-moc gen
        PATHS
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_SOURCE_DIR}/tools"
        "${CMAKE_BINARY_DIR}"
)

if(NOT RAINY_MOC_EXECUTABLE)
    message("prism-moc not found! Disable automoc")
    return()
endif()

message(STATUS "Found RAINY_MOC_EXECUTABLE: ${RAINY_MOC_EXECUTABLE}")

define_property(TARGET
        PROPERTY RAINY_AUTOMOC
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

        # 不存在的直接原样保留
        if(NOT EXISTS "${SRC_ABS}")
            list(APPEND NEW_SOURCES "${SRC}")
            continue()
        endif()

        file(READ "${SRC_ABS}" CONTENT)
        if(CONTENT MATCHES "RAINY_REFLECTION|RAINY_ENABLE_MOC")
            get_filename_component(NAME "${SRC_ABS}" NAME_WE)
            set(MOC_OUT "${CMAKE_CURRENT_BINARY_DIR}/moc_${NAME}.cpp")
            # debug
            message(STATUS "Will generate: ${MOC_OUT}")
            message(STATUS "  From: ${SRC_ABS}")
            add_custom_command(
                    OUTPUT "${MOC_OUT}"
                    COMMAND ${CMAKE_COMMAND} -E echo "Running prism-moc on ${SRC_ABS}"
                    COMMAND "${RAINY_MOC_EXECUTABLE}" --input "${SRC_ABS}" --out "${MOC_OUT}"
                    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
                    DEPENDS "${SRC_ABS}" "${RAINY_MOC_EXECUTABLE}"
                    COMMENT "Generating moc_${NAME}.cpp from ${NAME}"
                    VERBATIM
            )
            # ⭐ 确保文件被标记为 GENERATED
            set_source_files_properties("${MOC_OUT}" PROPERTIES
                    GENERATED TRUE
                    SKIP_AUTOGEN TRUE
            )

            list(APPEND GENERATED_SOURCES "${MOC_OUT}")
        else()
            list(APPEND NEW_SOURCES "${SRC}")
        endif()
    endforeach()

    if(GENERATED_SOURCES)
        # ⭐ 保留原始源文件 + 添加生成的文件
        list(APPEND NEW_SOURCES ${GENERATED_SOURCES})
        set_property(TARGET ${TARGET} PROPERTY SOURCES "${NEW_SOURCES}")

        message(STATUS "Target ${TARGET} will generate ${GENERATED_SOURCES}")
    endif()
endfunction()

function(_rainy_automoc_finalize)
    get_property(ALL_TARGETS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)
    foreach(TARGET ${ALL_TARGETS})
        get_target_property(AUTOMOC_ENABLED ${TARGET} RAINY_AUTOMOC)
        if(AUTOMOC_ENABLED)
            message(STATUS "Processing AUTOMOC for target: ${TARGET}")
            _rainy_process_automoc(${TARGET})
        endif()
    endforeach()
endfunction()

cmake_language(DEFER CALL _rainy_automoc_finalize)

message(STATUS "Rainy AUTOMOC loaded")