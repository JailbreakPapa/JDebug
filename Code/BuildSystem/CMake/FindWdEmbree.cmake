# find the folder in which Embree is located

# early out, if this target has been created before
if(TARGET WdEmbree::WdEmbree)
	return()
endif()

find_path(WD_EMBREE_DIR include/embree3/rtcore.h
	PATHS
	${CMAKE_SOURCE_DIR}/Code/ThirdParty/embree
)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(EMBREE_LIB_PATH "${WD_EMBREE_DIR}/vc141win64")
else()
	set(EMBREE_LIB_PATH "${WD_EMBREE_DIR}/vc141win32")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WdEmbree DEFAULT_MSG WD_EMBREE_DIR)

if(WDEMBREE_FOUND)
	add_library(WdEmbree::WdEmbree SHARED IMPORTED)
	set_target_properties(WdEmbree::WdEmbree PROPERTIES IMPORTED_LOCATION "${EMBREE_LIB_PATH}/embree3.dll")
	set_target_properties(WdEmbree::WdEmbree PROPERTIES IMPORTED_IMPLIB "${EMBREE_LIB_PATH}/embree3.lib")
	set_target_properties(WdEmbree::WdEmbree PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${WD_EMBREE_DIR}/include")
endif()

mark_as_advanced(FORCE WD_EMBREE_DIR)

unset(EMBREE_LIB_PATH)
