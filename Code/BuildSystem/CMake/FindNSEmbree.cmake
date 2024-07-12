# find the folder in which Embree is located

# early out, if this target has been created before
if(TARGET NsEmbree::NsEmbree)
	return()
endif()

find_path(NS_EMBREE_DIR include/embree3/rtcore.h
	PATHS
	${NS_ROOT}/Code/ThirdParty/embree
)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(EMBREE_LIB_PATH "${NS_EMBREE_DIR}/vc141win64")
else()
	set(EMBREE_LIB_PATH "${NS_EMBREE_DIR}/vc141win32")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(NsEmbree DEFAULT_MSG NS_EMBREE_DIR)

if(NSEMBREE_FOUND)
	add_library(NsEmbree::NsEmbree SHARED IMPORTED)
	set_target_properties(NsEmbree::NsEmbree PROPERTIES IMPORTED_LOCATION "${EMBREE_LIB_PATH}/embree3.dll")
	set_target_properties(NsEmbree::NsEmbree PROPERTIES IMPORTED_IMPLIB "${EMBREE_LIB_PATH}/embree3.lib")
	set_target_properties(NsEmbree::NsEmbree PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${NS_EMBREE_DIR}/include")
endif()

mark_as_advanced(FORCE NS_EMBREE_DIR)

unset(EMBREE_LIB_PATH)
