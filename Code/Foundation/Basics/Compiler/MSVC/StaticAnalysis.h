
#pragma once

#undef NS_MSVC_ANALYSIS_WARNING_PUSH
#undef NS_MSVC_ANALYSIS_WARNING_POP
#undef NS_MSVC_ANALYSIS_WARNING_DISABLE
#undef NS_MSVC_ANALYSIS_ASSUME

// These use the __pragma version to control the warnings so that they can be used within other macros etc.
#define NS_MSVC_ANALYSIS_WARNING_PUSH __pragma(warning(push))
#define NS_MSVC_ANALYSIS_WARNING_POP __pragma(warning(pop))
#define NS_MSVC_ANALYSIS_WARNING_DISABLE(warningNumber) __pragma(warning(disable \
                                                                         : warningNumber))
#define NS_MSVC_ANALYSIS_ASSUME(expression) __assume(expression)
