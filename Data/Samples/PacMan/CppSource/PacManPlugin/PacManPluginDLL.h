#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Configuration/Plugin.h>

// Configure the DLL Import/Export Define
#if WD_ENABLED(WD_COMPILE_ENGINE_AS_DLL)
#  ifdef BUILDSYSTEM_BUILDING_PACMANPLUGIN_LIB
#    define WD_PACMANPLUGIN_DLL WD_DECL_EXPORT
#  else
#    define WD_PACMANPLUGIN_DLL WD_DECL_IMPORT
#  endif
#else
#  define WD_PACMANPLUGIN_DLL
#endif
