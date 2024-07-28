#pragma once

#include <Foundation/Basics.h>

// Configure the DLL Import/Export Define
#if NS_ENABLED(NS_COMPILE_ENGINE_AS_DLL)
#  ifdef BUILDSYSTEM_BUILDING_CORE_LIB
#    define NS_CORE_DLL NS_DECL_EXPORT
#    define NS_CORE_DLL_FRIEND NS_DECL_EXPORT_FRIEND
#  else
#    define NS_CORE_DLL NS_DECL_IMPORT
#    define NS_CORE_DLL_FRIEND NS_DECL_IMPORT_FRIEND
#  endif
#else
#  define NS_CORE_DLL
#  define NS_CORE_DLL_FRIEND
#endif
