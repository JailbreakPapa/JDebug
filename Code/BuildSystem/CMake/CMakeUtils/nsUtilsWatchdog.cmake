# #####################################
# ## Watchdog Profiler support
# #####################################

set(NS_BUILD_WATCHDOG OFF CACHE BOOL "Whether support for the Watchdog profiler should be added")
set(NS_WATCHDOG_PATH CACHE PATH "Path to the Watchdog profiler")

## NOTE: Watchdog is now compiled within Foundation, so it can profile across DLLS.