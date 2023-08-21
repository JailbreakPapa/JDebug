#pragma once

#ifndef WD_CONCAT

/// \brief Concatenates two strings, even when the strings are macros themselves
#define WD_CONCAT(x,y) WD_CONCAT_HELPER(x,y)
#define WD_CONCAT_HELPER(x,y) WD_CONCAT_HELPER2(x,y)
#define WD_CONCAT_HELPER2(x,y) x##y

#endif

#ifndef WD_STRINGIZE

/// \brief Turns some piece of code (usually some identifier name) into a string. Even works on macros.
#define WD_STRINGIZE(str) WD_STRINGIZE_HELPER(str)
#define WD_STRINGIZE_HELPER(x) #x

#endif

#ifndef WD_ON

/// \brief Used in conjunction with WD_ENABLED and WD_DISABLED for safe checks. Define something to WD_ON or WD_OFF to work with those macros.
#define WD_ON =

/// \brief Used in conjunction with WD_ENABLED and WD_DISABLED for safe checks. Define something to WD_ON or WD_OFF to work with those macros.
#define WD_OFF !

/// \brief Used in conjunction with WD_ON and WD_OFF for safe checks. Use #if WD_ENABLED(x) or #if WD_DISABLED(x) in conditional compilation.
#define WD_ENABLED(x) (1 WD_CONCAT(x,=) 1)

/// \brief Used in conjunction with WD_ON and WD_OFF for safe checks. Use #if WD_ENABLED(x) or #if WD_DISABLED(x) in conditional compilation.
#define WD_DISABLED(x) (1 WD_CONCAT(x,=) 2)

/// \brief Checks whether x AND y are both defined as WD_ON or WD_OFF. Usually used to check whether configurations overlap, to issue an error.
#define WD_IS_NOT_EXCLUSIVE(x, y) ((1 WD_CONCAT(x,=) 1) == (1 WD_CONCAT(y,=) 1))

#endif
