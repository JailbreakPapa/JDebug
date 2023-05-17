#pragma once

/// \file

#include <Foundation/Reflection/Implementation/DynamicRTTI.h>

/// \brief Casts the given object to the given type with no runtime cost (like C++ static_cast).
/// This function will assert when the object is not an instance of the given type.
/// E.g. DerivedType* d = wdStaticCast<DerivedType*>(pObj);
template <typename T>
WD_ALWAYS_INLINE T wdStaticCast(wdReflectedClass* pObject)
{
  typedef typename wdTypeTraits<T>::NonPointerType NonPointerT;
  WD_ASSERT_DEV(pObject == nullptr || pObject->IsInstanceOf<NonPointerT>(), "Invalid static cast: Object of type '{0}' is not an instance of '{1}'",
    pObject->GetDynamicRTTI()->GetTypeName(), wdGetStaticRTTI<NonPointerT>()->GetTypeName());
  return static_cast<T>(pObject);
}

/// \brief Casts the given object to the given type with no runtime cost (like C++ static_cast).
/// This function will assert when the object is not an instance of the given type.
/// E.g. const DerivedType* d = wdStaticCast<const DerivedType*>(pConstObj);
template <typename T>
WD_ALWAYS_INLINE T wdStaticCast(const wdReflectedClass* pObject)
{
  typedef typename wdTypeTraits<T>::NonConstReferencePointerType NonPointerT;
  WD_ASSERT_DEV(pObject == nullptr || pObject->IsInstanceOf<NonPointerT>(), "Invalid static cast: Object of type '{0}' is not an instance of '{1}'",
    pObject->GetDynamicRTTI()->GetTypeName(), wdGetStaticRTTI<NonPointerT>()->GetTypeName());
  return static_cast<T>(pObject);
}

/// \brief Casts the given object to the given type with no runtime cost (like C++ static_cast).
/// This function will assert when the object is not an instance of the given type.
/// E.g. DerivedType& d = wdStaticCast<DerivedType&>(obj);
template <typename T>
WD_ALWAYS_INLINE T wdStaticCast(wdReflectedClass& in_object)
{
  typedef typename wdTypeTraits<T>::NonReferenceType NonReferenceT;
  WD_ASSERT_DEV(in_object.IsInstanceOf<NonReferenceT>(), "Invalid static cast: Object of type '{0}' is not an instance of '{1}'",
    in_object.GetDynamicRTTI()->GetTypeName(), wdGetStaticRTTI<NonReferenceT>()->GetTypeName());
  return static_cast<T>(in_object);
}

/// \brief Casts the given object to the given type with no runtime cost (like C++ static_cast).
/// This function will assert when the object is not an instance of the given type.
/// E.g. const DerivedType& d = wdStaticCast<const DerivedType&>(constObj);
template <typename T>
WD_ALWAYS_INLINE T wdStaticCast(const wdReflectedClass& object)
{
  typedef typename wdTypeTraits<T>::NonConstReferenceType NonReferenceT;
  WD_ASSERT_DEV(object.IsInstanceOf<NonReferenceT>(), "Invalid static cast: Object of type '{0}' is not an instance of '{1}'",
    object.GetDynamicRTTI()->GetTypeName(), wdGetStaticRTTI<NonReferenceT>()->GetTypeName());
  return static_cast<T>(object);
}

/// \brief Casts the given object to the given type with by checking if the object is actually an instance of the given type (like C++
/// dynamic_cast). This function will return a nullptr if the object is not an instance of the given type.
/// E.g. DerivedType* d = wdDynamicCast<DerivedType*>(pObj);
template <typename T>
WD_ALWAYS_INLINE T wdDynamicCast(wdReflectedClass* pObject)
{
  if (pObject)
  {
    typedef typename wdTypeTraits<T>::NonPointerType NonPointerT;
    if (pObject->IsInstanceOf<NonPointerT>())
    {
      return static_cast<T>(pObject);
    }
  }
  return nullptr;
}

/// \brief Casts the given object to the given type with by checking if the object is actually an instance of the given type (like C++
/// dynamic_cast). This function will return a nullptr if the object is not an instance of the given type.
/// E.g. const DerivedType* d = wdDynamicCast<const DerivedType*>(pConstObj);
template <typename T>
WD_ALWAYS_INLINE T wdDynamicCast(const wdReflectedClass* pObject)
{
  if (pObject)
  {
    typedef typename wdTypeTraits<T>::NonConstReferencePointerType NonPointerT;
    if (pObject->IsInstanceOf<NonPointerT>())
    {
      return static_cast<T>(pObject);
    }
  }
  return nullptr;
}
