set (NS_BUILD_UNITTESTS ON CACHE BOOL "Whether unit-test projects should be added to the solution")

ns_requires(NS_BUILD_UNITTESTS)

ns_add_all_subdirs()