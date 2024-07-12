#pragma once

#ifdef BUILDSYSTEM_BUILDING_FOUNDATION_LIB
#  define NS_FOUNDATION_INTERNAL_HEADER_ALLOWED 1
#else
#  define NS_FOUNDATION_INTERNAL_HEADER_ALLOWED 0
#endif

#define NS_FOUNDATION_INTERNAL_HEADER static_assert(NS_FOUNDATION_INTERNAL_HEADER_ALLOWED, "This is an internal ns header. Please do not #include it directly.");
