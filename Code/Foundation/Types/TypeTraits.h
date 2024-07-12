#pragma once

/// \file

#include <Foundation/Basics.h>

/// Type traits
template <int v>
struct nsTraitInt
{
  static constexpr int value = v;
};

using nsTypeIsMemRelocatable = nsTraitInt<2>;
using nsTypeIsPod = nsTraitInt<1>;
using nsTypeIsClass = nsTraitInt<0>;

using nsCompileTimeTrueType = char;
using nsCompileTimeFalseType = int;

/// \brief Converts a bool condition to CompileTimeTrue/FalseType
template <bool cond>
struct nsConditionToCompileTimeBool
{
  using type = nsCompileTimeFalseType;
};

template <>
struct nsConditionToCompileTimeBool<true>
{
  using type = nsCompileTimeTrueType;
};

/// \brief Default % operator for T and TypeIsPod which returns a CompileTimeFalseType.
template <typename T>
nsCompileTimeFalseType operator%(const T&, const nsTypeIsPod&);

/// \brief If there is an % operator which takes a TypeIsPod and returns a CompileTimeTrueType T is Pod. Default % operator return false.
template <typename T>
struct nsIsPodType : public nsTraitInt<(sizeof(*((T*)0) % *((const nsTypeIsPod*)0)) == sizeof(nsCompileTimeTrueType)) ? 1 : 0>
{
};

/// \brief Pointers are POD types.
template <typename T>
struct nsIsPodType<T*> : public nsTypeIsPod
{
};

/// \brief arrays are POD types
template <typename T, int N>
struct nsIsPodType<T[N]> : public nsTypeIsPod
{
};

/// \brief Default % operator for T and nsTypeIsMemRelocatable which returns a CompileTimeFalseType.
template <typename T>
nsCompileTimeFalseType operator%(const T&, const nsTypeIsMemRelocatable&);

/// \brief If there is an % operator which takes a nsTypeIsMemRelocatable and returns a CompileTimeTrueType T is Pod. Default % operator
/// return false.
template <typename T>
struct nsGetTypeClass
  : public nsTraitInt<(sizeof(*((T*)0) % *((const nsTypeIsMemRelocatable*)0)) == sizeof(nsCompileTimeTrueType)) ? 2 : nsIsPodType<T>::value>
{
};

/// \brief Static Conversion Test
template <typename From, typename To>
struct nsConversionTest
{
  static nsCompileTimeTrueType Test(const To&);
  static nsCompileTimeFalseType Test(...);
  static From MakeFrom();

  enum
  {
    exists = sizeof(Test(MakeFrom())) == sizeof(nsCompileTimeTrueType),
    sameType = 0
  };
};

/// \brief Specialization for above Type.
template <typename T>
struct nsConversionTest<T, T>
{
  enum
  {
    exists = 1,
    sameType = 1
  };
};

// remapping of the 0 (not special) type to 3
template <typename T1, typename T2>
struct nsGetStrongestTypeClass : public nsTraitInt<(T1::value == 0 || T2::value == 0) ? 0 : NS_COMPILE_TIME_MAX(T1::value, T2::value)>
{
};


#ifdef __INTELLISENSE__

/// \brief Embed this into a class to mark it as a POD type.
/// POD types will get special treatment from allocators and container classes, such that they are faster to construct and copy.
#  define NS_DECLARE_POD_TYPE()

/// \brief Embed this into a class to mark it as memory relocatable.
/// Memory relocatable types will get special treatment from allocators and container classes, such that they are faster to construct and
/// copy. A type is memory relocatable if it does not have any internal references. e.g: struct example { char[16] buffer; char* pCur;
/// example() pCur(buffer) {} }; A memory relocatable type also must not give out any pointers to its own location. If these two conditions
/// are met, a type is memory relocatable.
#  define NS_DECLARE_MEM_RELOCATABLE_TYPE()

/// \brief mark a class as memory relocatable if the passed type is relocatable or pod.
#  define NS_DECLARE_MEM_RELOCATABLE_TYPE_CONDITIONAL(T)

// \brief embed this into a class to automatically detect which type class it belongs to
// This macro is only guaranteed to work for classes / structs which don't have any constructor / destructor / assignment operator!
// As arguments you have to list the types of all the members of the class / struct.
#  define NS_DETECT_TYPE_CLASS(...)

#else

/// \brief Embed this into a class to mark it as a POD type.
/// POD types will get special treatment from allocators and container classes, such that they are faster to construct and copy.
#  define NS_DECLARE_POD_TYPE()                               \
    nsCompileTimeTrueType operator%(const nsTypeIsPod&) const \
    {                                                         \
      return {};                                              \
    }

/// \brief Embed this into a class to mark it as memory relocatable.
/// Memory relocatable types will get special treatment from allocators and container classes, such that they are faster to construct and
/// copy. A type is memory relocatable if it does not have any internal references. e.g: struct example { char[16] buffer; char* pCur;
/// example() pCur(buffer) {} }; A memory relocatable type also must not give out any pointers to its own location. If these two conditions
/// are met, a type is memory relocatable.
#  define NS_DECLARE_MEM_RELOCATABLE_TYPE()                              \
    nsCompileTimeTrueType operator%(const nsTypeIsMemRelocatable&) const \
    {                                                                    \
      return {};                                                         \
    }

/// \brief mark a class as memory relocatable if the passed type is relocatable or pod.
#  define NS_DECLARE_MEM_RELOCATABLE_TYPE_CONDITIONAL(T)                                                                                       \
    typename nsConditionToCompileTimeBool<nsGetTypeClass<T>::value == nsTypeIsMemRelocatable::value || nsIsPodType<T>::value>::type operator%( \
      const nsTypeIsMemRelocatable&) const                                                                                                     \
    {                                                                                                                                          \
      return {};                                                                                                                               \
    }

#  define NS_DETECT_TYPE_CLASS_1(T1) nsGetTypeClass<T1>
#  define NS_DETECT_TYPE_CLASS_2(T1, T2) nsGetStrongestTypeClass<NS_DETECT_TYPE_CLASS_1(T1), NS_DETECT_TYPE_CLASS_1(T2)>
#  define NS_DETECT_TYPE_CLASS_3(T1, T2, T3) nsGetStrongestTypeClass<NS_DETECT_TYPE_CLASS_2(T1, T2), NS_DETECT_TYPE_CLASS_1(T3)>
#  define NS_DETECT_TYPE_CLASS_4(T1, T2, T3, T4) nsGetStrongestTypeClass<NS_DETECT_TYPE_CLASS_2(T1, T2), NS_DETECT_TYPE_CLASS_2(T3, T4)>
#  define NS_DETECT_TYPE_CLASS_5(T1, T2, T3, T4, T5) nsGetStrongestTypeClass<NS_DETECT_TYPE_CLASS_4(T1, T2, T3, T4), NS_DETECT_TYPE_CLASS_1(T5)>
#  define NS_DETECT_TYPE_CLASS_6(T1, T2, T3, T4, T5, T6) \
    nsGetStrongestTypeClass<NS_DETECT_TYPE_CLASS_4(T1, T2, T3, T4), NS_DETECT_TYPE_CLASS_2(T5, T6)>

// \brief embed this into a class to automatically detect which type class it belongs to
// This macro is only guaranteed to work for classes / structs which don't have any constructor / destructor / assignment operator!
// As arguments you have to list the types of all the members of the class / struct.
#  define NS_DETECT_TYPE_CLASS(...)                                                                                                \
    nsCompileTimeTrueType operator%(                                                                                               \
      const nsTraitInt<NS_CALL_MACRO(NS_CONCAT(NS_DETECT_TYPE_CLASS_, NS_VA_NUM_ARGS(__VA_ARGS__)), (__VA_ARGS__))::value>&) const \
    {                                                                                                                              \
      return {};                                                                                                                   \
    }
#endif

/// \brief Defines a type T as Pod.
/// POD types will get special treatment from allocators and container classes, such that they are faster to construct and copy.
#define NS_DEFINE_AS_POD_TYPE(T)             \
  template <>                                \
  struct nsIsPodType<T> : public nsTypeIsPod \
  {                                          \
  }

NS_DEFINE_AS_POD_TYPE(bool);
NS_DEFINE_AS_POD_TYPE(float);
NS_DEFINE_AS_POD_TYPE(double);

NS_DEFINE_AS_POD_TYPE(char);
NS_DEFINE_AS_POD_TYPE(nsInt8);
NS_DEFINE_AS_POD_TYPE(nsInt16);
NS_DEFINE_AS_POD_TYPE(nsInt32);
NS_DEFINE_AS_POD_TYPE(nsInt64);
NS_DEFINE_AS_POD_TYPE(nsUInt8);
NS_DEFINE_AS_POD_TYPE(nsUInt16);
NS_DEFINE_AS_POD_TYPE(nsUInt32);
NS_DEFINE_AS_POD_TYPE(nsUInt64);
NS_DEFINE_AS_POD_TYPE(wchar_t);
NS_DEFINE_AS_POD_TYPE(unsigned long);
NS_DEFINE_AS_POD_TYPE(long);
NS_DEFINE_AS_POD_TYPE(std::byte);

/// \brief Checks inheritance at compile time.
#define NS_IS_DERIVED_FROM_STATIC(BaseClass, DerivedClass) \
  (nsConversionTest<const DerivedClass*, const BaseClass*>::exists && !nsConversionTest<const BaseClass*, const void*>::sameType)

/// \brief Checks whether A and B are the same type
#define NS_IS_SAME_TYPE(TypeA, TypeB) nsConversionTest<TypeA, TypeB>::sameType

template <typename T>
struct nsTypeTraits
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
#define NS_MAKE_MEMBERFUNCTION_CHECKER(functionName, checkerName)                \
  template <typename T, typename Signature>                                      \
  struct checkerName                                                             \
  {                                                                              \
    template <typename U, U>                                                     \
    struct type_check;                                                           \
    template <typename O>                                                        \
    static nsCompileTimeTrueType& chk(type_check<Signature, &O::functionName>*); \
    template <typename>                                                          \
    static nsCompileTimeFalseType& chk(...);                                     \
    enum                                                                         \
    {                                                                            \
      value = (sizeof(chk<T>(0)) == sizeof(nsCompileTimeTrueType)) ? 1 : 0       \
    };                                                                           \
  }
