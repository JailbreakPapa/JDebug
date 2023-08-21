# #####################################
# ## wd_export_target_dx12(<target>)
# #####################################
# #####################################
# ## Exports D3D12 Dlls to Target Project.
# #####################################
set(WD_EXPERMENTAL_D3D12_SUPPORT ON CACHE BOOL "Add support for D3D12. PC/Xbox")
macro(wd_requires_d3d12)
    wd_requires_d3d()
    wd_requires(WD_EXPERMENTAL_D3D12_SUPPORT)
endmacro()
function(wd_export_target_dx12 TARGET_NAME)
    
    wd_requires_d3d12()
    # Install D3D12 Aglity SDK for the latest sdk.
    wd_nuget_init()
    
    message("D3D12 SDK DLL PATH: ${CMAKE_SOURCE_DIR}/${WD_CMAKE_RELPATH_CODE}/ThirdParty/D3D12SDK")
    # Path where d3d12 dlls will end up on build.
    set(WD_D3D12_RESOURCES ${WD_OUTPUT_DIRECTORY_DLL}/${WD_CMAKE_PLATFORM_PREFIX}${WD_CMAKE_GENERATOR_PREFIX}${WD_CMAKE_COMPILER_POSTFIX}${CMAKE_BUILD_TYPE}${WD_CMAKE_ARCHITECTURE_POSTFIX}/x64)

    if(NOT EXISTS ${WD_D3D12_RESOURCES})
        file(MAKE_DIRECTORY ${WD_D3D12_RESOURCES})
    endif()

    file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/${WD_CMAKE_RELPATH_CODE}/ThirdParty/D3D12SDK)

    add_custom_command(TARGET ${PROJECT_NAME}
    PRE_BUILD
    COMMAND ${NUGET} restore ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/packages.config -PackagesDirectory ${CMAKE_SOURCE_DIR}/${WD_CMAKE_RELPATH_CODE}/ThirdParty/D3D12SDK
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    add_custom_command(TARGET ${TARGET_NAME}
	PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/${WD_CMAKE_RELPATH_CODE}/ThirdParty/D3D12SDK/Microsoft.Direct3D.D3D12.1.610.4/build/native/bin/x64/D3D12Core.dll ${WD_D3D12_RESOURCES}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/${WD_CMAKE_RELPATH_CODE}/ThirdParty/D3D12SDK/Microsoft.Direct3D.D3D12.1.610.4/build/native/bin/x64/d3d12SDKLayers.dll ${WD_D3D12_RESOURCES}
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/${WD_CMAKE_RELPATH_CODE}/ThirdParty/D3D12SDK/Microsoft.Direct3D.D3D12.1.610.4/build/native/include)
endfunction()

# #####################################
# ## wd_link_target_dx12(<target>)
# #####################################
function(wd_link_target_dx12 TARGET_NAME)
    # wd_export_target_dx12(${TARGET_NAME})
    target_link_libraries(${TARGET_NAME}
    PRIVATE
    RendererDX12
    )
endfunction()