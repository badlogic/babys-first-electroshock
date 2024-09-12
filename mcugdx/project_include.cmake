# Function to add a custom command and integrate with ESP-IDF build
function(mcugdx_create_rofs_partition partition_name input_dir)
    set(OUTPUT_FILE "${CMAKE_BINARY_DIR}/${partition_name}.bin")

    # Create a custom target for the ROFS binary generation
    add_custom_target(${partition_name}_rofs_bin ALL
        COMMAND ${CMAKE_COMMAND} -E echo "Generating ROFS binary..."
        COMMAND ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/rofs.sh ${OUTPUT_FILE} ${input_dir}
        COMMENT "Building ROFS partition image"
    )

    # Ensure the ROFS binary is added to the list of additional make clean files
    set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" APPEND PROPERTY
        ADDITIONAL_MAKE_CLEAN_FILES
        ${OUTPUT_FILE}
    )

    if(DEFINED ESP_PLATFORM)
        # Flash support similar to littlefs
        partition_table_get_partition_info(size "--partition-name ${partition_name}" "size")
        partition_table_get_partition_info(offset "--partition-name ${partition_name}" "offset")

        if("${size}" AND "${offset}")
            idf_component_get_property(main_args esptool_py FLASH_ARGS)
            idf_component_get_property(sub_args esptool_py FLASH_SUB_ARGS)

            esptool_py_flash_target(${partition_name}-flash "${main_args}" "${sub_args}")
            esptool_py_flash_target_image(${partition_name}-flash "${partition_name}" "${offset}" "${OUTPUT_FILE}")

            add_dependencies(${partition_name}-flash ${partition_name}_rofs_bin)

            esptool_py_flash_target_image(flash "${partition}" "${offset}" "${OUTPUT_FILE}")
            add_dependencies(flash  ${partition_name}_rofs_bin)
        else()
            message(FATAL_ERROR "Partition '${partition_name}' is not defined in the partition table.")
        endif()
    endif()
endfunction()