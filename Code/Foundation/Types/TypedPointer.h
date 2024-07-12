#pragma once

#include <Foundation/Types/TypeTraits.h>

class nsRTTI;

/// \brief A typed raw pointer.
///
/// Common use case is the storage of object pointers inside an nsVariant.
/// Has the same lifetime concerns that any other raw pointer.
/// \sa nsVariant
struct nsTypedPointer
{
  NS_DECLARE_POD_TYPE();
  void* m_pObject = nullptr;
  const nsRTTI* m_pType = nullptr;

  nsTypedPointer() = default;
  nsTypedPointer(void* pObject, const nsRTTI* pType)
    : m_pObject(pObject)
    , m_pType(pType)
  {
  }

  bool operator==(const nsTypedPointer& rhs) const
  {
    return m_pObject == rhs.m_pObject;
  }
  bool operator!=(const nsTypedPointer& rhs) const
  {
    return m_pObject != rhs.m_pObject;
  }
};
