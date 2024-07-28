######################################
### Jolt support
######################################

set (NS_3RDPARTY_JOLT_SUPPORT ON CACHE BOOL "Whether to add support for the Jolt physics engine.")
mark_as_advanced(FORCE NS_3RDPARTY_JOLT_SUPPORT)

######################################
### ns_requires_jolt()
######################################

macro(ns_requires_jolt)

	ns_requires(NS_3RDPARTY_JOLT_SUPPORT)

endmacro()
