# find the folder into which the Vulkan SDK has been installed

# early out, if this target has been created before
if((TARGET NsVulkan::Loader) AND(TARGET NsVulkan::DXC))
	return()
endif()

set(NS_VULKAN_DIR $ENV{VULKAN_SDK} CACHE PATH "Directory of the Vulkan SDK")

ns_pull_compiler_and_architecture_vars()
ns_pull_config_vars()

get_property(NS_SUBMODULE_PREFIX_PATH GLOBAL PROPERTY NS_SUBMODULE_PREFIX_PATH)

if (COMMAND ns_platformhook_find_vulkan)
	ns_platformhook_find_vulkan()
else()
	message(FATAL_ERROR "TODO: Vulkan is not yet supported on this platform and/or architecture.")
endif()
