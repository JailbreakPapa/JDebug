#pragma once

#if NS_DISABLED(NS_PLATFORM_LINUX) && NS_DISABLED(NS_PLATFORM_ANDROID)
#  error "This header should only be included on Linux"
#endif

#include <cstdio>
#include <malloc.h>
#include <pthread.h>
#include <stdarg.h>
#include <sys/time.h>
#include <unistd.h>

// unset common macros
#ifdef min
#  undef min
#endif
#ifdef max
#  undef max
#endif

#include <Foundation/Basics/Compiler/Clang/Clang.h>
#include <Foundation/Basics/Compiler/GCC/GCC.h>
#include <Foundation/Basics/Compiler/MSVC/MSVC.h>

#undef NS_PLATFORM_LITTLE_ENDIAN
#define NS_PLATFORM_LITTLE_ENDIAN NS_ON
