# #####################################
# ## NuGet support
# #####################################

# #####################################
# ## wd_nuget_init()
# #####################################

function(wd_nuget_init)
	find_program(NUGET nuget
	HINTS ${CMAKE_BINARY_DIR})

    find_program(NUGET nuget
	HINTS "${CMAKE_SOURCE_DIR}/Workspace")

	if(NOT NUGET)
		message(STATUS "Downloading Nuget...")
		file(DOWNLOAD
			https://dist.nuget.org/win-x86-commandline/latest/nuget.exe
			${CMAKE_BINARY_DIR}/nuget.exe
			SHOW_PROGRESS
		)
        # Ensure that nuget is also in workspace, as visual studio will first look in there for requesting packages.
        configure_file(${CMAKE_BINARY_DIR}/nuget.exe ${CMAKE_SOURCE_DIR}/Workspace/nuget.exe COPYONLY)

		find_program(NUGET nuget
			HINTS ${CMAKE_BINARY_DIR})
	endif()

	mark_as_advanced(FORCE NUGET)
endfunction()
