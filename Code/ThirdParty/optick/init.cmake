# Optick Utils


function(wd_export_optick TARGET_NAME)
add_custom_command(TARGET ${PROJECT_NAME}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Optick.exe $<TARGET_FILE_DIR:${TARGET_NAME}>
    WORKING_DIRECTORY ${CMAKE_CURRENT_FUNCTION_LIST_DIR}
)
endfunction()