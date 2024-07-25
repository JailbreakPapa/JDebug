set(SAMPLES_BACKEND "auto" CACHE STRING "Backend platform and renderer used for the samples.")
message(NOTICE "SET CMAKE_TOOLCHAIN_FILE TO ${NS_ROOT}/Code/ThirdParty/freetype/scripts/buildsystems/vcpkg.cmake!")
function(evaluate_js_engine_platform PROJECT_NAME)
    if(NS_CMAKE_PLATFORM_CONSOLE)
        target_link_libraries(${PROJECT_NAME} PUBLIC apertureuijsc)
    else()
        target_link_libraries(${PROJECT_NAME} PUBLIC apertureuiv8)
    endif()
endfunction()
# set(CMAKE_TOOLCHAIN_FILE "${NS_ROOT}/Code/ThirdParty/freetype/scripts/buildsystems/vcpkg.cmake")
function(export_folder_contents SOURCE_FOLDER DESTINATION_FOLDER)
    # Validate arguments
    if(NOT IS_DIRECTORY "${SOURCE_FOLDER}")
        message(ERROR "Source folder '${SOURCE_FOLDER}' does not exist")
        return()
    endif()

    if(NOT IS_DIRECTORY "${DESTINATION_FOLDER}")
        message(ERROR "Destination folder '${DESTINATION_FOLDER}' does not exist")
        return()
    endif()

    # Use file(GLOB_RECURSE) for efficiency
    file(GLOB_RECURSE FILES RELATIVE "${SOURCE_FOLDER}" "${SOURCE_FOLDER}/*")

    # Install each file
    foreach(FILE IN FILES)
        install(FILES "${SOURCE_FOLDER}/${FILE}"
            DESTINATION "${DESTINATION_FOLDER}"
            PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ)
    endforeach()

    # Optionally create an export file (can be customized)
    set(EXPORT_FILE "${DESTINATION_FOLDER}/CMakeLists.txt")
    file(WRITE "${EXPORT_FILE}" "# Exported contents from folder '${SOURCE_FOLDER}'")
    install(FILES "${EXPORT_FILE}"
        DESTINATION "${DESTINATION_FOLDER}"
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ)

    message(STATUS "Exported folder '${SOURCE_FOLDER}' to '${DESTINATION_FOLDER}'")
endfunction()

function(export_folder source_folder destination_folder PROJECT_NAME)
    if(NOT EXISTS "${source_folder}")
        message(ERROR "Source folder '${source_folder}' does not exist.")
        return()
    endif()

    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${source_folder}" "${destination_folder}"
        DEPENDS "${source_folder}"
        COMMENT "Exporting folder '${source_folder}' to '${destination_folder}'"
    )
endfunction()
# NO NEED FOR THIS FUNCTION NOW.
function(ns_setup_dependancys_apui PROJECT_NAME)
        #add_custom_command(
        #    TARGET ALL TARGET ALL DOES NOT WORK!
        #    POST_BUILD
        #    export_folder_contents(${NS_ROOT}/Data/Samples ${CMAKE_BINARY_DIR}../Samples)
        #)
        export_folder(${NS_ROOT}/Data/Samples $<TARGET_FILE_DIR:${PROJECT_NAME}>/Samples ${PROJECT_NAME})
endfunction()

if(BUILD_SAMPLES)
    # Add platform defines.
    if(SAMPLES_BACKEND MATCHES "^SDL")
        if(EMSCRIPTEN)
            set(EMSCRIPTEN_FLAGS "${EMSCRIPTEN_FLAGS} -sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sSDL2_IMAGE_FORMATS='[tga]'")
        endif()
    endif()

    if(SAMPLES_BACKEND MATCHES "^GLFW")
        add_compile_definitions(APUI_RENDERER_GL3)
    endif()

    # Add renderer dependencies.
    if(SAMPLES_BACKEND MATCHES "GL2$")
        add_compile_definitions(APUI_RENDERER_GL2)

        if(APPLE)
            add_compile_definitions(GL_SILENCE_DEPRECATION)
        endif()
    endif()

    if(SAMPLES_BACKEND MATCHES "VK$")
        message("-- Using Vulkan renderer backend.")
        add_compile_definitions(APUI_RENDERER_VK)
        option(APUI_VK_DEBUG "Enable debugging mode for Vulkan renderer." OFF)
        mark_as_advanced(APUI_VK_DEBUG)

        if(APUI_VK_DEBUG)
            add_compile_definitions(APUI_VK_DEBUG)
        endif()
    endif()

    if(SAMPLES_BACKEND MATCHES "GL3$")
        message("-- Adding OpenGL 3 renderer backend.")

        if(EMSCRIPTEN)
            set(EMSCRIPTEN_EXE_FLAGS "${EMSCRIPTEN_EXE_FLAGS} -sMAX_WEBGL_VERSION=2")
        else()
            add_compile_definitions(APUI_RENDERER_GL3)

            if(APPLE)
                add_compile_definitions(GL_SILENCE_DEPRECATION)
            endif()
        endif()
    endif()
endif()