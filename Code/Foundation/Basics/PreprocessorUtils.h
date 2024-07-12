
#pragma once

/// \file

/// \brief Used to pass a token through without modification
/// Useful to separate tokens that have no whitespace in between and thus would otherwise form one string.
#define NS_PP_IDENTITY(x) x

/// \brief Concatenates two strings, even when the strings are macros themselves
#define NS_CONCAT(x, y) NS_CONCAT_HELPER(x, y)
#define NS_CONCAT_HELPER(x, y) NS_CONCAT_HELPER2(x, y)
#define NS_CONCAT_HELPER2(x, y) x##y

/// \brief Turns some piece of code (usually some identifier name) into a string. Even works on macros.
#define NS_STRINGIZE(str) NS_STRINGIZE_HELPER(str)
#define NS_STRINGIZE_HELPER(x) #x

/// \brief Concatenates two strings, even when the strings are macros themselves
#define NS_PP_CONCAT(x, y) NS_CONCAT_HELPER(x, y)

/// \brief Turns some piece of code (usually some identifier name) into a string. Even works on macros.
#define NS_PP_STRINGIFY(str) NS_STRINGIZE_HELPER(str)

/// \brief Max value of two compile-time constant expression.
#define NS_COMPILE_TIME_MAX(a, b) ((a) > (b) ? (a) : (b))

/// \brief Min value of two compile-time constant expression.
#define NS_COMPILE_TIME_MIN(a, b) ((a) < (b) ? (a) : (b))


/// \brief Creates a bit mask with only the n-th Bit set. Useful when creating enum values for flags.
#define NS_BIT(n) (1ull << (n))
