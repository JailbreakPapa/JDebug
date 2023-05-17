/*
 *   Copyright (c) 2023 Watch Dogs LLC
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#pragma once

#include <Foundation/Basics.h>

// Configure the DLL Import/Export Define
#if WD_ENABLED(WD_COMPILE_ENGINE_AS_DLL)
#  ifdef BUILDSYSTEM_BUILDING_TOOLSFOUNDATION_LIB
#    define WD_TOOLSFOUNDATION_DLL WD_DECL_EXPORT
#  else
#    define WD_TOOLSFOUNDATION_DLL WD_DECL_IMPORT
#  endif
#else
#  define WD_TOOLSFOUNDATION_DLL
#endif
