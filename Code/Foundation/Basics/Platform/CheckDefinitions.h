#pragma once

#ifndef NULL
#  error "NULL is not defined."
#endif

#ifndef WD_FORCE_INLINE
#  error "WD_FORCE_INLINE is not defined."
#endif

#ifndef WD_ALWAYS_INLINE
#  error "WD_ALWAYS_INLINE is not defined."
#endif

#ifndef WD_ALIGNMENT_OF
#  error "WD_ALIGNMENT_OF is not defined."
#endif

#if WD_IS_NOT_EXCLUSIVE(WD_PLATFORM_32BIT, WD_PLATFORM_64BIT)
#  error "Platform is not defined as 32 Bit or 64 Bit"
#endif

#ifndef WD_DEBUG_BREAK
#  error "WD_DEBUG_BREAK is not defined."
#endif

#ifndef WD_SOURCE_FUNCTION
#  error "WD_SOURCE_FUNCTION is not defined."
#endif

#ifndef WD_SOURCE_FILE
#  error "WD_SOURCE_FILE is not defined."
#endif

#ifndef WD_SOURCE_LINE
#  error "WD_SOURCE_LINE is not defined."
#endif

#if WD_IS_NOT_EXCLUSIVE(WD_PLATFORM_LITTLE_ENDIAN, WD_PLATFORM_BIG_ENDIAN)
#  error "Endianess is not correctly defined."
#endif

#ifndef WD_MATH_CHECK_FOR_NAN
#  error "WD_MATH_CHECK_FOR_NAN is not defined."
#endif

#if WD_IS_NOT_EXCLUSIVE(WD_PLATFORM_ARCH_X86, WD_PLATFORM_ARCH_ARM)
#  error "Platform architecture is not correctly defined."
#endif

#if !defined(WD_SIMD_IMPLEMENTATION) || (WD_SIMD_IMPLEMENTATION == 0)
#  error "WD_SIMD_IMPLEMENTATION is not correctly defined."
#endif
