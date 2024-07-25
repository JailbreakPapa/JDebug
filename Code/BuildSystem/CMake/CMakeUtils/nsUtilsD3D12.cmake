set(NS_D3D12_AGILITY_SDK_DIR "NS_D3D12_AGILITY_SDK_DIR-NOTFOUND" CACHE PATH "Directory of D3D12 Agility SDK")
set(NS_D3D12_AGILITY_SDK_INCLUDE_DIR "NS_D3D12_AGILITY_SDK_INCLUDE_DIR-NOTFOUND" CACHE PATH "Directory of D3D12 Agliity SDK Includes")
set(NS_BUILD_EXPERIMENTAL_D3D12_SUPPORT OFF CACHE BOOL "Add support for D3D12. PC/Xbox")

mark_as_advanced(FORCE NS_D3D12_AGILITY_SDK_DIR)
mark_as_advanced(FORCE NS_D3D12_AGILITY_SDK_INCLUDE_DIR)

# NOTE: This shouldnt be fixed Aglity Version. Find a way to "adjust this."
set(NS_D3D12_AGILITY_SDK_PACKAGE_PATH "${CMAKE_BINARY_DIR}/packages/Microsoft.Direct3D.D3D12.1.611.2/build/native")
set(NS_D3D12_AGILITY_SDK_PACKAGE_PATH_INCLUDE "${NS_D3D12_AGILITY_SDK_PACKAGE_PATH}/include")

macro(ns_requires_d3d12)
    ns_requires_d3d()
    ns_requires(NS_BUILD_EXPERIMENTAL_D3D12_SUPPORT)
endmacro()

function(ns_export_target_dx12 TARGET_NAME)
    
    ns_requires_d3d12()
    # Install D3D12 Aglity SDK for the latest sdk.
    ns_nuget_init()
    
    message("D3D12 SDK DLL PATH: ${NS_D3D12_AGILITY_SDK_PACKAGE_PATH}")
    # Path where d3d12 dlls will end up on build.
    # NOTE: Should we allow the user to change this?
    set(NS_D3D12_RESOURCES ${CMAKE_BINARY_DIR}/x64)

    if(NOT EXISTS ${NS_D3D12_RESOURCES})
        file(MAKE_DIRECTORY ${NS_D3D12_RESOURCES})
    endif()
    
    target_include_directories(${TARGET_NAME} PRIVATE ${NS_D3D12_AGILITY_SDK_PACKAGE_PATH_INCLUDE})

    execute_process(COMMAND ${NUGET} restore ${CMAKE_SOURCE_DIR}/${NS_CMAKE_RELPATH}/CMakeUtils/packages.config -PackagesDirectory ${CMAKE_BINARY_DIR}/packages
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

    add_custom_command(TARGET ${TARGET_NAME}
	PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${NS_D3D12_AGILITY_SDK_PACKAGE_PATH}/bin/x64/D3D12Core.dll ${NS_D3D12_RESOURCES}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${NS_D3D12_AGILITY_SDK_PACKAGE_PATH}/bin/x64/d3d12SDKLayers.dll ${NS_D3D12_RESOURCES}
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

endfunction()

# #####################################
# ## ns_link_target_dx12(<target>)
# #####################################
function(ns_link_target_dx12 TARGET_NAME)
    # ns_export_target_dx12(${TARGET_NAME})
    target_link_libraries(${TARGET_NAME}
    PRIVATE
    RendererDX12
    )
endfunction()