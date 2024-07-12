set_property(GLOBAL PROPERTY NS_BUILDTYPENAME_DEBUG "Debug")
set_property(GLOBAL PROPERTY NS_BUILDTYPENAME_DEV "Dev")
set_property(GLOBAL PROPERTY NS_BUILDTYPENAME_RELEASE "Shipping")

set_property(GLOBAL PROPERTY NS_BUILDTYPENAME_DEBUG_UPPER "DEBUG")
set_property(GLOBAL PROPERTY NS_BUILDTYPENAME_DEV_UPPER "DEV")
set_property(GLOBAL PROPERTY NS_BUILDTYPENAME_RELEASE_UPPER "SHIPPING")

set_property(GLOBAL PROPERTY NS_DEV_BUILD_LINKERFLAGS "DEBUG")

set_property(GLOBAL PROPERTY NS_CMAKE_RELPATH "Code/BuildSystem/CMake")
set_property(GLOBAL PROPERTY NS_CMAKE_RELPATH_CODE "Code")

set_property(GLOBAL PROPERTY NS_CONFIG_PATH_7ZA "${CMAKE_SOURCE_DIR}/Data/Tools/Precompiled/7z.exe")

set_property(GLOBAL PROPERTY NS_CONFIG_QT_WINX64_VERSION "Qt6-6.4.0-vs143-x64")
set_property(GLOBAL PROPERTY NS_CONFIG_QT_WINX64_URL "https://github.com/ezEngine/thirdparty/releases/download/Qt6-6.4.0-vs143-x64/Qt6-6.4.0-vs143-x64.7z")

set_property(GLOBAL PROPERTY NS_CONFIG_VULKAN_SDK_LINUXX64_VERSION "1.3.216.0")
set_property(GLOBAL PROPERTY NS_CONFIG_VULKAN_SDK_LINUXX64_URL "https://sdk.lunarg.com/sdk/download/1.3.216.0/linux/vulkansdk-linux-x86_64-1.3.216.0.tar.gz")


function(generate_version_rc out_version_file name type)
    set(version_file "${CMAKE_CURRENT_BINARY_DIR}/${name}.version.rc")

    set(PRODUCT_VERSION ${PROJECT_VERSION_MAJOR},${PROJECT_VERSION_MINOR},${PROJECT_VERSION_PATCH},0)
    set(FILE_VERSION ${PROJECT_VERSION_MAJOR},${PROJECT_VERSION_MINOR},${PROJECT_VERSION_PATCH},0)

    set(FILE_FLAGS VS_FF_PRERELEASE)
    set(FILE_VERSION_TEXT ${PROJECT_VERSION})
    set(PRODUCT_VERSION_TEXT ${PROJECT_VERSION})

    # These values come from winver.h, which is included by the config-ed file.
    set(FILE_OS VOS__WINDOWS32)

    if(NOT type MATCHES "STATIC|SHARED|MODULE|DRIVER|EXECUTABLE")
        set(type UNKNOWN)
    endif()

    set(FILE_TYPE VFT_UNKNOWN)
    set(FILE_SUBTYPE 0)
    set(extension error)

    if(${type} STREQUAL "EXECUTABLE")
        if(STC_WINNT)
            set(FILE_TYPE VFT_DRV)
            set(FILE_SUBTYPE VFT2_DRV_SYSTEM)
            set(extension sys)
        else()
            set(FILE_TYPE VFT_APP)
            set(extension exe)
        endif()
    elseif(${type} STREQUAL "STATIC")
        set(FILE_TYPE VFT_STATIC_LIB)
        set(extension lib)
    elseif(${type} STREQUAL "SHARED" OR ${type} STREQUAL "MODULE")
        set(FILE_TYPE VFT_DLL)
        set(extension dll)
    elseif(${type} STREQUAL "DRIVER")
        set(FILE_TYPE VFT_DRV)
        set(FILE_SUBTYPE VFT2_DRV_SYSTEM)
        set(extension sys)
    endif()

    set(ORIGINAL_FILENAME ${name}.${extension})

    set(COMMENT ${ADDITIONAL_COMMENT})

    get_property(FILE_DESCRIPTION DIRECTORY PROPERTY ${name}_FILE_DESCRIPTION)
    if(NOT FILE_DESCRIPTION)
        set(FILE_DESCRIPTION "${name}")
    endif()

    if(NOT CMAKE_CONFIGURATION_TYPES)
        # Add markup to the description for non-debug mode.
        if(NOT CMAKE_BUILD_TYPE MATCHES "Release|MinSizeRel|RelWithDebInfo")
            set(build_type ${CMAKE_BUILD_TYPE})
        endif()
        set(FILE_DESCRIPTION "${FILE_DESCRIPTION} (${ARCHITECTURE}) ${CMAKE_BUILD_TYPE}")
    endif()

    set(PRODUCT_NAME ${PROJECT})

    configure_file(
        "${VERSION_RC_TEMPLATE}"
        "${version_file}"
        @ONLY
        )

    set(${out_version_file} ${version_file} PARENT_SCOPE)
endfunction()