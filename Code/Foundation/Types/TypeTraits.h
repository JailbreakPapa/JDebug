#pragma once

/// \file

#include <Foundation/Basics.h>

/// Type traits
template <int v>
struct wdTraitInt
{
  enum
  {
    value = v
  };
};

using wdTypeIsMemRelocatable = wdTraitInt<2>;
using wdTypeIsPod = wdTraitInt<1>;
using wdTypeIsClass = wdTraitInt<0>;

using wdCompileTimeTrueType = char;
using wdCompileTimeFalseType = int;

/// \brief Converts a bool condition to CompileTimeTrue/FalseType
template <bool cond>
struct wdConditionToCompileTimeBool
{
  using type = wdCompileTimeFalseType;
};

template <>
struct wdConditionToCompileTimeBool<true>
{
  using type = wdCompileTimeTrueType;
};

/// \brief Default % operator for T and TypeIsPod which returns a CompileTimeFalseType.
template <typename T>
wdCompileTimeFalseType operator%(const T&, const wdTypeIsPod&);

/// \brief If there is an % operator which takes a TypeIsPod and returns a CompileTimeTrueType T is Pod. Default % operator return false.
template <typename T>
struct wdIsPodType : public wdTraitInt<(sizeof(*((T*)0) % *((const wdTypeIsPod*)0)) == sizeof(wdCompileTimeTrueType)) ? 1 : 0>
{
};

/// \brief Pointers are POD types.
template <typename T>
struct wdIsPodType<T*> : public wdTypeIsPod
{
};

/// \brief arrays are POD types
template <typename T, int N>
struct wdIsPodType<T[N]> : public wdTypeIsPod
{
};

/// \brief Default % operator for T and wdTypeIsMemRelocatable which returns a CompileTimeFalseType.
template <typename T>
wdCompileTimeFalseType operator%(const T&, const wdTypeIsMemRelocatable&);

/// \brief If there is an % operator which takes a wdTypeIsMemRelocatable and returns a CompileTimeTrueType T is Pod. Default % operator
/// return false.
template <typename T>
struct wdGetTypeClass
  : public wdTraitInt<(sizeof(*((T*)0) % *((const wdTypeIsMemRelocatable*)0)) == sizeof(wdCompileTimeTrueType)) ? 2 : wdIsPodType<T>::value>
{
};

/// \brief Static Conversion Test
template <typename From, typename To>
struct wdConversionTest
{
  static wdCompileTimeTrueType Test(const To&);
  static wdCompileTimeFalseType Test(...);
  static From MakeFrom();

  enum
  {
    exists = sizeof(Test(MakeFrom())) == sizeof(wdCompileTimeTrueType),
    sameType = 0
  };
};

/// \brief Specialization for above Type.
template <typename T>
struct wdConversionTest<T, T>
{
  enum
  {
    exists = 1,
    sameType = 1
  };
};

// remapping of the 0 (not special) type to 3
template <typename T1, typename T2>
struct wdGetStrongestTypeClass : public wdTraitInt<(T1::value == 0 || T2::value == 0) ? 0 : WD_COMPILE_TIME_MAX(T1::value, T2::value)>
{
};


/// \brief Determines whether a type is a pointer.
template <typename T>
struct wdIsPointer
{
  static constexpr bool value = false;
};

template <typename T>
struct wdIsPointer<T*>
{
  static constexpr bool value = true;
};


#ifdef __INTELLISENSE__

/// \brief Embed this into a class to mark it as a POD type.
/// POD types will get special treatment from allocators and container classes, such that they are faster to construct and copy.
#  define WD_DECLARE_POD_TYPE()

/// \brief Embed this into a class to mark it as memory relocatable.
/// Memory relocatable types will get special treatment from allocators and container classes, such that they are faster to construct and
/// copy. A type is memory relocatable if it does not have any internal references. e.g: struct example { char[16] buffer; char* pCur;
/// example() pCur(buffer) {} }; A memory relocatable type also must not give out any pointers to its own location. If these two conditions
/// are met, a type is memory relocatable.
#  define WD_DECLARE_MEM_RELOCATABLE_TYPE()

/// \brief mark a class as memory relocatable if the passed type is relocatable or pod.
#  define WD_DECLARE_MEM_RELOCATABLE_TYPE_CONDITIONAL(T)

// \brief embed this into a class to automatically detect which type class it belongs to
// This macro is only guaranteed to work for classes / structs which don't have any constructor / destructor / assignment operator!
// As arguments you have to list the types of all the members of the class / struct.
#  define WD_DETECT_TYPE_CLASS(...)

#else

/// \brief Embed this into a class to mark it as a POD type.
/// POD types will get special treatment from allocators and container classes, such that they are faster to construct and copy.
#  define WD_DECLARE_POD_TYPE() \
    wdCompileTimeTrueType operator%(const wdTypeIsPod&) const { return {}; }

/// \brief Embed this into a class to mark it as memory relocatable.
/// Memory relocatable types will get special treatment from allocators and container classes, such that they are faster to construct and
/// copy. A type is memory relocatable if it does not have any internal references. e.g: struct example { char[16] buffer; char* pCur;
/// example() pCur(buffer) {} }; A memory relocatable type also must not give out any pointers to its own location. If these two conditions
/// are met, a type is memory relocatable.
#  define WD_DECLARE_MEM_RELOCATABLE_TYPE() \
    wdCompileTimeTrueType operator%(const wdTypeIsMemRelocatable&) const { return {}; }

/// \brief mark a class as memory relocatable if the passed type is relocatable or pod.
#  define WD_DECLARE_MEM_RELOCATABLE_TYPE_CONDITIONAL(T)                                                                                       \
    typename wdConditionToCompileTimeBool<wdGetTypeClass<T>::value == wdTypeIsMemRelocatable::value || wdIsPodType<T>::value>::type operator%( \
      const wdTypeIsMemRelocatable&) const { return {}; }

#  define WD_DETECT_TYPE_CLASS_1(T1) wdGetTypeClass<T1>
#  define WD_DETECT_TYPE_CLASS_2(T1, T2) wdGetStrongestTypeClass<WD_DETECT_TYPE_CLASS_1(T1), WD_DETECT_TYPE_CLASS_1(T2)>
#  define WD_DETECT_TYPE_CLASS_3(T1, T2, T3) wdGetStrongestTypeClass<WD_DETECT_TYPE_CLASS_2(T1, T2), WD_DETECT_TYPE_CLASS_1(T3)>
#  define WD_DETECT_TYPE_CLASS_4(T1, T2, T3, T4) wdGetStrongestTypeClass<WD_DETECT_TYPE_CLASS_2(T1, T2), WD_DETECT_TYPE_CLASS_2(T3, T4)>
#  define WD_DETECT_TYPE_CLASS_5(T1, T2, T3, T4, T5) wdGetStrongestTypeClass<WD_DETECT_TYPE_CLASS_4(T1, T2, T3, T4), WD_DETECT_TYPE_CLASS_1(T5)>
#  define WD_DETECT_TYPE_CLASS_6(T1, T2, T3, T4, T5, T6) \
    wdGetStrongestTypeClass<WD_DETECT_TYPE_CLASS_4(T1, T2, T3, T4), WD_DETECT_TYPE_CLASS_2(T5, T6)>

// \brief embed this into a class to automatically detect which type class it belongs to
// This macro is only guaranteed to work for classes / structs which don't have any constructor / destructor / assignment operator!
// As arguments you have to list the types of all the members of the class / struct.
#  define WD_DETECT_TYPE_CLASS(...)  \
    wdCompileTimeTrueType operator%( \
      const wdTraitInt<WD_CALL_MACRO(WD_CONCAT(WD_DETECT_TYPE_CLASS_, WD_VA_NUM_ARGS(__VA_ARGS__)), (__VA_ARGS__))::value>&) const { return {}; }
#endif

/// \brief Defines a type T as Pod.
/// POD types will get special treatment from allocators and container classes, such that they are faster to construct and copy.
#define WD_DEFINE_AS_POD_TYPE(T)             \
  template <>                                \
  struct wdIsPodType<T> : public wdTypeIsPod \
  {                                          \
  }

WD_DEFINE_AS_POD_TYPE(bool);
WD_DEFINE_AS_POD_TYPE(float);
WD_DEFINE_AS_POD_TYPE(double);

WD_DEFINE_AS_POD_TYPE(char);
WD_DEFINE_AS_POD_TYPE(wdInt8);
WD_DEFINE_AS_POD_TYPE(wdInt16);
WD_DEFINE_AS_POD_TYPE(wdInt32);
WD_DEFINE_AS_POD_TYPE(wdInt64);
WD_DEFINE_AS_POD_TYPE(wdUInt8);
WD_DEFINE_AS_POD_TYPE(wdUInt16);
WD_DEFINE_AS_POD_TYPE(wdUInt32);
WD_DEFINE_AS_POD_TYPE(wdUInt64);
WD_DEFINE_AS_POD_TYPE(wchar_t);
WD_DEFINE_AS_POD_TYPE(unsigned long);
WD_DEFINE_AS_POD_TYPE(long);

/// \brief Checks inheritance at compile time.
#define WD_IS_DERIVED_FROM_STATIC(BaseClass, DerivedClass) \
  (wdConversionTest<const DerivedClass*, const BaseClass*>::exists && !wdConversionTest<const BaseClass*, const void*>::sameType)

/// \brief Checks whether A and B are the same type
#define WD_IS_SAME_TYPE(TypeA, TypeB) wdConversionTest<TypeA, TypeB>::sameType

template <typename T>
struct wdTypeTraits
{
  /// \brief removes const qualifier
  using NonConstType = typename std::remove_const<T>::type;

  /// \brief removes reference
  using NonReferenceType = typename std::remove_reference<T>::type;

  /// \brief removes pointer
  using NonPointerType = typename std::remove_pointer<T>::type;

  /// \brief removes reference and const qualifier
  using NonConstReferenceType = typename std::remove_const<typename std::remove_reference<T>::type>::type;

  /// \brief removes reference and pointer qualifier
  using NonReferencePointerType = typename std::remove_pointer<typename std::remove_reference<T>::type>::type;

  /// \brief removes reference, const and pointer qualifier
  /// Note that this removes the const and reference of the type pointed too, not of the pointer.
  using NonConstReferencePointerType = typename std::remove_const<typename std::remove_reference<typename std::remove_pointer<T>::type>::type>::type;
};

/// generates a template named 'checkerName' which checks for the existence of a member function with
/// the name 'functionName' and the signature 'Signature'
#define WD_MAKE_MEMBERFUNCTION_CHECKER(functionName, checkerName)                \
  template <typename T, typename Signature>                                      \
  struct checkerName                                                             \
  {                                                                              \
    template <typename U, U>                                                     \
    struct type_check;                                                           \
    template <typename O>                                                        \
    static wdCompileTimeTrueType& chk(type_check<Signature, &O::functionName>*); \
    template <typename>                                                          \
    static wdCompileTimeFalseType& chk(...);                                     \
    enum                                                                         \
    {                                                                            \
      value = (sizeof(chk<T>(0)) == sizeof(wdCompileTimeTrueType)) ? 1 : 0       \
    };                                                                           \
  }
