#pragma once

#if defined(__clang__) || defined(__GNUC__)

#  if defined(__x86_64__) || defined(__i386__)
#    undef NS_PLATFORM_ARCH_X86
#    define NS_PLATFORM_ARCH_X86 NS_ON
#  elif defined(__arm__) || defined(__aarch64__)
#    undef NS_PLATFORM_ARCH_ARM
#    define NS_PLATFORM_ARCH_ARM NS_ON
#  else
#    error unhandled target architecture
#  endif

#  if defined(__x86_64__) || defined(__aarch64__)
#    undef NS_PLATFORM_64BIT
#    define NS_PLATFORM_64BIT NS_ON
#  elif defined(__i386__) || defined(__arm__)
#    undef NS_PLATFORM_32BIT
#    define NS_PLATFORM_32BIT NS_ON
#  else
#    error unhandled platform bit count
#  endif

#elif defined(_MSC_VER)

#  if defined(_M_AMD64) || defined(_M_IX86)
#    undef NS_PLATFORM_ARCH_X86
#    define NS_PLATFORM_ARCH_X86 NS_ON
#  elif defined(_M_ARM) || defined(_M_ARM64)
#    undef NS_PLATFORM_ARCH_ARM
#    define NS_PLATFORM_ARCH_ARM NS_ON
#  else
#    error unhandled target architecture
#  endif

#  if defined(_M_AMD64) || defined(_M_ARM64)
#    undef NS_PLATFORM_64BIT
#    define NS_PLATFORM_64BIT NS_ON
#  elif defined(_M_IX86) || defined(_M_ARM)
#    undef NS_PLATFORM_32BIT
#    define NS_PLATFORM_32BIT NS_ON
#  else
#    error unhandled platform bit count
#  endif

#else
#  error unhandled compiler
#endif
