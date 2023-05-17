#pragma once

/// \file

#include <Foundation/Reflection/Implementation/StaticRTTI.h>

/// \brief This needs to be put into the class declaration of EVERY dynamically reflectable class.
///
/// This macro extends a class, such that it is now able to return its own type information via GetDynamicRTTI(),
/// which is a virtual function, that is reimplemented on each type. A class needs to be derived from wdReflectedClass
/// (at least indirectly) for this.
#define WD_ADD_DYNAMIC_REFLECTION_NO_GETTER(SELF, BASE_TYPE) \
  WD_ALLOW_PRIVATE_PROPERTIES(SELF);                         \
                                                             \
public:                                                      \
  using OWNTYPE = SELF;                                      \
  using SUPER = BASE_TYPE;                                   \
  WD_ALWAYS_INLINE static const wdRTTI* GetStaticRTTI()      \
  {                                                          \
    return &SELF::s_RTTI;                                    \
  }                                                          \
                                                             \
private:                                                     \
  static wdRTTI s_RTTI;                                      \
  WD_REFLECTION_DEBUG_CODE


#define WD_ADD_DYNAMIC_REFLECTION(SELF, BASE_TYPE)      \
  WD_ADD_DYNAMIC_REFLECTION_NO_GETTER(SELF, BASE_TYPE)  \
public:                                                 \
  virtual const wdRTTI* GetDynamicRTTI() const override \
  {                                                     \
    return &SELF::s_RTTI;                               \
  }


#if WD_ENABLED(WD_COMPILE_FOR_DEVELOPMENT) && WD_ENABLED(WD_COMPILER_MSVC)

#  define WD_REFLECTION_DEBUG_CODE                       \
    static const wdRTTI* ReflectionDebug_GetParentType() \
    {                                                    \
      return __super::GetStaticRTTI();                   \
    }

#  define WD_REFLECTION_DEBUG_GETPARENTFUNC &OwnType::ReflectionDebug_GetParentType

#else
#  define WD_REFLECTION_DEBUG_CODE /*empty*/
#  define WD_REFLECTION_DEBUG_GETPARENTFUNC nullptr
#endif


/// \brief Implements the necessary functionality for a type to be dynamically reflectable.
///
/// \param Type
///   The type for which the reflection functionality should be implemented.
/// \param BaseType
///   The base class type of \a Type. If it has no base class, pass wdNoBase
/// \param AllocatorType
///   The type of an wdRTTIAllocator that can be used to create and destroy instances
///   of \a Type. Pass wdRTTINoAllocator for types that should not be created dynamically.
///   Pass wdRTTIDefaultAllocator<Type> for types that should be created on the default heap.
///   Pass a custom wdRTTIAllocator type to handle allocation differently.
#define WD_BEGIN_DYNAMIC_REFLECTED_TYPE(Type, Version, AllocatorType) \
  WD_RTTIINFO_DECL(Type, Type::SUPER, Version)                        \
  wdRTTI Type::s_RTTI = GetRTTI((Type*)0);                            \
  WD_RTTIINFO_GETRTTI_IMPL_BEGIN(Type, Type::SUPER, AllocatorType)

/// \brief Ends the reflection code block that was opened with WD_BEGIN_DYNAMIC_REFLECTED_TYPE.
#define WD_END_DYNAMIC_REFLECTED_TYPE                                                                                                \
  return wdRTTI(GetTypeName((OwnType*)0), wdGetStaticRTTI<OwnBaseType>(), sizeof(OwnType), GetTypeVersion((OwnType*)0),              \
    wdVariant::TypeDeduction<OwnType>::value, flags, &Allocator, Properties, Functions, Attributes, MessageHandlers, MessageSenders, \
    WD_REFLECTION_DEBUG_GETPARENTFUNC);                                                                                              \
  }

/// \brief All classes that should be dynamically reflectable, need to be derived from this base class.
class WD_FOUNDATION_DLL wdReflectedClass : public wdNoBase
{
  WD_ADD_DYNAMIC_REFLECTION_NO_GETTER(wdReflectedClass, wdNoBase);

public:
  virtual const wdRTTI* GetDynamicRTTI() const { return &wdReflectedClass::s_RTTI; }

public:
  WD_ALWAYS_INLINE wdReflectedClass() = default;
  WD_ALWAYS_INLINE virtual ~wdReflectedClass() = default;

  /// \brief Returns whether the type of this instance is of the given type or derived from it.
  bool IsInstanceOf(const wdRTTI* pType) const;

  /// \brief Returns whether the type of this instance is of the given type or derived from it.
  template <typename T>
  WD_ALWAYS_INLINE bool IsInstanceOf() const
  {
    const wdRTTI* pType = wdGetStaticRTTI<T>();
    return IsInstanceOf(pType);
  }
};
