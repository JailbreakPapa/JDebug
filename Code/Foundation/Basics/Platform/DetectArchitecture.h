#pragma once

#if defined(__clang__) || defined(__GNUC__)

#  if defined(__x86_64__) || defined(__i386__)
#    undef WD_PLATFORM_ARCH_X86
#    define WD_PLATFORM_ARCH_X86 WD_ON
#  elif defined(__arm__) || defined(__aarch64__)
#    undef WD_PLATFORM_ARCH_ARM
#    define WD_PLATFORM_ARCH_ARM WD_ON
#  else
#    error unhandled target architecture
#  endif

#  if defined(__x86_64__) || defined(__aarch64__)
#    undef WD_PLATFORM_64BIT
#    define WD_PLATFORM_64BIT WD_ON
#  elif defined(__i386__) || defined(__arm__)
#    undef WD_PLATFORM_32BIT
#    define WD_PLATFORM_32BIT WD_ON
#  else
#    error unhandled platform bit count
#  endif

#elif defined(_MSC_VER)

#  if defined(_M_AMD64) || defined(_M_IX86)
#    undef WD_PLATFORM_ARCH_X86
#    define WD_PLATFORM_ARCH_X86 WD_ON
#  elif defined(_M_ARM) || defined(_M_ARM64)
#    undef WD_PLATFORM_ARCH_ARM
#    define WD_PLATFORM_ARCH_ARM WD_ON
#  else
#    error unhandled target architecture
#  endif

#  if defined(_M_AMD64) || defined(_M_ARM64)
#    undef WD_PLATFORM_64BIT
#    define WD_PLATFORM_64BIT WD_ON
#  elif defined(_M_IX86) || defined(_M_ARM)
#    undef WD_PLATFORM_32BIT
#    define WD_PLATFORM_32BIT WD_ON
#  else
#    error unhandled platform bit count
#  endif

#else
#  error unhandled compiler
#endif
