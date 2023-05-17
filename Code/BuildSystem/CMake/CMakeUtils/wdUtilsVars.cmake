# #####################################
# ## Output directories
# #####################################
set(WD_OUTPUT_DIRECTORY_LIB "${CMAKE_SOURCE_DIR}/Output/Lib" CACHE PATH "Where to store the compiled .lib files.")
set(WD_OUTPUT_DIRECTORY_DLL "${CMAKE_SOURCE_DIR}/Output/Bin" CACHE PATH "Where to store the compiled .dll files.")

mark_as_advanced(FORCE WD_OUTPUT_DIRECTORY_LIB)
mark_as_advanced(FORCE WD_OUTPUT_DIRECTORY_DLL)

# #####################################
# ## PCH support
# #####################################
set(WD_USE_PCH ON CACHE BOOL "Whether to use Precompiled Headers.")

mark_as_advanced(FORCE WD_USE_PCH)

# #####################################
# ## Folder Unity files
# #####################################
set(WD_ENABLE_FOLDER_UNITY_FILES ON CACHE BOOL "Whether unity cpp files should be created per folder")

mark_as_advanced(FORCE WD_ENABLE_FOLDER_UNITY_FILES)

# #####################################
# ## PVS Studio support
# #####################################
set(WD_ENABLE_PVS_STUDIO_HEADER_IN_UNITY_FILES ON CACHE BOOL "Adds the necessary comment to the generated unity files for PVS checking")

mark_as_advanced(FORCE WD_ENABLE_PVS_STUDIO_HEADER_IN_UNITY_FILES)

# #####################################
# ## Static analysis support
# #####################################
set(WD_ENABLE_COMPILER_STATIC_ANALYSIS OFF CACHE BOOL "Enables static analysis in the compiler options")

mark_as_advanced(FORCE WD_ENABLE_COMPILER_STATIC_ANALYSIS)

# #####################################
# ## vcpkg
# #####################################

# ## Qt
# set (WD_VCPKG_INSTALL_QT OFF CACHE BOOL "Whether to install Qt via vcpkg.")
