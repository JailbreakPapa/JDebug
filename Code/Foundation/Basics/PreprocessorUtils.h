
#pragma once

/// \file

/// \brief Concatenates two strings, even when the strings are macros themselves
#define WD_CONCAT(x, y) WD_CONCAT_HELPER(x, y)
#define WD_CONCAT_HELPER(x, y) WD_CONCAT_HELPER2(x, y)
#define WD_CONCAT_HELPER2(x, y) x##y

/// \brief Turns some piece of code (usually some identifier name) into a string. Even works on macros.
#define WD_STRINGIZE(str) WD_STRINGIZE_HELPER(str)
#define WD_STRINGIZE_HELPER(x) #x

/// \brief Concatenates two strings, even when the strings are macros themselves
#define WD_PP_CONCAT(x, y) WD_CONCAT_HELPER(x, y)

/// \brief Turns some piece of code (usually some identifier name) into a string. Even works on macros.
#define WD_PP_STRINGIFY(str) WD_STRINGIZE_HELPER(str)

/// \brief Max value of two compile-time constant expression.
#define WD_COMPILE_TIME_MAX(a, b) ((a) > (b) ? (a) : (b))

/// \brief Min value of two compile-time constant expression.
#define WD_COMPILE_TIME_MIN(a, b) ((a) < (b) ? (a) : (b))


/// \brief Creates a bit mask with only the n-th Bit set. Useful when creating enum values for flags.
#define WD_BIT(n) (1ull << (n))
