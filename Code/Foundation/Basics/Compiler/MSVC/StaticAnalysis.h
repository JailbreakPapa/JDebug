
#pragma once

#undef WD_MSVC_ANALYSIS_WARNING_PUSH
#undef WD_MSVC_ANALYSIS_WARNING_POP
#undef WD_MSVC_ANALYSIS_WARNING_DISABLE
#undef WD_MSVC_ANALYSIS_ASSUME

// These use the __pragma version to control the warnings so that they can be used within other macros etc.
#define WD_MSVC_ANALYSIS_WARNING_PUSH __pragma(warning(push))
#define WD_MSVC_ANALYSIS_WARNING_POP __pragma(warning(pop))
#define WD_MSVC_ANALYSIS_WARNING_DISABLE(warningNumber) __pragma(warning(disable : warningNumber))
#define WD_MSVC_ANALYSIS_ASSUME(expression) __assume(expression)
