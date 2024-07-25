set(NS_3RDPARTY_SUPERLUMINAL_SUPPORT OFF CACHE BOOL "Add Support for Superluminal Profiler/Performance.")
set(NS_3RDPARTY_SUPERLUMINAL_PATH "" CACHE PATH "Superluminal Hard Path.")

function(ns_find_profiler_superluminal TARGET)
    if(NS_3RDPARTY_SUPERLUMINAL_SUPPORT)
        find_package(SuperluminalAPI)

        if(SuperluminalAPI_FOUND)
            message(NOTICE "Superluminal Was found, enabling for the engine.")

            if(${CMAKE_BUILD_TYPE} STREQUAL "Dev" OR "Debug")
                target_link_libraries(${TARGET} PUBLIC
                    SuperluminalAPI_LIBS_DEBUG
                )
            else()
                target_link_libraries(${TARGET} PUBLIC
                    SuperluminalAPI_LIBS_RELEASE
                )
            endif()

            target_compile_definitions(${TARGET} PUBLIC SUPERLUMINALAPI=1)
            target_include_directories(${TARGET} PUBLIC SuperluminalAPI_INCLUDE_DIRS)
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${NS_3RDPARTY_SUPERLUMINAL_PATH}/API/dll/x64/PerformanceAPI.dll" $<TARGET_FILE_DIR:${PROJECT_NAME}>
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            )
        else()
            set(ENV_SUPERLUMINAL_PATH $ENV{SUPERLUMINAL_API_DIR})
            if(EXISTS $ENV{SUPERLUMINAL_API_DIR} AND NOT NS_CMAKE_PLATFORM_PLAYSTATION_5)
                set(ENV_SUPERLUMINAL_PATH "$ENV{SUPERLUMINAL_API_DIR}")
                target_link_libraries(${TARGET} PUBLIC
                    ${ENV_SUPERLUMINAL_PATH}/lib/x64/PerformanceAPI_MD.lib
                )

                target_compile_definitions(${TARGET} PUBLIC SUPERLUMINALAPI=1)
                target_include_directories(${TARGET} PUBLIC ${ENV_SUPERLUMINAL_PATH}/include/Superluminal)
                add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ENV_SUPERLUMINAL_PATH}/dll/x64/PerformanceAPI.dll" $<TARGET_FILE_DIR:${PROJECT_NAME}>
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                )
            elseif(NS_3RDPARTY_SUPERLUMINAL_PATH AND NOT NS_CMAKE_PLATFORM_PLAYSTATION_5)
                set(NS_3RDPARTY_SUPERLUMINAL_PATH "${ENV_SUPERLUMINAL_PATH}")
                target_link_libraries(${TARGET} PUBLIC
                    ${NS_3RDPARTY_SUPERLUMINAL_PATH}/lib/x64/PerformanceAPI_MD.lib
                )

                target_compile_definitions(${TARGET} PUBLIC SUPERLUMINALAPI=1)
                target_include_directories(${TARGET} PUBLIC ${NS_3RDPARTY_SUPERLUMINAL_PATH}/include/Superluminal)
                add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${NS_3RDPARTY_SUPERLUMINAL_PATH}/dll/x64/PerformanceAPI.dll" $<TARGET_FILE_DIR:${PROJECT_NAME}>
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                )
            elseif(NS_CMAKE_PLATFORM_PLAYSTATION_5)
                message(NOTICE "Superluminal's API isnt directly supported on PS5, but the PRX is available. forcing it to be used.")
                target_compile_definitions(${TARGET} PUBLIC SUPERLUMINALAPI=1)
            else()
                message(FATAL_ERROR "Couldn't find Superluminal, and 3rd Party path isnt resolved. please set it.")
            endif()
        endif()
    endif()
endfunction()