#pragma once

#if WD_DISABLED(WD_PLATFORM_WINDOWS)
#  error "This header should only be included on windows platforms"
#endif

#ifdef _WIN64
#  undef WD_PLATFORM_64BIT
#  define WD_PLATFORM_64BIT WD_ON
#else
#  undef WD_PLATFORM_32BIT
#  define WD_PLATFORM_32BIT WD_ON
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif

#include <winapifamily.h>

#undef WD_PLATFORM_WINDOWS_UWP
#undef WD_PLATFORM_WINDOWS_DESKTOP

// Distinguish between Windows desktop and Windows UWP.
#if WINAPI_FAMILY == WINAPI_FAMILY_APP
#  define WD_PLATFORM_WINDOWS_UWP WD_ON
#  define WD_PLATFORM_WINDOWS_DESKTOP WD_OFF
#else
#  define WD_PLATFORM_WINDOWS_UWP WD_OFF
#  define WD_PLATFORM_WINDOWS_DESKTOP WD_ON
#endif

#ifndef NULL
#  define NULL 0
#endif

#undef WD_PLATFORM_LITTLE_ENDIAN
#define WD_PLATFORM_LITTLE_ENDIAN WD_ON

#include <Foundation/Basics/Compiler/Clang/Clang.h>
#include <Foundation/Basics/Compiler/GCC/GCC.h>
#include <Foundation/Basics/Compiler/MSVC/MSVC.h>
