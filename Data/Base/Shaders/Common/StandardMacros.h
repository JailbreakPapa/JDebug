#pragma once

#ifndef NS_CONCAT

/// \brief Concatenates two strings, even when the strings are macros themselves
#  define NS_CONCAT(x, y) NS_CONCAT_HELPER(x, y)
#  define NS_CONCAT_HELPER(x, y) NS_CONCAT_HELPER2(x, y)
#  define NS_CONCAT_HELPER2(x, y) x##y

#endif

#ifndef NS_STRINGIZE

/// \brief Turns some piece of code (usually some identifier name) into a string. Even works on macros.
#  define NS_STRINGIZE(str) NS_STRINGIZE_HELPER(str)
#  define NS_STRINGIZE_HELPER(x) #x

#endif

#ifndef NS_ON

/// \brief Used in conjunction with NS_ENABLED and NS_DISABLED for safe checks. Define something to NS_ON or NS_OFF to work with those macros.
#  define NS_ON =

/// \brief Used in conjunction with NS_ENABLED and NS_DISABLED for safe checks. Define something to NS_ON or NS_OFF to work with those macros.
#  define NS_OFF !

/// \brief Used in conjunction with NS_ON and NS_OFF for safe checks. Use #if NS_ENABLED(x) or #if NS_DISABLED(x) in conditional compilation.
#  define NS_ENABLED(x) (1 NS_CONCAT(x, =) 1)

/// \brief Used in conjunction with NS_ON and NS_OFF for safe checks. Use #if NS_ENABLED(x) or #if NS_DISABLED(x) in conditional compilation.
#  define NS_DISABLED(x) (1 NS_CONCAT(x, =) 2)

/// \brief Checks whether x AND y are both defined as NS_ON or NS_OFF. Usually used to check whether configurations overlap, to issue an error.
#  define NS_IS_NOT_EXCLUSIVE(x, y) ((1 NS_CONCAT(x, =) 1) == (1 NS_CONCAT(y, =) 1))

#endif

/// \brief #TODO_SHADER Right now these are only used in the ST_SetsSlots unit test. We will need to decide what the best separation and naming for these sets is once the renderer can make actual use of these to improve performance.
#define SET_FRAME 0
#define SET_RENDER_PASS 1
#define SET_MATERIAL 2
#define SET_DRAW_CALL 3
#define SLOT_AUTO AUTO

/// \brief Binds the resource to the given set and slot. Note that this does not produce valid HLSL code, the code will instead be patched by the shader compiler.
#define BIND_RESOURCE(Slot, Set) : register(NS_CONCAT(x, Slot), NS_CONCAT(space, Set))

/// \brief Binds the resource to the given set. Note that this does not produce valid HLSL code, the code will instead be patched by the shader compiler.
#define BIND_SET(Set) BIND_RESOURCE(SLOT_AUTO, Set)
