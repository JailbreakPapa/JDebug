#pragma once

/// \file

#include <Foundation/Reflection/Implementation/DynamicRTTI.h>

NS_WARNING_PUSH()
NS_WARNING_DISABLE_CLANG("-Wunused-local-typedef")
NS_WARNING_DISABLE_GCC("-Wunused-local-typedefs")

/// \brief Casts the given object to the given type with no runtime cost (like C++ static_cast).
/// This function will assert when the object is not an instance of the given type.
/// E.g. DerivedType* d = nsStaticCast<DerivedType*>(pObj);
template <typename T>
NS_ALWAYS_INLINE T nsStaticCast(nsReflectedClass* pObject)
{
  using NonPointerT = typename nsTypeTraits<T>::NonPointerType;
  NS_ASSERT_DEV(pObject == nullptr || pObject->IsInstanceOf<NonPointerT>(), "Invalid static cast: Object of type '{0}' is not an instance of '{1}'",
    pObject->GetDynamicRTTI()->GetTypeName(), nsGetStaticRTTI<NonPointerT>()->GetTypeName());
  return static_cast<T>(pObject);
}

/// \brief Casts the given object to the given type with no runtime cost (like C++ static_cast).
/// This function will assert when the object is not an instance of the given type.
/// E.g. const DerivedType* d = nsStaticCast<const DerivedType*>(pConstObj);
template <typename T>
NS_ALWAYS_INLINE T nsStaticCast(const nsReflectedClass* pObject)
{
  using NonPointerT = typename nsTypeTraits<T>::NonConstReferencePointerType;
  NS_ASSERT_DEV(pObject == nullptr || pObject->IsInstanceOf<NonPointerT>(), "Invalid static cast: Object of type '{0}' is not an instance of '{1}'",
    pObject->GetDynamicRTTI()->GetTypeName(), nsGetStaticRTTI<NonPointerT>()->GetTypeName());
  return static_cast<T>(pObject);
}

/// \brief Casts the given object to the given type with no runtime cost (like C++ static_cast).
/// This function will assert when the object is not an instance of the given type.
/// E.g. DerivedType& d = nsStaticCast<DerivedType&>(obj);
template <typename T>
NS_ALWAYS_INLINE T nsStaticCast(nsReflectedClass& in_object)
{
  using NonReferenceT = typename nsTypeTraits<T>::NonReferenceType;
  NS_ASSERT_DEV(in_object.IsInstanceOf<NonReferenceT>(), "Invalid static cast: Object of type '{0}' is not an instance of '{1}'",
    in_object.GetDynamicRTTI()->GetTypeName(), nsGetStaticRTTI<NonReferenceT>()->GetTypeName());
  return static_cast<T>(in_object);
}

/// \brief Casts the given object to the given type with no runtime cost (like C++ static_cast).
/// This function will assert when the object is not an instance of the given type.
/// E.g. const DerivedType& d = nsStaticCast<const DerivedType&>(constObj);
template <typename T>
NS_ALWAYS_INLINE T nsStaticCast(const nsReflectedClass& object)
{
  using NonReferenceT = typename nsTypeTraits<T>::NonConstReferenceType;
  NS_ASSERT_DEV(object.IsInstanceOf<NonReferenceT>(), "Invalid static cast: Object of type '{0}' is not an instance of '{1}'",
    object.GetDynamicRTTI()->GetTypeName(), nsGetStaticRTTI<NonReferenceT>()->GetTypeName());
  return static_cast<T>(object);
}

/// \brief Casts the given object to the given type with by checking if the object is actually an instance of the given type (like C++
/// dynamic_cast). This function will return a nullptr if the object is not an instance of the given type.
/// E.g. DerivedType* d = nsDynamicCast<DerivedType*>(pObj);
template <typename T>
NS_ALWAYS_INLINE T nsDynamicCast(nsReflectedClass* pObject)
{
  if (pObject)
  {
    using NonPointerT = typename nsTypeTraits<T>::NonPointerType;
    if (pObject->IsInstanceOf<NonPointerT>())
    {
      return static_cast<T>(pObject);
    }
  }
  return nullptr;
}

/// \brief Casts the given object to the given type with by checking if the object is actually an instance of the given type (like C++
/// dynamic_cast). This function will return a nullptr if the object is not an instance of the given type.
/// E.g. const DerivedType* d = nsDynamicCast<const DerivedType*>(pConstObj);
template <typename T>
NS_ALWAYS_INLINE T nsDynamicCast(const nsReflectedClass* pObject)
{
  if (pObject)
  {
    using NonPointerT = typename nsTypeTraits<T>::NonConstReferencePointerType;
    if (pObject->IsInstanceOf<NonPointerT>())
    {
      return static_cast<T>(pObject);
    }
  }
  return nullptr;
}

NS_WARNING_POP()
