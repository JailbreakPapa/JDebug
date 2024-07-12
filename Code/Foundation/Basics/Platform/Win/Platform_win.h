#pragma once

#if NS_DISABLED(NS_PLATFORM_WINDOWS)
#  error "This header should only be included on windows platforms"
#endif

#ifdef _WIN64
#  undef NS_PLATFORM_64BIT
#  define NS_PLATFORM_64BIT NS_ON
#else
#  undef NS_PLATFORM_32BIT
#  define NS_PLATFORM_32BIT NS_ON
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif

#include <winapifamily.h>

#undef NS_PLATFORM_WINDOWS_UWP
#undef NS_PLATFORM_WINDOWS_DESKTOP

// Distinguish between Windows desktop and Windows UWP.
#if WINAPI_FAMILY == WINAPI_FAMILY_APP
#  define NS_PLATFORM_WINDOWS_UWP NS_ON
#  define NS_PLATFORM_WINDOWS_DESKTOP NS_OFF
#else
#  define NS_PLATFORM_WINDOWS_UWP NS_OFF
#  define NS_PLATFORM_WINDOWS_DESKTOP NS_ON
#endif

#ifndef NULL
#  define NULL 0
#endif

#undef NS_PLATFORM_LITTLE_ENDIAN
#define NS_PLATFORM_LITTLE_ENDIAN NS_ON

#include <Foundation/Basics/Compiler/Clang/Clang.h>
#include <Foundation/Basics/Compiler/GCC/GCC.h>
#include <Foundation/Basics/Compiler/MSVC/MSVC.h>
