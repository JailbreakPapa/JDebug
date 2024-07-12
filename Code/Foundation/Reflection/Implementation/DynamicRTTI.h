#pragma once

/// \file

#include <Foundation/Reflection/Implementation/StaticRTTI.h>

/// \brief This needs to be put into the class declaration of EVERY dynamically reflectable class.
///
/// This macro extends a class, such that it is now able to return its own type information via GetDynamicRTTI(),
/// which is a virtual function, that is reimplemented on each type. A class needs to be derived from nsReflectedClass
/// (at least indirectly) for this.
#define NS_ADD_DYNAMIC_REFLECTION_NO_GETTER(SELF, BASE_TYPE) \
  NS_ALLOW_PRIVATE_PROPERTIES(SELF);                         \
                                                             \
public:                                                      \
  using OWNTYPE = SELF;                                      \
  using SUPER = BASE_TYPE;                                   \
  NS_ALWAYS_INLINE static const nsRTTI* GetStaticRTTI()      \
  {                                                          \
    return &SELF::s_RTTI;                                    \
  }                                                          \
                                                             \
private:                                                     \
  static nsRTTI s_RTTI;                                      \
  NS_REFLECTION_DEBUG_CODE


#define NS_ADD_DYNAMIC_REFLECTION(SELF, BASE_TYPE)      \
  NS_ADD_DYNAMIC_REFLECTION_NO_GETTER(SELF, BASE_TYPE)  \
public:                                                 \
  virtual const nsRTTI* GetDynamicRTTI() const override \
  {                                                     \
    return &SELF::s_RTTI;                               \
  }


#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT) && NS_ENABLED(NS_COMPILER_MSVC)

#  define NS_REFLECTION_DEBUG_CODE                       \
    static const nsRTTI* ReflectionDebug_GetParentType() \
    {                                                    \
      return __super::GetStaticRTTI();                   \
    }

#  define NS_REFLECTION_DEBUG_GETPARENTFUNC &OwnType::ReflectionDebug_GetParentType

#else
#  define NS_REFLECTION_DEBUG_CODE /*empty*/
#  define NS_REFLECTION_DEBUG_GETPARENTFUNC nullptr
#endif


/// \brief Implements the necessary functionality for a type to be dynamically reflectable.
///
/// \param Type
///   The type for which the reflection functionality should be implemented.
/// \param BaseType
///   The base class type of \a Type. If it has no base class, pass nsNoBase
/// \param AllocatorType
///   The type of an nsRTTIAllocator that can be used to create and destroy instances
///   of \a Type. Pass nsRTTINoAllocator for types that should not be created dynamically.
///   Pass nsRTTIDefaultAllocator<Type> for types that should be created on the default heap.
///   Pass a custom nsRTTIAllocator type to handle allocation differently.
#define NS_BEGIN_DYNAMIC_REFLECTED_TYPE(Type, Version, AllocatorType) \
  NS_RTTIINFO_DECL(Type, Type::SUPER, Version)                        \
  nsRTTI Type::s_RTTI = GetRTTI((Type*)0);                            \
  NS_RTTIINFO_GETRTTI_IMPL_BEGIN(Type, Type::SUPER, AllocatorType)

/// \brief Ends the reflection code block that was opened with NS_BEGIN_DYNAMIC_REFLECTED_TYPE.
#define NS_END_DYNAMIC_REFLECTED_TYPE                                                                                                \
  return nsRTTI(GetTypeName((OwnType*)0), nsGetStaticRTTI<OwnBaseType>(), sizeof(OwnType), GetTypeVersion((OwnType*)0),              \
    nsVariant::TypeDeduction<OwnType>::value, flags, &Allocator, Properties, Functions, Attributes, MessageHandlers, MessageSenders, \
    NS_REFLECTION_DEBUG_GETPARENTFUNC);                                                                                              \
  }

/// \brief Same as NS_BEGIN_DYNAMIC_REFLECTED_TYPE but forces the type to be treated as abstract by reflection even though
/// it might not be abstract from a C++ perspective.
#define NS_BEGIN_ABSTRACT_DYNAMIC_REFLECTED_TYPE(Type, Version)     \
  NS_BEGIN_DYNAMIC_REFLECTED_TYPE(Type, Version, nsRTTINoAllocator) \
    flags.Add(nsTypeFlags::Abstract);

#define NS_END_ABSTRACT_DYNAMIC_REFLECTED_TYPE NS_END_DYNAMIC_REFLECTED_TYPE

/// \brief All classes that should be dynamically reflectable, need to be derived from this base class.
class NS_FOUNDATION_DLL nsReflectedClass : public nsNoBase
{
  NS_ADD_DYNAMIC_REFLECTION_NO_GETTER(nsReflectedClass, nsNoBase);

public:
  virtual const nsRTTI* GetDynamicRTTI() const { return &nsReflectedClass::s_RTTI; }

public:
  NS_ALWAYS_INLINE nsReflectedClass() = default;
  NS_ALWAYS_INLINE virtual ~nsReflectedClass() = default;

  /// \brief Returns whether the type of this instance is of the given type or derived from it.
  bool IsInstanceOf(const nsRTTI* pType) const;

  /// \brief Returns whether the type of this instance is of the given type or derived from it.
  template <typename T>
  NS_ALWAYS_INLINE bool IsInstanceOf() const
  {
    const nsRTTI* pType = nsGetStaticRTTI<T>();
    return IsInstanceOf(pType);
  }
};
