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
    message(STATUS "prism-moc not found! Disable automoc")
    return()
endif()

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
    get_property(ALL_TARGETS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)
    set(NEW_SOURCES)
    set(MOC_INPUTS)  # 需要 moc 的源文件列表
    set(MOC_OUTPUTS) # 生成的 moc 文件列表

    foreach(SRC ${ORIG_SOURCES})
        get_filename_component(SRC_ABS "${SRC}" ABSOLUTE)

        # 不存在的直接原样保留
        if(NOT EXISTS "${SRC_ABS}")
            list(APPEND NEW_SOURCES "${SRC}")
            continue()
        endif()

        # 获取文件扩展名
        get_filename_component(SRC_EXT "${SRC_ABS}" EXT)

        file(READ "${SRC_ABS}" CONTENT)
        if(CONTENT MATCHES "RAINY_ENABLE_MOC")
            get_filename_component(NAME "${SRC_ABS}" NAME_WE)
            set(MOC_OUT "${CMAKE_CURRENT_BINARY_DIR}/rainy_moc/${NAME}.moc.cpp")

            # 生成单个 moc 文件
            add_custom_command(
                    OUTPUT "${MOC_OUT}"
                    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/rainy_moc"
                    COMMAND "${RAINY_MOC_EXECUTABLE}" --input "${SRC_ABS}" --out "${MOC_OUT}"
                    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
                    DEPENDS "${SRC_ABS}" "${RAINY_MOC_EXECUTABLE}"
                    COMMENT "Generating ${NAME}.moc.cpp"
                    VERBATIM
            )

            set_source_files_properties("${MOC_OUT}" PROPERTIES
                    GENERATED TRUE
                    SKIP_AUTOGEN TRUE
            )

            list(APPEND MOC_INPUTS "${SRC_ABS}")
            list(APPEND MOC_OUTPUTS "${MOC_OUT}")
            # 如果是 .cpp/.cc/.cxx 等源文件，从构建列表中移除
            if(SRC_EXT MATCHES "\\.(cpp|cxx|cc|c)$")
            else()
                # 头文件等保留
                list(APPEND NEW_SOURCES "${SRC}")
            endif()
        else()
            list(APPEND NEW_SOURCES "${SRC}")
        endif()
    endforeach()
    get_target_property(ORIG_SOURCES ${TARGET} SOURCES)
    foreach(SRC ${ORIG_SOURCES})
        get_filename_component(SRC_ABS "${SRC}" ABSOLUTE)
        if(NOT EXISTS "${SRC_ABS}")
            continue()
        endif()

        file(READ "${SRC_ABS}" CONTENT)
    endforeach()


    if(MOC_OUTPUTS)
        # 生成 mocs_compilation.cpp
        set(MOCS_COMPILATION "${CMAKE_CURRENT_BINARY_DIR}/rainy_moc/mocs_compilation.cpp")

        add_custom_command(
                OUTPUT "${MOCS_COMPILATION}"
                COMMAND ${CMAKE_COMMAND} -E echo "// Auto-generated mocs compilation file" > "${MOCS_COMPILATION}"
                COMMAND ${CMAKE_COMMAND} -E echo "" >> "${MOCS_COMPILATION}"
                DEPENDS ${MOC_OUTPUTS}
                COMMENT "Generating mocs_compilation.cpp"
                VERBATIM
        )

        # 为每个 moc 文件添加 #include
        foreach(MOC_FILE ${MOC_OUTPUTS})
            get_filename_component(MOC_NAME "${MOC_FILE}" NAME)
            add_custom_command(
                    OUTPUT "${MOCS_COMPILATION}"
                    COMMAND ${CMAKE_COMMAND} -E echo "#include \"${MOC_NAME}\"" >> "${MOCS_COMPILATION}"
                    APPEND
            )
        endforeach()

        set_source_files_properties("${MOCS_COMPILATION}" PROPERTIES
                GENERATED TRUE
                SKIP_AUTOGEN TRUE
        )
        # 只添加 mocs_compilation.cpp 到构建
        list(APPEND NEW_SOURCES "${MOCS_COMPILATION}")
        set_property(TARGET ${TARGET} PROPERTY SOURCES "${NEW_SOURCES}")
    else()
        # 没有需要 moc 的文件，保持原样
        set_property(TARGET ${TARGET} PROPERTY SOURCES "${NEW_SOURCES}")
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

cmake_language(DEFER DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" CALL _rainy_automoc_finalize)

message(STATUS "Rainy AUTOMOC loaded")