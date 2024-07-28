#pragma once

#include <Foundation/Basics.h>

// Configure the DLL Import/Export Define
#if NS_ENABLED(NS_COMPILE_ENGINE_AS_DLL)
#  ifdef BUILDSYSTEM_BUILDING_RENDERERCORE_LIB
#    define NS_RENDERERCORE_DLL NS_DECL_EXPORT
#  else
#    define NS_RENDERERCORE_DLL NS_DECL_IMPORT
#  endif
#else
#  define NS_RENDERERCORE_DLL
#endif

#define NS_EMBED_FONT_FILE NS_ON
