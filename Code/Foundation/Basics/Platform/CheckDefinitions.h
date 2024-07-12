#pragma once

#ifndef NULL
#  error "NULL is not defined."
#endif

#ifndef NS_FORCE_INLINE
#  error "NS_FORCE_INLINE is not defined."
#endif

#ifndef NS_ALWAYS_INLINE
#  error "NS_ALWAYS_INLINE is not defined."
#endif

#ifndef NS_ALIGNMENT_OF
#  error "NS_ALIGNMENT_OF is not defined."
#endif

#if NS_IS_NOT_EXCLUSIVE(NS_PLATFORM_32BIT, NS_PLATFORM_64BIT)
#  error "Platform is not defined as 32 Bit or 64 Bit"
#endif

#ifndef NS_DEBUG_BREAK
#  error "NS_DEBUG_BREAK is not defined."
#endif

#ifndef NS_SOURCE_FUNCTION
#  error "NS_SOURCE_FUNCTION is not defined."
#endif

#ifndef NS_SOURCE_FILE
#  error "NS_SOURCE_FILE is not defined."
#endif

#ifndef NS_SOURCE_LINE
#  error "NS_SOURCE_LINE is not defined."
#endif

#if NS_IS_NOT_EXCLUSIVE(NS_PLATFORM_LITTLE_ENDIAN, NS_PLATFORM_BIG_ENDIAN)
#  error "Endianess is not correctly defined."
#endif

#ifndef NS_MATH_CHECK_FOR_NAN
#  error "NS_MATH_CHECK_FOR_NAN is not defined."
#endif

#if NS_IS_NOT_EXCLUSIVE(NS_PLATFORM_ARCH_X86, NS_PLATFORM_ARCH_ARM)
#  error "Platform architecture is not correctly defined."
#endif

#if !defined(NS_SIMD_IMPLEMENTATION) || (NS_SIMD_IMPLEMENTATION == 0)
#  error "NS_SIMD_IMPLEMENTATION is not correctly defined."
#endif
