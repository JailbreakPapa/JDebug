ns_cmake_init()

# Get the name of this folder as the project name
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

ns_glob_source_files(${CMAKE_CURRENT_SOURCE_DIR} ALL_SOURCE_FILES)
add_custom_target(${PROJECT_NAME} SOURCES "${ALL_SOURCE_FILES}")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ALL_SOURCE_FILES})

ns_set_project_ide_folder(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR})
